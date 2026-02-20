#include "driver/twai.h"
#include "CAN.h"
#include "device.h"
#include "api.h"
#include "freertos/task.h"
#include "bitset"


void handle_twai_message(twai_message_t message){
    long unsigned int rxId;
    int index;
    static int max_max = 1;
    static int rxCount[1<<11]; // there are 5 bits of type and 6 bits of number in the index
    static int count = 0; // loop count for clearing every 10th time
  
    // Parse the CAN Header according to FRC standard
    rxId = message.identifier;
    
    CANHeader header;
    header.devNum= rxId & 0x3F;
    header.apiIndex = (rxId & 0x3C0) >> 6;
    header.apiClass = (rxId & 0xFC00) >> 10;
    header.manuf = (rxId & 0xFF0000) >> 16;
    header.devType = (rxId & 0x1F000000) >> 24;
    
    // check for broadcast signals
    if (header.devType == 0 && header.manuf == 0) 
    {
       Serial.println("Got broadcast");
       writeFuncArray[0](header, &message.data);
    }
    
    // filter non-addressed messages
    if (header.devType != 10 || header.manuf != 8 || header.devNum != deviceID) return;
   

    // Handle RTR frames, where we treat RTR as config
    if (message.rtr) {
        if (header.apiClass < std::size(confFuncArray)){
            
            if (confFuncArray[header.apiClass] != nullptr) { // If the read is implemented
                //Serial.printf("Passing header with apiIndex %d", header.apiIndex);
                uint32_t response = confFuncArray[header.apiClass](header); // PHIL - what happens if apiClass goes past the end of the array?
                send_data_frame(message.identifier, message.data_length_code, pack_data(response));
		//send_rtr_reply(message.identifier, 1, 0xAA);
            } else{
                Serial.println("Bad class");
            }
        } else {
            Serial.printf("Don't reply bad class\n");
        }
    } 
    // Handle data frames
    else {
        if (header.apiClass < std::size(writeFuncArray)){ // Handle an apiClass goes past end of array
                                                           //
            if (writeFuncArray[header.apiClass] != nullptr) // If the read is implemented
                writeFuncArray[header.apiClass](header, &message.data);
        } else {
            Serial.printf("bad Class %d\n", header.apiClass);
        }
    }
    //Serial.printf("\nPost Message passed | class: %d, index: %d, rtr %d  data[0] %d\n", header.apiClass, header.apiIndex, message.rtr, message.data[0]);
}

// runs on a task, handles the constant broadcasts
void broadcastHandler(void *pvParameters) {
    while (1) {
        for (broadcastOperation bcast : broadcastFuncArray){
            bcast();
        }
        delay(broadcastPeriod);
    }
}

uint32_t broadcastPeriod = 20;

void setupBroadcast(){
    TaskHandle_t broadcast_task;
    xTaskCreate(broadcastHandler, "BROADCASTHANDLE",4096,NULL,0, &broadcast_task);
}



/** @brief given a little endian array, gets a uint32_t from a range of bytes, indexed at 0
 * @param startByte inclusive starting byte
 * @param endByte inclusive ending byte
 */

uint32_t unpack_int(uint8_t (*data)[8], int startByte, int endByte){
    if (startByte < 0 || endByte >= 8 || endByte-startByte >= 4){
        Serial.print("Attempted to retrieve non-existant bytes from message");
        return 0;
    }

    uint32_t result = 0;
    for (int i = endByte; i>=startByte;i--){ 
        result <<= 8; 
        result += (*data)[i]; 
    }
    

    return result;
}

/**@brief packs a single integer into a uint8_t, length 8 array. Less versatile but easier to use and more efficient than the pack_data below
 * 
 */
std::array<uint8_t,8> pack_data(uint32_t dataInt) {
    std::array<uint8_t, 8> data{};
    for (int i = 0; i<4; i++){
        data[i] = (uint8_t) dataInt & 0xFF;
        dataInt >>= 8;
    }

    return data;
}

/**Packs given data into a uint8_t, 8 length array, given a vector of numbers and bit sizes, where the two must be equal. Is untested.
 */
std::array<uint8_t,8> pack_data(std::vector<uint32_t> data, std::vector<uint32_t> bitSizes){
    std::array<uint8_t,8> packedData = {};

    int arrSize = std::size(data);

    if (arrSize != std::size(bitSizes)) {
        Serial.println("Attempted to pack data with extra datapoints in either the data or the bitSizes");
        return packedData;
    };

    // Store bits in bitset;
    long bs;
    int totalSize = 0;
    for (int idx = 0; idx < arrSize;idx++){
        bs |= (
                (long) (data[idx] & ((1ULL << bitSizes[idx])-1))
                ) << totalSize;
        totalSize += bitSizes[idx];
    }

    //Convert bitset byte by byte to array
    long mask = 0xFF;
    for (int idx = 0; idx < 8;idx++){
        packedData[idx] = bs & mask;
        bs >>= 8;
    }

    return packedData;
}

// sends a CAN message with a header.
void send_data_frame(long unsigned int identifier, int DLC, std::array<uint8_t,8> data){
    
    
    twai_message_t tx_msg;
    tx_msg.identifier = identifier;      // Match the requested ID
    tx_msg.extd = 1;            // 0 for Standard, 1 for Extended (FRC is extended?)
    tx_msg.rtr = 0;             // MUST be 0 to send actual data
    tx_msg.data_length_code = DLC; // Number of bytes to send - should match the request
    for (int i = 0; i<DLC;i++) {
        tx_msg.data[i] = data[i];
    }

    esp_err_t rval = twai_transmit(&tx_msg, pdMS_TO_TICKS(POLLING_RATE_MS));
    if (rval != ESP_OK) {
      Serial.printf("Failed to send message: 0x%x\n", rval);
    }
}


/*
class PackedBuffer{    
    
    public:
        static PackedBuffer wrap(uint8_t (*data)[]); 
        static PackedBuffer wrap(std::vector<uint8_t> data);
};*/

PackedBuffer::PackedBuffer(){buf = 0;}


void PackedBuffer::putBits(int bits, int data){
    buf << bits;
    buf |= data & ((1UL << bits)-1);
}

void PackedBuffer::putBool(bool val){
    putBits(1,val);
};

void PackedBuffer::putByte(uint8_t byte){ putBits(8,byte); };
void PackedBuffer::putWord(uint32_t word){ putBits(32,word); };

unsigned int PackedBuffer::consumeBits(int bits){
    int consumed = buf | ((1UL << bits)-1);
    buf >> bits;
    return consumed;
}

bool PackedBuffer::consumeBool(){ return consumeBits(1); };
uint8_t PackedBuffer::consumeByte(){ return consumeBits(8); };
uint32_t PackedBuffer::consumeWord(){ return consumeBits(32); }

PackedBuffer PackedBuffer::wrap(uint8_t (*data)[8]){
    PackedBuffer pbuf = PackedBuffer();

    for (uint8_t byte : (*data)){
        pbuf.putByte(byte);
    }

    return pbuf;
};


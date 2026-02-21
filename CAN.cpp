#include "driver/twai.h"
#include "CAN.h"
#include "device.h"
#include "api.h"
#include "freertos/task.h"


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
       PackedBuffer pbuf = PackedBuffer::wrap(&message.data);
       writeFuncArray[0](header, &pbuf);
       
    }
    
    // filter non-addressed messages
    if (header.devType != 10 || header.manuf != 8 || header.devNum != deviceID) return;
   

    // Handle RTR frames, where we treat RTR as config
    if (message.rtr) {
        if (header.apiClass < std::size(confFuncArray)){
            
            if (confFuncArray[header.apiClass] != nullptr) { // If the read is implemented
                
                PackedBuffer response = confFuncArray[header.apiClass](header);
                send_data_frame(message.identifier, message.data_length_code, &response);
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
        PackedBuffer pbuf = PackedBuffer::wrap(&message.data);
        if (header.apiClass < std::size(writeFuncArray)){ // Handle an apiClass goes past end of array
                                                           //
            if (writeFuncArray[header.apiClass] != nullptr) // If the read is implemented
                writeFuncArray[header.apiClass](header, &pbuf);
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


// sends a CAN message with a header.
void send_data_frame(long unsigned int identifier, int DLC, PackedBuffer* pbuf){
    
    
    twai_message_t tx_msg;
    tx_msg.identifier = identifier;      // Match the requested ID
    tx_msg.extd = 1;            // 0 for Standard, 1 for Extended (FRC is extended?)
    tx_msg.rtr = 0;             // MUST be 0 to send actual data
    tx_msg.data_length_code = DLC; // Number of bytes to send - should match the request
    for (int i = 0; i<DLC;i++) {
        tx_msg.data[i] = pbuf->consumeByte();
    }

    esp_err_t rval = twai_transmit(&tx_msg, pdMS_TO_TICKS(POLLING_RATE_MS));
    if (rval != ESP_OK) {
      Serial.printf("Failed to send message: 0x%x\n", rval);
    }
}


/////////////////////////////////////
// Define PackedBuffer implementation
/////////////////////////////////////

PackedBuffer::PackedBuffer(){buf = 0; cur=0;}


void PackedBuffer::putBits(int bits, unsigned long long data){
    // Mask the data to the bits
    data &= ((1UL << bits)-1);
    // Add the data to the location of the cursor
    buf |= (data << cur);
    // Update cursor position
    cur += bits;
    
}

void PackedBuffer::putBool(bool val){
    putBits(1,val);
};

void PackedBuffer::putByte(uint8_t byte){ putBits(8,byte); };
void PackedBuffer::putWord(uint32_t word){ putBits(32,word); };

unsigned long long PackedBuffer::consumeBits(int bitlength){
    unsigned long long consumed = (buf & ((1ULL << bitlength)-1));
    buf >>= bitlength;
    // adjust cursor
    cur -= bitlength;
    if (cur < 0) cur = 0;

    return consumed;
}

bool PackedBuffer::consumeBool(){ return consumeBits(1); };
uint8_t PackedBuffer::consumeByte(){ return consumeBits(8); };
uint32_t PackedBuffer::consumeWord(){ return consumeBits(32); }

PackedBuffer PackedBuffer::wrap(uint8_t (*data)[DATA_BYTES_COUNT]){
    PackedBuffer pbuf = PackedBuffer();

    for (uint8_t byte : (*data)){
        pbuf.putByte(byte);
    }

    return pbuf;
};

PackedBuffer PackedBuffer::wrap(unsigned long long data, int bitlength){
    PackedBuffer pbuf = PackedBuffer();

    pbuf.putBits(bitlength,data);
    return pbuf;
}

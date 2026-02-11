#include "driver/twai.h"
#include "src/CAN/CAN.h"
#include "src/device/device.h"
#include "src/api/api.h"


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
    if (header.devType == 0 && header.manuf == 0) writeArray[0](header, &message.data);

    // filter non-addressed messages
    if (header.devType != 10 || header.manuf != 8 || header.devNum != getDeviceNum()) return;
    
    // Handle RTR frames
    if (message.rtr) {
        if (0<header.apiClass && header.apiClass <= std::size(readArray)){
            
            if (readArray[header.apiClass] != nullptr) { // If the read is implemented

                uint32_t response = readArray[header.apiClass](header);
                send_rtr_reply(message.identifier, get_message_from_int(response));
            }
        } // Implement response
    } 
    // Handle data frames
    else {
        if (0<header.apiClass && header.apiClass <= std::size(writeArray)){
            if (writeArray[header.apiClass] != nullptr) // If the read is implemented

                writeArray[header.apiClass](header, &message.data);
        }
    }
}

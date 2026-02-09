#include "driver/twai.h"
#include "src/CAN/CAN.h"
#include "src/device/device.h"

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
    
    
    // Implement broadcast signals later, for now just return
    
    // Todo figure out how to store device num. At the moment the program takes in all inputs

    if (header.devType != 10 || header.manuf != 8 || header.devNum != getDeviceNum()) return;
    

    // Add api calls
}

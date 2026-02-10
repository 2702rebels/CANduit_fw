#ifndef API_H
#define API_H

#include "src/CAN/CAN.h"

typedef void (*writeOperation)(CANHeader header, uint8_t (*data)[8]); 
typedef uint (*readOperation)(CANHeader header); 

// Broadcast operation
void BROADCAST_ALL(CANHeader header, uint8_t (*data)[8]);

// Define write operations
void MODE_W(CANHeader header, uint8_t (*data)[8]);
void DIGITAL_STATE_W(CANHeader header, uint8_t (*data)[8]);


// Define read operations
uint MODE_R(CANHeader header);
uint DIGITAL_STATE_R(CANHeader header);
uint PERIOD_R(CANHeader header); 
uint HIGHTIME_R(CANHeader header);
uint LOWTIME_R(CANHeader header);

// Declare write and read arrays. Make sure to put items in order as they are in CAN.h APIClass struct
// Add nullptr for unimplemented functions or just empty slots
const writeOperation writeArray[6] = {
    BROADCAST_ALL, 
    MODE_W, 
    DIGITAL_STATE_W, 
    nullptr, // Period has no write 
    nullptr, // Hightime has no write
    nullptr  // lowtime has no write
};

const readOperation readArray[6] = {
    nullptr, // Broadcastr has no read operation
    MODE_R, 
    DIGITAL_STATE_R, 
    PERIOD_R, 
    HIGHTIME_R, 
    LOWTIME_R
};


#endif

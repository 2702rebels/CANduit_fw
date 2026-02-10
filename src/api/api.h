#ifndef API_H
#define API_H

#include "src/CAN/CAN.h"

typedef void (*writeOperation)(CANHeader header, uint8_t (*data)[8]); 
typedef int (*readOperation)(CANHeader header); 

// Broadcast operation
void BROADCAST_ALL(CANHeader header, uint8_t (*data)[8]);

// Define write operations
void MODE_W(CANHeader header, uint8_t (*data)[8]);
void DIGITAL_STATE_W(CANHeader header, uint8_t (*data)[8]);


// Define read operations
int MODE_R(CANHeader header);
int DIGITAL_STATE_R(CANHeader header);



// Declare write and read arrays. Make sure to put items in order as they are in CAN.h APIClass struct
const writeOperation writeArray[6] = {
    BROADCAST_ALL, 
    MODE_W, 
    DIGITAL_STATE_W, 
    nullptr, 
    nullptr, 
    nullptr
};

const readOperation readArray[6] = {
    nullptr, 
    MODE_R, 
    DIGITAL_STATE_R, 
    nullptr, 
    nullptr, 
    nullptr
};


#endif

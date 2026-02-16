#ifndef API_H
#define API_H

#include "CAN.h"
#include "stdint.h"

typedef void (*writeOperation)(CANHeader header, uint8_t (*data)[8]); 
typedef uint32_t (*readOperation)(CANHeader header); 
typedef void (*broadcastOperation)();


// Broadcast recieve 
void BROADCAST_RECIEVE(CANHeader header, uint8_t (*data)[8]); // special case, goes under Writes

// Broadcast sends
void BROADCAST_STATUS();

// Define write operations
void MODE_W(CANHeader header, uint8_t (*data)[8]);
void DIGITAL_STATE_W(CANHeader header, uint8_t (*data)[8]);


// Define read operations
uint32_t MODE_R(CANHeader header);



// Functions which perform operations on data frames
const writeOperation writeFuncArray[] = {
    BROADCAST_RECIEVE, 
    MODE_W, 
    DIGITAL_STATE_W,
};


// Functions which return values on RTR frames, for example, Reading Mode, Broadcast period, pwm sample period
const readOperation confFuncArray[] = {
    nullptr,
    MODE_R, 
};

// Functions which output value on the broadcast timer
const broadcastOperation broadcastFuncArray[] = {
    BROADCAST_STATUS
};


#endif

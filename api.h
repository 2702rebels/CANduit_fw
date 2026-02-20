#ifndef API_H
#define API_H

#include "CAN.h"
#include "stdint.h"

typedef void (*writeOperation)(CANHeader header, PackedBuffer* data); 
typedef PackedBuffer (*readOperation)(CANHeader header); 
typedef void (*broadcastOperation)();


// Broadcast recieve 
void BROADCAST_RECIEVE(CANHeader header, PackedBuffer* data); // special case, goes under Writes

// Broadcast sends
void BROADCAST_STATUS();
void BROADCAST_PWM_TIMES();

// Define write operations
void MODE_W(CANHeader header, PackedBuffer* data);
void DIGITAL_STATE_W(CANHeader header, PackedBuffer* data);
void CONFIG_W(CANHeader header, PackedBuffer* data);

// Define read operations
PackedBuffer MODE_R(CANHeader header);
PackedBuffer CONFIG_R(CANHeader header);



// Functions which perform operations on data frames
const writeOperation writeFuncArray[] = {
    BROADCAST_RECIEVE, 
    MODE_W, 
    DIGITAL_STATE_W,
    nullptr,
    nullptr,
    nullptr,
    CONFIG_W
};


// Functions which return values on RTR frames, for example, Reading Mode, Broadcast period, pwm sample period
const readOperation confFuncArray[] = {
    nullptr,
    MODE_R,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    CONFIG_R

};

// Functions which output value on the broadcast timer
const broadcastOperation broadcastFuncArray[] = {
    BROADCAST_STATUS,
    BROADCAST_PWM_TIMES
};


#endif

#ifndef API_H
#define API_H

#include "./CAN.h"
#include "stdint.h"

typedef void (*writeOperation)(CANHeader header, uint8_t (*data)[8]); 
typedef uint32_t (*readOperation)(CANHeader header); 

// Broadcast operations
void BROADCAST_RECIEVE(CANHeader header, uint8_t (*data)[8]);

// Define write operations
void MODE_W(CANHeader header, uint8_t (*data)[8]);
void DIGITAL_STATE_W(CANHeader header, uint8_t (*data)[8]);


// Define read operations
uint32_t MODE_R(CANHeader header);
uint32_t DIGITAL_STATE_R(CANHeader header);
uint32_t PERIOD_R(CANHeader header);
uint32_t HIGHTIME_R(CANHeader header);
uint32_t LOWTIME_R(CANHeader header);


const writeOperation writeFuncArray[] = {
    BROADCAST_RECIEVE, 
    MODE_W, 
    DIGITAL_STATE_W,
};


const readOperation readFuncArray[] = {
    nullptr, // Broadcastr has no read operation
    MODE_R, 
    DIGITAL_STATE_R, 
    PERIOD_R,
    HIGHTIME_R,
    LOWTIME_R 
};


#endif

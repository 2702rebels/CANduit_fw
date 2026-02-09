#ifndef API_H
#define API_H

#include "src/CAN/CAN.h"

typedef void (*writeOperation)(CANHeader header, uint8_t (*data)[8]); 
typedef int (*readOperation)(CANHeader header); 

extern writeOperation writeArray[8]; 
extern readOperation readArray[8];


#endif

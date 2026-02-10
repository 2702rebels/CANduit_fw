#ifndef API_H
#define API_H

#include "src/CAN/CAN.h"

typedef void (*writeOperation)(CANHeader header, uint8_t (*data)[8]); 
typedef int (*readOperation)(CANHeader header); 





writeOperation MODE_W;

readOperation MODE_R;


// Declare write and read arrays. Make sure to put items in order as they are in CAN.h APIClass struct
const writeOperation writeArray[6] = {nullptr, MODE_W, nullptr, nullptr, nullptr, nullptr}; 
const readOperation readArray[6] = {nullptr, MODE_R, nullptr, nullptr, nullptr, nullptr};


#endif

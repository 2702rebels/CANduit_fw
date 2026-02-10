#include "src/CAN/CAN.h"
#include "src/api/api.h"

void disable();

void BROADCAST_ALL(CANHeader header, uint8_t (*data)[8]){
    
    switch (header.apiIndex){
        case 0:
            disable();
    }
}


void disable(){
    // Implement disable
}

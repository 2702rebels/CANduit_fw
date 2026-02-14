#include "stdint.h"
#include "src/gpio/gpio.h"
#include "src/CAN/CAN.h"
#include "src/api/api.h"
#include <Arduino.h>

// Period only has read mode
// period, hightime, and lowtime are defined in gpio.h


uint32_t PERIOD_R(CANHeader header){
    
    if (!inPorts(header.apiIndex)) return 0;
    
    Port *port = getGPIO(header.apiIndex);
    

    return period[port->id];
}



uint32_t HIGHTIME_R(CANHeader header){
    
    if (!inPorts(header.apiIndex)) return 0;
    
    Port *port = getGPIO(header.apiIndex); // Use array lookup? - PHIL
    Serial.printf("Read high time %d from port %d\n", highTime[port->id], port->id);  

    return highTime[port->id];
}



uint32_t LOWTIME_R(CANHeader header){
    
    if (!inPorts(header.apiIndex)) return 0;
    
    Port *port = getGPIO(header.apiIndex);
    

    return lowTime[port->id];
}

#if 0
uint32_t PWM_R_HIGH(CANHeader header){

    return HIGHTIME_R(header.apiIndex);

}

uint32_t PWM_R_LOW(CANHeader header){
    return LOWTIME_R(header.apiIndex);

}

uint32_t PWM_R_PERIOD(CANHeader header){
    return PERIOD_R(header.apiIndex);
}
#endif

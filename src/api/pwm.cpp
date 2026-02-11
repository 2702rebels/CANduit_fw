#include "stdint.h"
#include "src/gpio/gpio.h"
#include "src/CAN/CAN.h"
#include "src/api/api.h"


// Period only has read mode
// period, hightime, and lowtime are defined in gpio.h


uint32_t PERIOD_R(int gpio){
    
    if (!inPorts(gpio)) return 0;
    
    Port *port = getGPIO(gpio);
    

    return period[port->id];
}



uint32_t HIGHTIME_R(int gpio){
    
    if (!inPorts(gpio)) return 0;
    
    Port *port = getGPIO(gpio);
    

    return highTime[port->id];
}



uint32_t LOWTIME_R(int gpio){
    
    if (!inPorts(gpio)) return 0;
    
    Port *port = getGPIO(gpio);
    

    return lowTime[port->id];
}

uint32_t PWM_R(CANHeader header){
    
    if (header.apiIndex < 10) return PERIOD_R(header.apiIndex % 10);
    if (header.apiIndex < 20) return HIGHTIME_R(header.apiIndex % 10);
    if (header.apiIndex < 30) return LOWTIME_R(header.apiIndex % 10);


    return 0;
}


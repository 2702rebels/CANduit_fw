#include "src/gpio/gpio.h"
#include "src/CAN/CAN.h"
#include "src/api/api.h"

// Period only has read mode
// period, hightime, and lowtime are defined in gpio.h

uint PERIOD_R(CANHeader header){
    
    if (!inPorts(header.apiIndex)) return 0;
    
    Port *port = getGPIO(header.apiIndex);
    

    return period[port->id];
}



uint HIGHTIME_R(CANHeader header){
    
    if (!inPorts(header.apiIndex)) return 0;
    
    Port *port = getGPIO(header.apiIndex);
    

    return highTime[port->id];
}



uint LOWTIME_R(CANHeader header){
    
    if (!inPorts(header.apiIndex)) return 0;
    
    Port *port = getGPIO(header.apiIndex);
    

    return lowTime[port->id];
}

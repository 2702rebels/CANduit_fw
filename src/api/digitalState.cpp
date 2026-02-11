#include "Arduino.h"
#include "stdint.h"
#include "src/gpio/gpio.h"
#include "src/CAN/CAN.h"
#include "src/api/api.h"

// Write port mode
void DIGITAL_STATE_W(CANHeader header, uint8_t (*data)[8]){
    // Header.apiIndex acts as the port ID

    if (!inPorts(header.apiIndex)) return;
    
    Port *port = getGPIO(header.apiIndex);
    
    if (port->mode != GPIOMode.DIG_OUT) return;

    int setting = (*data)[0];
    if (setting != 0 && setting != 1) return;
    
    port->outValue = setting;
    digitalWrite(GPIO[port->id], setting);
}


// Read modes
uint32_t DIGITAL_STATE_R(CANHeader header){
    
    if (!inPorts(header.apiIndex)) return 0;
    
    Port *port = getGPIO(header.apiIndex);
    
    if (port->readOnly){
        return digitalRead(GPIO[port->id]);
    } else {
        return port->outValue;
    }
}


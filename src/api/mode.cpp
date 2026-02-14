#include "Arduino.h"
#include "stdint.h"
#include "src/gpio/gpio.h"
#include "src/CAN/CAN.h"
#include "src/api/api.h"

// Write port mode
void MODE_W(CANHeader header, uint8_t (*data)[8]){
    // Header.apiIndex acts as the port ID
    if (!inPorts(header.apiIndex)) return;
    
    Port *port = getGPIO(header.apiIndex);
    

    port->setMode((*data)[0]);
}


// Read modes
uint32_t MODE_R(CANHeader header){
    Serial.println("ohno"); 
    if (!inPorts(header.apiIndex)) return 0;
    
    Port *port = getGPIO(header.apiIndex);
    
    return port->mode;
}


#include "Arduino.h"
#include "stdint.h"
#include "src/gpio/gpio.h"
#include "src/CAN/CAN.h"
#include "src/api/api.h"


/////////////////////////////////
// Define BROADCAST functions
/////////////////////////////////


void disable();

void BROADCAST_RECIEVE(CANHeader header, uint8_t (*data)[8]){
    
    switch (header.apiIndex){
        case 0:
            disable();
    }
}


void disable(){
    resetAllPorts();
    int i = 0;
    while (1){
        delay(1000);
        Serial.printf("Disabled #%d\n", i);
        i++;
    }
}



////////////////////////
// Define MODE functions
////////////////////////


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

/////////////////////////////////
// Define DIGITAL STATE functions
/////////////////////////////////

// Write port mode
void DIGITAL_STATE_W(CANHeader header, uint8_t (*data)[8]){
    // Header.apiIndex acts as the port I
    
    if (!inPorts(header.apiIndex)) return;
   
    Port *port = getGPIO(header.apiIndex);
    
    if (port->mode != GPIOMode.DIG_OUT) return;


    int setting = (*data)[0];
    if (setting != 0 && setting != 1) return;
    
    digitalWrite(GPIO[port->id], setting);
}


// Read modes
uint32_t DIGITAL_STATE_R(CANHeader header){
    
    if (!inPorts(header.apiIndex)) return 0;
    
    Port *port = getGPIO(header.apiIndex);
    
    if (port->mode != GPIOMode.DIG_IN) return 0;

    return digitalRead(GPIO[port->id]);
}




/////////////////////////////////
// Define PWM INPUT functions
/////////////////////////////////

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

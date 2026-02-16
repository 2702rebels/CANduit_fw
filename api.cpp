#include "Arduino.h"
#include "stdint.h"
#include "gpio.h"
#include "CAN.h"
#include "api.h"
#include "device.h"
#include "array"

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





////////////////////////////////////////
// Define BROADCAST message transmitters
////////////////////////////////////////

void BROADCAST_STATUS(){
        unsigned int identifier;

        int apiClass = 20; // According to protocol document
        int apiIndex = 0;
        identifier = (deviceID + 
                (apiIndex << 6) +
                (apiClass << 10) +
                (8 << 16) + // Manuf 
                (10 << 24) // DevType
        );

        
        std::array<uint8_t,8> data = {};

        // fill first byte
        for (int g = 7; g>=0;g--){
            if (!inPorts(g)) continue;
        
            Port *port = getGPIO(g);
            
            if (port->mode != GPIOMode.DIG_IN) continue;
            
            data[0] <<= 1;
            data[0] = digitalRead(GPIO[port->id]);
        }
        
        // the bitmask of internal state and error flags has yet to be determined
        


        send_data_frame(identifier,3,data);
}


void BROADCAST_PWM_TIMES(){
        unsigned int identifier;
        int apiClass = 21; // According to protocol document

        std::vector<uint32_t> bitSizes = {4,4};

        for (int g = 0; g<= 8; g++){ // on message per port
            
            Port *port = getGPIO(g);
            if (port->mode != GPIOMode.PWM_IN) {
                continue;
            }


            int apiIndex = g; // is the port according to the canduit protocol
            identifier = (deviceID + 
                    (apiIndex << 6) +
                    (apiClass << 10) +
                    (8 << 16) + // Manuf 
                    (10 << 24) // DevType
            );

            // Check if mode is not valid 

            std::vector<uint32_t> datapoints = {
                highTime[g], period[g]
            };

            std::array<uint8_t,8> data = pack_data(datapoints, bitSizes);
                        
            
            send_data_frame(identifier,8,data);
        }
}

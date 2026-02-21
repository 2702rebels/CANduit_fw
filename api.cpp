#include "Arduino.h"
#include "stdint.h"
#include "gpio.h"
#include "CAN.h"
#include "api.h"
#include "device.h"
#include "array"
#include <bit>

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
    port-> outValue = setting;    
    digitalWrite(GPIO[port->id], setting);
}

/////////////////////////////////
// Define PERIOD CONFIG functions
/////////////////////////////////

void CONFIG_W(CANHeader header, uint8_t (*data)[8]){
    switch (header.apiIndex){
        case 1:
            broadcastPeriod = unpack_int(data,0,1);
            break;
        case 2:
            samplePeriod = unpack_int(data,0,1);
            break;
    }
}

uint32_t CONFIG_R(CANHeader header){

    switch (header.apiIndex){
        case 0: // broadcast period
            return broadcastPeriod;

        case 1: //pwm sample period
            return samplePeriod;
    }
    return 0;
}



////////////////////////////////////////
// Define BROADCAST message transmitters
////////////////////////////////////////

void BROADCAST_STATUS(){
        uint32_t identifier;
        static int count = 0;

        
        CANHeader header;
        header.apiClass = 20;
        header.apiIndex = 0;
        header.devNum = deviceID;
        header.devType = 10;
        header.manuf = 8;
        
        identifier = std::bit_cast<uint32_t>(header); // convert struct to uint32_t

        
        std::array<uint8_t,8> data = {};

        // fill first byte
        for (int g = 7; g>=0;g--){
            if (!inPorts(g)) continue;
        
            Port *port = getGPIO(g);
            
            data[0] <<= 1;
            if (port->mode == GPIOMode.DIG_IN){
                data[0] |= digitalRead(GPIO[port->id]);
            
            } else if (port->mode == GPIOMode.DIG_OUT){
                data[0] |= port->outValue;
            }
        }
        
        // the bitmask of internal state and error flags has yet to be determined
        if ((count %51) == 0) 
        {
            Serial.printf("Broadcast Status header 0x%x with data[0]=0x%x\n", header, data[0]);
        }
        count++;
        send_data_frame(identifier,4,data);
}


void BROADCAST_PWM_TIMES(){
        uint32_t identifier;
        CANHeader header;
        static int count = 0;

        //std::vector<uint32_t> bitSizes = {32,32};
        

        for (int g = 0; g< 8; g++){ // on message per port
            
            Port *port = getGPIO(g);
            if (port->mode != GPIOMode.PWM_IN) {
                continue;
            }

            header.apiClass = 21;
            header.apiIndex = g;
            header.devNum = deviceID;
            header.devType = 10;
            header.manuf = 8;

            identifier = std::bit_cast<uint32_t>(header); // convert struct to uint32_t

            // Check if mode is not valid 

            //std::vector<uint32_t> datapoints = {
            //    highTime[g], period[g]
            //};

            std::array<uint8_t,8> data = pack_data(highTime[g], period[g]);
                        
            if ((count %51) == 0) 
            {
                Serial.printf("Broadcast header 0x%x\n", header), 
                Serial.printf("Broadcast PWM gpio %d  high %d usec,  period %d usec\n", g, highTime[g]/1000, period[g]/1000);
                Serial.printf("Broadcast payload %x %x %x %x %x %x %x %x\n", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
            }
            //if (g==6) count++;
            count++;
            send_data_frame(identifier, 8, data);
        }
}

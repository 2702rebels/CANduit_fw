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

void BROADCAST_RECIEVE(CANHeader header, PackedBuffer data){
    
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
void MODE_W(CANHeader header, PackedBuffer* data){
    // Header.apiIndex acts as the port ID
    if (!inPorts(header.apiIndex)) return;
    
    Port *port = getGPIO(header.apiIndex);
    

    port->setMode(data->consumeByte());
}


// Read modes
PackedBuffer MODE_R(CANHeader header){
    if (!inPorts(header.apiIndex)) return PackedBuffer();
    
    Port *port = getGPIO(header.apiIndex);
    
    return PackedBuffer::wrap(port->mode,8);
}

/////////////////////////////////
// Define DIGITAL STATE functions
/////////////////////////////////

// Write port mode
void DIGITAL_STATE_W(CANHeader header, PackedBuffer* data){
    // Header.apiIndex acts as the port I
    
    if (!inPorts(header.apiIndex)) return;
   
    Port *port = getGPIO(header.apiIndex);
    
    if (port->mode != GPIOMode.DIG_OUT) return;


    int setting = data->consumeBool();
    if (setting != 0 && setting != 1) return;
    port-> outValue = setting;    
    digitalWrite(GPIO[port->id], setting);
}

/////////////////////////////////
// Define PERIOD CONFIG functions
/////////////////////////////////

void CONFIG_W(CANHeader header, PackedBuffer* data){
    switch (header.apiIndex){
        case 1:
            broadcastPeriod = data->consumeBits(16);
            break;
        case 2:
            samplePeriod = data->consumeBits(16);
            break;
    }
}

PackedBuffer CONFIG_R(CANHeader header){

    switch (header.apiIndex){
        case 0: // broadcast period
            return PackedBuffer::wrap(broadcastPeriod,16);

        case 1: //pwm sample period
            return PackedBuffer::wrap(samplePeriod,16);
    }
    return PackedBuffer();
}



////////////////////////////////////////
// Define BROADCAST message transmitters
////////////////////////////////////////

void BROADCAST_STATUS(){
        uint32_t identifier;


        CANHeader header;
        header.apiClass = 20;
        header.apiIndex = 0;
        header.devNum = deviceID;
        header.devType = 10;
        header.manuf = 8;

        identifier = std::bit_cast<uint32_t>(header); // convert struct to uint32_t

        
        PackedBuffer data = PackedBuffer();

        // fill first byte
        for (int g = 0; g<8;g++){
            if (!inPorts(g)) continue;
        
            Port *port = getGPIO(g);
            
            
            if (port->mode == GPIOMode.DIG_IN){
                data.putBool(digitalRead(GPIO[port->id]));
            
            } else if (port->mode == GPIOMode.DIG_OUT){
                data.putBool(port->outValue);
            }
        }
        
        // the bitmask of internal state and error flags has yet to be determined
        


        send_data_frame(identifier,4,&data);
}


void BROADCAST_PWM_TIMES(){
        uint32_t identifier;
        CANHeader header;

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

            PackedBuffer data = PackedBuffer();
            data.putWord(highTime[g]);
            data.putWord(period[g]);
            
            send_data_frame(identifier,8,&data);
        }
}

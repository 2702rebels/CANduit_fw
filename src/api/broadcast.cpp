#include "Arduino.h"
#include "src/CAN/CAN.h"
#include "src/api/api.h"
#include "src/gpio/gpio.h"

void disable();

void BROADCAST_ALL(CANHeader header, uint8_t (*data)[8]){
    
    switch (header.apiIndex){
        case 0:
            disable();
    }
}


void disable(){
    resetAllPorts();
    
    while (1){
        // Add blinking pattern
        Serial.println("Disabled");
        delay(2000);
    }

    // Turn mainloop into an LED blinking pattern
}

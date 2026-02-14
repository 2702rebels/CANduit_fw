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
    return; // remove this later
    resetAllPorts();
    int i = 0;
    while (1){
        // Add blinking pattern
        delay(1000);
        Serial.printf("Disabled #%d\n", i);
        i++;
    }

    // Turn mainloop into an LED blinking pattern
}

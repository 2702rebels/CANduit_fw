#include "src/gpio/gpio.h"

// Define the Port class
Port::Port(){}

void Port::reset(){
    // The default port is defined to be a digital output with a value of LOW
    if (!inPorts(id)) return;
    
    mode = GPIOMode.UNASSIGNED;
    outValue = 0;
    readOnly = true;
    pinMode(GPIO[id],OUTPUT);
    digitalWrite(GPIO[id],LOW);
    digitalWrite(PWR[id],LOW);

}

void Port::setMode(int mode){
    
    if (!(0<id && id <= modesCount)) return;
    reset();

    // Based on the mode switch the gpio port settings. No need to add setting for unassigned since that is covered by reset
    this->mode = mode;

    switch (mode){
        case GPIOMode.DIG_IN:
            readOnly = true;
            pinMode(GPIO[id],INPUT);
            digitalWrite(PWR[id],HIGH);
            digitalWrite(DIR[id],LOW);
            break;

        case GPIOMode.DIG_OUT:
            readOnly = false;
            outValue = GPIO_LOW;
            pinMode(GPIO[id],OUTPUT);
            digitalWrite(PWR[id],HIGH);
            digitalWrite(DIR[id],HIGH);
            break;

        case GPIOMode.PWM_IN:
            readOnly = true;
            pinMode(GPIO[id],INPUT);
            digitalWrite(DIR[id],LOW);
            digitalWrite(PWR[id],HIGH);
            break;
        
        case GPIOMode.PWM_OUT:
            readOnly = false;
            outValue = 0;
            pinMode(GPIO[id],OUTPUT);
            digitalWrite(DIR[id],HIGH);
            digitalWrite(PWR[id],HIGH);
            break;
    }
}




#include <Arduino.h>
#include "src/gpio/gpio.h"


/////////////////////////////////
// Define PORT class
///////////////////////////////////

Port::Port(){}

void Port::reset(){
    
    mode = GPIOMode.UNASSIGNED;
    readOnly = true;
    pinMode(GPIO[id],OUTPUT);
    digitalWrite(GPIO[id],LOW);
    digitalWrite(PWR[id],LOW);

}

void Port::setMode(int _mode){
    
    if (!(0<_mode && _mode <= modesCount)) return;
    reset();

    // Based on the mode switch the gpio port settings. No need to add setting for unassigned since that is covered by reset
    mode = _mode;

    switch (_mode){
        case GPIOMode.DIG_IN:
            readOnly = true;
            pinMode(GPIO[id],INPUT);
            digitalWrite(PWR[id],HIGH);
            digitalWrite(DIR[id],LOW);
            break;

        case GPIOMode.DIG_OUT:
            readOnly = false;
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
            
    }
}


/////////////////////////////////
// Define general PORT functions 
/////////////////////////////////

Port ports[portsCount];

void setupGPIO(){
    for (int g = 0;g<portsCount;g++){
        Port newPort;
        newPort.id = g;
        newPort.mode = GPIOMode.UNASSIGNED;
        newPort.readOnly = true;

        ports[g] = newPort; 
        pinMode(GPIO[g], INPUT); // PHIL default to input
        //pinMode(GPIO[g],OUTPUT);
        //digitalWrite(GPIO[g],LOW);

        pinMode(PWR[g],OUTPUT);
        pinMode(DIR[g],OUTPUT);
        digitalWrite(PWR[g],LOW); // PHIL change back to low
        digitalWrite(DIR[g],LOW); // Input by default
    }
}

bool inPorts(int id) {return (0)<=id && id<portsCount;}

Port* getGPIO(int id){
    if (!inPorts(id)) return nullptr;
    return &ports[id];
}

void resetAllPorts(){
    for (int g = 0; g<portsCount;g++){
        getGPIO(g)->reset();
    }
}

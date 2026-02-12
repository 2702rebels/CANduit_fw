#include <Arduino.h>
#include "stdint.h"
#include "src/gpio/gpio.h"


Port ports[portsCount];

void setupGPIO(){
    for (int g = 0;g<portsCount;g++){
        Port newPort;
        newPort.id = g;
        newPort.mode = GPIOMode.UNASSIGNED;
        newPort.outValue = 0;
        newPort.readOnly = true;

        ports[g] = newPort; 
        pinMode(GPIO[g],OUTPUT);
        digitalWrite(GPIO[g],LOW);

        pinMode(PWR[g],OUTPUT);
        pinMode(DIR[g],OUTPUT);
        digitalWrite(PWR[g],LOW);
        digitalWrite(DIR[g],HIGH);
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

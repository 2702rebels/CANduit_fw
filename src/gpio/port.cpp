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

}

void Port::setMode(int id){
    reset();
    mode = id;
    // add switch statement based on mode to port
}




#ifndef GPIO_H
#define GPIO_H

#include "array";


const int GPIO[] = {4, 7, 17, 19, 9, 11, 14, 2};
const int PWR[] = {5, 15, 18, 3, 10, 13, 45, 1};
const int DIR[] = {6, 16, 8, 20, 46, 12, 48, 38};
const int portsCount = 8;


const int GPIO_LOW = 0;
const int GPIO_HIGH = 1;

constexpr struct { 
    const int UNASSIGNED = 0;
    const int DIG_IN = 1;
    const int DIG_OUT = 2;
    const int PWM_IN = 3;
} GPIOMode;

const int modesCount = 4;

class Port {
public:
    Port();
    int id; // What GPIO it represents, 0,1,2,3 ... etc.
    uint8_t mode; // The current mode as represented by the GPIOMode struct
    uint32_t outValue; // Could be up to 24 bit
    bool readOnly;

    void reset();
    void setMode(int id);
};


extern Port ports[portsCount];

// general gpio
void setupGPIO();
Port* getGPIO(int id);
void resetAllPorts();
bool inPorts(int id);
void PWMSetup();

extern volatile uint32_t period[8];
extern volatile uint32_t highTime[8];
extern volatile uint32_t lowTime[8];


#endif

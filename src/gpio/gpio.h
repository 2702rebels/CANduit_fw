#ifndef GPIO_H
#define GPIO_H

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
    const int PWM_OUT = 3;
    const int PWM_IN = 4;
} GPIOMode;

const int modesCount = 5;

class Port {
public:
    Port();
    int id; // What GPIO it represents, 0,1,2,3 ... etc.
    int mode; // The current mode as represented by the GPIOMode struct
    int outValue;
    int readOnly;

    void reset();
    void setPWMIn(); // todo bundle this under setmode
    void setMode(int id);
};


typedef void (*isr_callback)(); 

extern Port ports[portsCount];
extern isr_callback IsrArray[8]; 

// general gpio
void setupGPIO();
Port* getGPIO(int id);
void resetAllPorts();
bool inPorts(int id);

#endif

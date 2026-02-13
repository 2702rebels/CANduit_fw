
#include "Wire.h"
#include "src/gpio/gpio.h";


I2CMaster::I2CMaster(int SDA, int SCL){
    this->SDA = getGPIO(SDA);
    this->SCL = getGPIO(SCL);
    this->SDA->mode = GPIOMode.I2C;
    this->SCL->mode = GPIOMode.I2C;
}

void I2CMaster::destroy(){
    delete[] this;
}



I2CSlave::I2CSlave(int SDA, int SCL){
    this->SDA = getGPIO(SDA);
    this->SCL = getGPIO(SCL);
    this->SDA->mode = GPIOMode.I2C;
    this->SCL->mode = GPIOMode.I2C;

}

void I2CSlave::destroy(){
    delete[] this;
}

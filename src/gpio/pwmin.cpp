#include "src/CAN/CAN.h"
#include "src/gpio/gpio.h"
#include "esp_attr.h"


volatile uint32_t riseTime[8]; 
volatile uint32_t fallTime[8]; 
volatile bool newPulse[8];

// IRAM_ATTR ensures the interrupt runs from RAM, not Flash
void IRAM_ATTR ISR0() {
  // Read the pin state inside the ISR to determine if it was a RISE or FALL
  if (digitalRead(GPIO[0]) == HIGH) { // Rising edge
    riseTime[0] = micros();
  }
  else {
    fallTime[0]=micros();
    newPulse[0]=true;
  }
}
void IRAM_ATTR ISR1() {
  if (digitalRead(GPIO[1]) == HIGH) { // Rising edge
    riseTime[1] = micros();
  }
  else {
    fallTime[1]=micros();
    newPulse[1]=true;
  }
}
void IRAM_ATTR ISR2() {
  if (digitalRead(GPIO[2]) == HIGH) { // Rising edge
    riseTime[2] = micros();
  }
  else {
    fallTime[2]=micros();
    newPulse[2]=true;
  }
}
void IRAM_ATTR ISR3() {
  if (digitalRead(GPIO[3]) == HIGH) { // Rising edge
    riseTime[3] = micros();
  }
  else {
    fallTime[3]=micros();
    newPulse[3]=true;
  }
}
void IRAM_ATTR ISR4() {
  if (digitalRead(GPIO[4]) == HIGH) { // Rising edge
    riseTime[4] = micros();
  }
  else {
    fallTime[4]=micros();
    newPulse[4]=true;
  } 
}
void IRAM_ATTR ISR5() {
  if (digitalRead(GPIO[5]) == HIGH) { // Rising edge
    riseTime[5] = micros();
  }
  else {
    fallTime[5]=micros();
    newPulse[5]=true;
  }
}
void IRAM_ATTR ISR6() {
  if (digitalRead(GPIO[6]) == HIGH) { // Rising edge
    riseTime[6] = micros();
  }
  else {
    fallTime[6]=micros();
    newPulse[6]=true;
  }
}
void IRAM_ATTR ISR7() {
  if (digitalRead(GPIO[7]) == HIGH) { // Rising edge
    riseTime[7] = micros();
  }
  else {
    fallTime[7]=micros();
    newPulse[7]=true;
  }
}

// Create the array of function pointers
isr_callback IsrArray[] = { ISR0, ISR1, ISR2, ISR3, ISR4, ISR5, ISR6, ISR7 };

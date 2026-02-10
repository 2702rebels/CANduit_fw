#include <Arduino.h>
#include "stdint.h"
#include "src/gpio/gpio.h"
#include "esp_attr.h"

volatile uint32_t oldRiseTime[8];
volatile uint32_t riseTime[8]; 
volatile uint32_t fallTime[8]; 
volatile bool newPulse[8];

volatile uint32_t period[8];
volatile uint32_t highTime[8];
volatile uint32_t lowTime[8];

// IRAM_ATTR ensures the interrupt runs from RAM, not Flash
void IRAM_ATTR ISR0() {
    
  // Read the pin state inside the ISR to determine if it was a RISE or FALL
  if (digitalRead(GPIO[0]) == HIGH) { // Rising edge
    oldRiseTime[0] = riseTime[0];
    riseTime[0] = micros();
    period[0] = riseTime[0] - oldRiseTime[0];
    lowTime[0] = riseTime[0] - fallTime[0];
  }
  else {
    fallTime[0]=micros();
    newPulse[0]=true;
    highTime[0] = fallTime[0] - riseTime[0];
  }
}

void IRAM_ATTR ISR1() {
  if (digitalRead(GPIO[1]) == HIGH) {
    oldRiseTime[1] = riseTime[1];
    riseTime[1] = micros();
    period[1] = riseTime[1] - oldRiseTime[1];
    lowTime[1] = riseTime[1] - fallTime[1];
  } else {
    fallTime[1] = micros();
    newPulse[1] = true;
    highTime[1] = fallTime[1] - riseTime[1];
  }
}

void IRAM_ATTR ISR2() {
  if (digitalRead(GPIO[2]) == HIGH) {
    oldRiseTime[2] = riseTime[2];
    riseTime[2] = micros();
    period[2] = riseTime[2] - oldRiseTime[2];
    lowTime[2] = riseTime[2] - fallTime[2];
  } else {
    fallTime[2] = micros();
    newPulse[2] = true;
    highTime[2] = fallTime[2] - riseTime[2];
  }
}

void IRAM_ATTR ISR3() {
  if (digitalRead(GPIO[3]) == HIGH) {
    oldRiseTime[3] = riseTime[3];
    riseTime[3] = micros();
    period[3] = riseTime[3] - oldRiseTime[3];
    lowTime[3] = riseTime[3] - fallTime[3];
  } else {
    fallTime[3] = micros();
    newPulse[3] = true;
    highTime[3] = fallTime[3] - riseTime[3];
  }
}

void IRAM_ATTR ISR4() {
  if (digitalRead(GPIO[4]) == HIGH) {
    oldRiseTime[4] = riseTime[4];
    riseTime[4] = micros();
    period[4] = riseTime[4] - oldRiseTime[4];
    lowTime[4] = riseTime[4] - fallTime[4];
  } else {
    fallTime[4] = micros();
    newPulse[4] = true;
    highTime[4] = fallTime[4] - riseTime[4];
  }
}

void IRAM_ATTR ISR5() {
  if (digitalRead(GPIO[5]) == HIGH) {
    oldRiseTime[5] = riseTime[5];
    riseTime[5] = micros();
    period[5] = riseTime[5] - oldRiseTime[5];
    lowTime[5] = riseTime[5] - fallTime[5];
  } else {
    fallTime[5] = micros();
    newPulse[5] = true;
    highTime[5] = fallTime[5] - riseTime[5];
  }
}

void IRAM_ATTR ISR6() {
  if (digitalRead(GPIO[6]) == HIGH) {
    oldRiseTime[6] = riseTime[6];
    riseTime[6] = micros();
    period[6] = riseTime[6] - oldRiseTime[6];
    lowTime[6] = riseTime[6] - fallTime[6];
  } else {
    fallTime[6] = micros();
    newPulse[6] = true;
    highTime[6] = fallTime[6] - riseTime[6];
  }
}

void IRAM_ATTR ISR7() {
  if (digitalRead(GPIO[7]) == HIGH) {
    oldRiseTime[7] = riseTime[7];
    riseTime[7] = micros();
    period[7] = riseTime[7] - oldRiseTime[7];
    lowTime[7] = riseTime[7] - fallTime[7];
  } else {
    fallTime[7] = micros();
    newPulse[7] = true;
    highTime[7] = fallTime[7] - riseTime[7];
  }
}
// Create the array of function pointers
isr_callback IsrArray[] = { ISR0, ISR1, ISR2, ISR3, ISR4, ISR5, ISR6, ISR7 };

#include <dummy.h>

/* 
This test measures the pulse width of PWM inputs on all GPIOs at the same time
This code assumes positive pulses but it could easily be adapted to negative pulses
by having the falling edge lead the rising edge.

We will use GPIOs 5 through 8.  For testing we connect the output from the servo
controller on the MotorDrivinator

Use Tools > Board > esp32 and choosing "ESP32S3 Dev Module"
*/
// Pin definitions - each CANduit GPIO is controlled by 3 ESP gpios
//   - one for direction, one to enable power and one for the signal
const int GPIO[] = {4, 7, 17, 19, 9, 11, 14, 2};
const int PWR[] = {5, 15, 18, 3, 10, 13, 45, 1};
const int DIR[] = {6, 16, 8, 20, 46, 12, 48, 38};

volatile int  min_value = 1000000;
volatile int max_value = 0;

volatile int pwm[8];

volatile uint32_t RiseTime[8]; 
volatile uint32_t FallTime[8]; 
volatile bool NewPulse[8];


// IRAM_ATTR ensures the interrupt runs from RAM, not Flash
void IRAM_ATTR ISR0() {
  // Read the pin state inside the ISR to determine if it was a RISE or FALL
  if (digitalRead(GPIO[0]) == HIGH) { // Rising edge
    RiseTime[0] = micros();
  }
  else {
    FallTime[0]=micros();
    NewPulse[0]=true;
    pwm[0] = FallTime[0]-RiseTime[0];
  }
}
void IRAM_ATTR ISR1() {
  if (digitalRead(GPIO[1]) == HIGH) { // Rising edge
    RiseTime[1] = micros();
  }
  else {
    FallTime[1]=micros();
    NewPulse[1]=true;
    pwm[1] = FallTime[1]-RiseTime[1];

  }
}
void IRAM_ATTR ISR2() {
  if (digitalRead(GPIO[2]) == HIGH) { // Rising edge
    RiseTime[2] = micros();
  }
  else {
    FallTime[2]=micros();
    NewPulse[2]=true;
    pwm[2] = FallTime[2]-RiseTime[2];

  }
}
void IRAM_ATTR ISR3() {
  if (digitalRead(GPIO[3]) == HIGH) { // Rising edge
    RiseTime[3] = micros();
  }
  else {
    FallTime[3]=micros();
    NewPulse[3]=true;
    pwm[3] = FallTime[3]-RiseTime[3];
  }
}
void IRAM_ATTR ISR4() {
  if (digitalRead(GPIO[4]) == HIGH) { // Rising edge
    RiseTime[4] = micros();
  }
  else {
    FallTime[4]=micros();
    NewPulse[4]=true;
    pwm[4] = FallTime[4]-RiseTime[4];
  } 
}
void IRAM_ATTR ISR5() {
  if (digitalRead(GPIO[5]) == HIGH) { // Rising edge
    RiseTime[5] = micros();
  }
  else {
    FallTime[5]=micros();
    NewPulse[5]=true;
    pwm[5] = FallTime[5]-RiseTime[5];
  }
}
void IRAM_ATTR ISR6() {
  if (digitalRead(GPIO[6]) == HIGH) { // Rising edge
    RiseTime[6] = micros();
  }
  else {
    FallTime[6]=micros();
    NewPulse[6]=true;
    pwm[6] = FallTime[6]-RiseTime[6];
  }
}
void IRAM_ATTR ISR7() {
  if (digitalRead(GPIO[7]) == HIGH) { // Rising edge
    RiseTime[7] = micros();
  }
  else {
    FallTime[7]=micros();
    NewPulse[7]=true;
    pwm[7] = FallTime[7]-RiseTime[7];

    if (pwm[7] > max_value){
      max_value = pwm[7];
    }
    
    if (pwm[7] < min_value){
      min_value = pwm[7];
    }
  }
}

// Create the array of function pointers
typedef void (*isr_callback)(); 
isr_callback IsrArray[] = { ISR0, ISR1, ISR2, ISR3, ISR4, ISR5, ISR6, ISR7 };


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); // Sets the baud rate at 9600 to match the software setting
  delay(1000);

  // Init the ESP gpios and configure the initial state for the CANduit gpios
  //  to digital inputs
  for (int g=0; g<8; g++)
  {
    pinMode(GPIO[g], INPUT);
    pinMode(PWR[g], OUTPUT);
    pinMode(DIR[g], OUTPUT);

    //digitalWrite(GPIO[g], HIGH);
    digitalWrite(PWR[g], HIGH);
    digitalWrite(DIR[g], LOW); // Set direction to input
  }

  // Attach interrupt handlers for rising and falling edges on each GPIO
  for (int g=0; g<8; g++)
  {
    attachInterrupt(GPIO[g], IsrArray[g], CHANGE);
    //attachInterrupt(GPIO[g], isrFallArray[g], FALLING);
    NewPulse[g] = false;
  }
}

void loop() {
  

  // put your main code here, to run repeatedly:
  //Serial.println("Hello World!"); // Prints "Hello World!" followed by a new line
  // Read each input in turn
    //int a = digitalRead(GPIO[g]);
    //Serial.printf("%d, ", a);
    if (NewPulse[7])
    {
      //Serial.printf("Pulse %d width: %d-%d=%d usec\n", g, FallTime[g], RiseTime[g], FallTime[g]-RiseTime[g]);
      // pwm[g] = FallTime[g]-RiseTime[g];
      NewPulse[7] = false;
      // Serial.print("i got a new pulse!");
      Serial.printf("PWM vale %d, min_value %d, max_value %d\n", pwm[7], min_value, max_value);
      min_value = 1000000000;
      max_value = 0;

      delay(100);

    }
    
  }

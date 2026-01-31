/* Use Tools > Board > esp32 and choosing "ESP32S3 Dev Module"
*/
// Pin definitions - each CANduit GPIO is controlled by 3 ESP gpios
//   - one for direction, one to enable power and one for the signal
// Change these to arrays - PHIL
const int GPIO[] = {4, 7, 17, 19, 9, 11, 14, 2};
const int PWR[] = {5, 15, 18, 3, 10, 13, 45, 1};
const int DIR[] = {6, 16, 8, 20, 46, 12, 48, 38};


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); // Sets the baud rate at 9600 to match the software setting
  delay(1000);

  // Init the ESP gpios and configure the initial state for the CANduit gpios
  //  to digital outputs with level = HIGH
  for (int g=0; g<8; g++)
  {
    pinMode(GPIO[g], OUTPUT);
    pinMode(PWR[g], OUTPUT);
    pinMode(DIR[g], OUTPUT);

    digitalWrite(GPIO[g], HIGH);
    digitalWrite(PWR[g], HIGH);
    digitalWrite(DIR[g], HIGH);
  }
}

void loop() {
#if 0
  // Blink all 8 LEDs together
  for (int g=0; g<8; g++)
  {
    digitalWrite(GPIO[g], HIGH);
  }
  delay(1000); // Waits for one second (1000 milliseconds)
  
  for (int g=0; g<8; g++)
  {
    digitalWrite(GPIO[g], LOW);
  }
  delay(1000);
#endif

  // Blink each LED in turn
  for (int g=0; g<8; g++)
  {
    digitalWrite(GPIO[g], HIGH);
    delay(100);
    digitalWrite(GPIO[g], LOW);
    delay(10);
  }

}

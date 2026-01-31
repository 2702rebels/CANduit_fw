
/* Use Tools > Board > esp32 and choosing "ESP32S3 Dev Module"
*/
// Pin definitions - each CANduit GPIO is controlled by 3 ESP gpios
//   - one for direction, one to enable power and one for the signal
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
    pinMode(GPIO[g], INPUT_PULLUP);
    pinMode(PWR[g], OUTPUT);
    pinMode(DIR[g], OUTPUT);

    //digitalWrite(GPIO[g], HIGH);
    digitalWrite(PWR[g], HIGH);
    digitalWrite(DIR[g], LOW);
  }
}

void loop() {
  Serial.println("\n\nGPIO 0:");
  Serial.print("VAL:  ");
  // Read pulsewidth of GPIO 0
  int a = pulseIn(GPIO[0],HIGH);
    
  Serial.printf("%f, ", a/1000.0);
  delay(500);

}

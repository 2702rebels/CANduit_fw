/* Use Tools > Board > esp32 and choosing "ESP32S3 Dev Module"
*/

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); // Sets the baud rate at 9600 to match the software setting
  delay(1000);

  // GPIO0 = 4, PWR0 = 5, DIR0 = 6
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  digitalWrite(4, HIGH);
  digitalWrite(5, HIGH);
  digitalWrite(6, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Hello World!"); // Prints "Hello World!" followed by a new line
  digitalWrite(4, HIGH);
  delay(1000); // Waits for one second (1000 milliseconds)
  digitalWrite(4, LOW);
  delay(1000);
}

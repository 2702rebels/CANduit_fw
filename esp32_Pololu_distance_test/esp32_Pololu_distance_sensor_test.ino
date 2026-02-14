// Example Arduino program for reading the Pololu Distance Sensor with Pulse Width Output, 50cm Max
 
// Change this to match the Arduino pin connected to the sensor's OUT pin.
 
// GPIO0 = 4, PWR0 = 5, DIR0 = 6
// what the flip does this segment do? Check diagram
// need to test the switch and blue led
// Boston dinamics, new robot check it out
const int GPIO[] = {4, 7, 17, 19, 9, 11, 14, 2};
const int PWR[] = {5, 15, 18, 3, 10, 13, 45, 1};
const int DIR[] = {6, 16, 8, 20, 46, 12, 48, 38};

const uint8_t sensorPin = 17; // or 3 pin


void setup()
{
  Serial.begin(115200);
  pinMode(GPIO[2], INPUT); // Set the sensor pin as an input or as output?

  pinMode(PWR[2], OUTPUT);

  pinMode(DIR[2], OUTPUT);

  //digitalWrite(GPIO[g], HIGH);

  digitalWrite(PWR[2], HIGH);

  //digitalWrite(DIR[2], HIGH);
}
 
void loop()
{
  //int16_t t
  unsigned long t = pulseIn(sensorPin, HIGH, 300000); // 30000 Timeout set to 25ms to handle max pulse period
  Serial.println(t);


 
  if (t == 0)
  {
    // pulseIn() did not detect the start of a pulse within 1 second.
    Serial.println("timeout");
  }
  else if (t > 1850)
  {
    // No detection.
    Serial.println(-1);
  }
  else
  {
    // Valid pulse width reading. Convert pulse width in microseconds to distance in millimeters.
    // All of this found from this article: https://www.pololu.com/product/4064, model being
    // Pololu Distance Sensor with Pulse Width Output, 50cm Max ->  irs16a 0j13085
    int16_t d = (t - 1000) * 3 / 4;
 
    // Limit minimum distance to 0.
    if (d < 0) { d = 0; } 
  
  //  Serial.println("distnace");
    Serial.printf("%d mm, measured distance", d);
    //  As objects approach the sensor, the output pulse width will
    // approach 1.0 ms, while an object detected at 50 cm will produce a 1.667 ms pulse width
    // Serial.println("");

    Serial.println("");
    Serial.printf("%d µs, pulse width time", t);
    //The sensor uses a pulse width of 2.0 ms to indicate no detection. The pulse period T ranges from 
    //around 9 ms to 20 ms, depending on the proximity of the detected object.
    delay(10);
    Serial.println("");
    Serial.println("");
  }
}
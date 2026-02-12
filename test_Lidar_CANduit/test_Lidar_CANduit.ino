/* Use Tools > Board > esp32 and choosing "ESP32S3 Dev Module"
*/
// today goal is to test the potentiometer ontop of the arduino and soder spring connector to each placeholder (im calling the 8 slots placeholders)
// also test the digital ios
// stole of all this from phil branch, thanks phill!
// Pin definitions - each CANduit GPIO is controlled by 3 ESP gpios
//   - one for direction, one to enable power and one for the signal 


// yellow for data line, gpio 0
// blue for clock line, gpio 1
const int GPIO[] = {4, 7, 17, 19, 9, 11, 14, 2};

#include <Wire.h>
#include <VL53L0X.h>

// Define the custom I2C pins for the ESP32
#define I2C_SDA_PIN GPIO[1]
#define I2C_SCL_PIN GPIO[0]

VL53L0X sensor;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); // Sets the baud rate at 115200 to match the software setting
  delay(500); 



  Serial.println("VL53L0X Time-of-Flight Sensor (ESP32 Custom I2C GPIOs)");

  // Initialize the I2C bus with custom pins (SDA, SCL) and frequency
  // The ESP32 Wire library automatically handles setting the pins
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN); 

  // The library uses the default Wire object (Wire)
  // If you need to use the second I2C bus (Wire1), you would call sensor.setBus(&Wire1);
  
  sensor.setTimeout(500);
  if (!sensor.init()) {
    Serial.println(F("Failed to detect and initialize sensor!"));
    while (1);
  }
  
  // Start continuous ranging measurements 
  sensor.startContinuous();
  Serial.println(F("Sensor initialized and continuous ranging started."));
}

void loop() {
  // Read distance in millimeters
  uint16_t distance = sensor.readRangeContinuousMillimeters();

  if (sensor.timeoutOccurred()) { 
    Serial.println("Sensor TIMEOUT"); 
  } else if (distance >= 2000) { // VL53L0X max range is about 2 meters
    Serial.println("Out of range (>2000 mm)");
  } else {
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" mm");
  }

  delay(100); // Adjust the delay as needed for your application
}
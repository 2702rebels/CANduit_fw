/* Use Tools > Board > esp32 and choosing “ESP32S3 Dev Module”

*/
// GPIO0 = 4, PWR0 = 5, DIR0 = 6
// what the flip does this segment do? Check diagram
// need to test the switch and blue led
// Boston dinamics, new robot check it out
const int GPIO[] = {4, 7, 17, 19, 9, 11, 14, 2};
const int PWR[] = {5, 15, 18, 3, 10, 13, 45, 1};
const int DIR[] = {6, 16, 8, 20, 46, 12, 48, 38};

static int nums_of_loop = 0;

const int switchPin = 41; 
static int switch_status = 0;

void setup() {

// put your setup code here, to run once:

  Serial.begin(115200); // Sets the baud rate at 9600 to match the software setting, this magic number is needed
  delay(1000);
  
  for (int g=0; g<8; g++)
  {
    pinMode(GPIO[g], OUTPUT);

    pinMode(PWR[g], OUTPUT);

    pinMode(DIR[g], OUTPUT);

    digitalWrite(GPIO[g], HIGH);

    digitalWrite(PWR[g], HIGH);

    digitalWrite(DIR[g], HIGH);


  }

  pinMode(42, OUTPUT);

  pinMode(switchPin, INPUT_PULLUP); 

  Serial.println("Running\n"); // Prints “Hello World!” followed by a new line


}



void loop() {
  
// put your main code here, to run repeatedly:
  Serial.printf("Looped items below %d\n", nums_of_loop);

  // Checking if switch on or off
  Serial.printf("Switch is %d\n", switch_status);
  

  delay(333);
  for (int g=0; g<8; g++)
  {
    
    digitalWrite(GPIO[g], HIGH);
    Serial.printf("%d On \n",g );

    delay(400); 

    digitalWrite(GPIO[g], LOW);
    
    Serial.printf("%d Off\n",g );

    if (g==7)
    {
      Serial.println("Looped through all LED\n");
      digitalWrite(42, HIGH);
      Serial.println("Blue LED on");
      delay(500);
      digitalWrite(42, LOW);
      Serial.println("Blue LED off");
    }
    delay(400);

  }
  for (int g=0; g<9; g++){
    digitalWrite(GPIO[g], HIGH);
  }
  nums_of_loop+=1;

  switch_status = digitalRead(switchPin);
  Serial.printf("%d\n", switch_status);

  // Print the state to the Serial Monitor
  Serial.println(switch_status);

  // Check if the button is pressed (it will be LOW because of the pull-up resistor)
  if (switch_status == LOW){
    //Serial.printf("%d\n", switch_status);
    digitalWrite(42, HIGH);

    // Code to run when the button is pressed
    // Example: turn on an LED
    // digitalWrite(LED_PIN, HIGH);

  } else {
    // Code to run when the button is not pressed (HIGH)
    // Example: turn off an LED
    // digitalWrite(LED_PIN, LOW);
    //Serial.printf("%d\n", switch_status);
    digitalWrite(42, LOW);
    }
}

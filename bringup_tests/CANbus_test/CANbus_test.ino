/* ESP32 TWAI version of the CAN doctor.
 This code will listen for new devices and output all found.

 - based on TWAIreceive example in the library

  Connect a CAN bus transceiver to the RX/TX pins.

  Set Board to "ESP32S3 Dev module" and PSRAM to disabled for this test
      
   
  See the REV protocol docs here 
         https://drive.google.com/drive/folders/1gfLjioWWnCgHdTAkMsPzkNUPPZ9W7C-F
*/

#include "driver/twai.h"

// **********************
// Constants
// **********************

#define POLLING_RATE_MS 100

static const String Manufacturers[] =
{
  { "B-cast" },
  { "NI" },
  { "TI" },
  { "DEKA" },
  { "CTRE" },
  { "REV" },
  { "Grapple" },
  { "MS" },
  { "Team" },
  { "Kauai" },
  { "CF" },
  { "PWF" },
  { "Studica" },
  { "Thrify" },
  { "Redux" },
  { "AndyM" },
  { "Vivid" },
};
#define NUM_MANUFACTURERS   17

static const String DeviceTypes[] =
{
  { "Bcast" }, // Limit names to 5 char to fit on screen
  { "Robo " },
  { "Motor" },
  { "Relay" },
  { "Gyro " },
  { "Accel" },
  { "Ultra" },
  { "Gear " },
  { "Power" },
  { "Pnuem" },
  { "Misc " },
  { "IO BO" },
  { "Servo" },
  //{ 0x1f, "Firmware Update" }
};
#define NUM_DEV_TYPES   13

// Pins used to connect to CAN bus transceiver:
#define RX_PIN 47
#define TX_PIN 21

#define NOT_PERIPH_EN   17

// The max number of devices that can be uniquely identified on the FRC CAN bus
// is larger, but I think this is a generous practical limit
#define MAX_NUM_DEVICES     32
#define SCROLL_BAR_HEIGHT   ((160-16)*19/MAX_NUM_DEVICES)


// **********************
// Types
// **********************
typedef struct
{
  int number;
  int manuf;
  int type;
} device_ty;



// **********************
// Globals
// **********************


bool DemoMode = true;
bool DriverInstalled = false;
int NumDevices = 0;
device_ty Devices[MAX_NUM_DEVICES];
int MaxRx[MAX_NUM_DEVICES]; // max rx count for each device
uint32_t BusErrCnt = 0;
uint32_t RxMissedCnt = 0;
uint32_t RxOverrunCnt = 0;
int32_t  RotCount = 0; 
int LastInterrupt = millis();
int ErrCount = 0;
bool Paused = false;


    
// **********************
// Functions
// **********************


// Convert device type number to human readable string
//  - handle the values that are not in the lookup table (https://docs.wpilib.org/en/stable/docs/software/can-devices/can-addressing.html)
const char *dev_type_str(int dev_type)
{
  if (dev_type < NUM_DEV_TYPES)
  {
    return(DeviceTypes[dev_type].c_str());
  }
  if (dev_type <= 30)
  {
    return("RSVD ");
  }
  if (dev_type == 31)
  {
    return("FWUPD");
  }
  return("---- ");
}


// Convert manufacturer number to human readable string
//  - handle the values that are not in the lookup table (https://docs.wpilib.org/en/stable/docs/software/can-devices/can-addressing.html)
const char *manuf_str(int manuf)
{
  if (manuf < NUM_MANUFACTURERS)
  {
    return(Manufacturers[manuf].c_str());
  }
  if (manuf <= 255)
  {
    return("RSVD ");
  }
  return("---- ");
}



void scan_devices()
{
  long unsigned int rxId;
  int dev_num;
  int index;
  int api_class;
  int manuf;
  int dev_type;
 
  
  Serial.println("Scanning CAN bus for devices....");
  
  while(1)
  {
    uint32_t alerts_triggered;
    twai_read_alerts(&alerts_triggered, pdMS_TO_TICKS(POLLING_RATE_MS));
    twai_status_info_t twaistatus;
    twai_get_status_info(&twaistatus);

    //Serial.printf("Alert = 0x%x\n", alerts_triggered);

#if 1 
    // Handle alerts
    if (alerts_triggered & TWAI_ALERT_ERR_PASS) {
      Serial.println("Alert: TWAI controller has become error passive.");
    }
    if (alerts_triggered & TWAI_ALERT_BUS_ERROR) {
      //Serial.println("Alert: A (Bit, Stuff, CRC, Form, ACK) error has occurred on the bus.");
      Serial.printf("Bus error count: %d\n", twaistatus.bus_error_count);
      Serial.flush();
    }
    if (alerts_triggered & TWAI_ALERT_RX_QUEUE_FULL) {
      //Serial.println("Alert: The RX queue is full causing a received frame to be lost.");
      //Serial.flush();
      
      //Serial.printf("RX buffered: %d\t", twaistatus.msgs_to_rx);
      //Serial.printf("RX missed: %d\t", twaistatus.rx_missed_count);
      //Serial.printf("RX overrun %d\n", twaistatus.rx_overrun_count);
    }
#endif  

    // Check if message is received
    if (alerts_triggered & TWAI_ALERT_RX_DATA) 
    {
      // One or more messages received. Handle all.
      twai_message_t message;
      while (twai_receive(&message, 0) == ESP_OK) 
      {
        rxId = message.identifier;
        
        // Parse the CAN ID according to FRC standard
        int dev_num = rxId & 0x3F;
        int index = (rxId & 0x3C0) >> 6;
        int api_class = (rxId & 0xFC00) >> 10;
        int manuf = (rxId & 0xFF0000) >> 16;
        int dev_type = (rxId & 0x1F000000) >> 24;
    
        // Is this a new device?
        bool got_it = false;
        for (int i=0; i < NumDevices; i++)
        {
          //Serial.printf("Does %d match the found dev_num %d?\n", Devices[i].number, dev_num); 
          if (Devices[i].number == dev_num && Devices[i].type == dev_type)
          {
            // Already got it - break
            got_it = true;
            break;
          }
        }
        if (!got_it)
        {
          // Check for out of bounds indexes? NumDevices - PHIL
          Serial.printf("New device ID: %d, %s %s\n", dev_num, manuf_str(manuf), dev_type_str(dev_type));
          //Serial.printf("Raw rxID: 0x%x\n", rxId);

          Devices[NumDevices].number = dev_num;
          Devices[NumDevices].manuf = manuf;
          Devices[NumDevices].type = dev_type;
          NumDevices++;
          
          Serial.printf("Found %d devices - Press button to move to next phase\n", NumDevices);
        }
        else
        {
          Serial.printf("Already got device ID: %d, %s %s\n", dev_num, manuf_str(manuf), DeviceTypes[dev_type].c_str());
        }
      } // While msgs
    } // if alert

    yield();
  } // while forever
}



/*************   SETUP   **************/
void setup() {
  // Start Serial:
  Serial.begin(115200);
  //delay(1000); //Take some time to open up the Serial Monitor
  
  // Turn on power to peripherals

  delay(1000);



  // Initialize configuration structures using macro initializers
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)TX_PIN, (gpio_num_t)RX_PIN, TWAI_MODE_LISTEN_ONLY);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_1MBITS(); 
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  // Install TWAI driver
  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    Serial.println("Driver installed");
  } else {
    Serial.println("Failed to install driver");
    return;
  }

  // Start TWAI driver
  if (twai_start() == ESP_OK) {
    Serial.println("Driver started");
  } else {
    Serial.println("Failed to start driver");
    return;
  }

  // Reconfigure alerts to detect frame receive, Bus-Off error and RX queue full states
  uint32_t alerts_to_enable = TWAI_ALERT_RX_DATA | TWAI_ALERT_ERR_PASS | TWAI_ALERT_BUS_ERROR | TWAI_ALERT_RX_QUEUE_FULL;
  if (twai_reconfigure_alerts(alerts_to_enable, NULL) == ESP_OK) {
    Serial.println("CAN Alerts reconfigured");
  } else {
    Serial.println("Failed to reconfigure alerts");
    return;
  }

  // TWAI driver is now successfully installed and started
  Serial.println("CAN is good to go");
  DriverInstalled = true;

  scan_devices(); // Build a table of the detected devices on the bus
  
  for (int i=0; i<MAX_NUM_DEVICES; i++)
  {
    MaxRx[i] = 0;
  }
} // Setup




static void handle_rx_message(twai_message_t& message) 
{
  long unsigned int rxId;
  int index;
  static int max_max = 1;
  static int rxCount[1<<11]; // there are 5 bits of type and 6 bits of number in the index
  static unsigned long startTime = millis();
  static int count = 0; // loop count for clearing every 10th time
  
#if 1
  // Process received message
  if (message.extd) {
    Serial.println("Message is in Extended Format");
  } else {
    Serial.println("Message is in Standard Format");
  }
  Serial.printf("ID: %x\nByte:", message.identifier);
  if (!(message.rtr)) {
    for (int i = 0; i < message.data_length_code; i++) {
      Serial.printf(" %d = %02x,", i, message.data[i]);
    }
    Serial.println("");
  }
#endif

  // Parse the CAN ID according to FRC standard
  rxId = message.identifier;
  int dev_num = rxId & 0x3F;
  //int api_index = (rxId & 0x3C0) >> 6;
  //int api_class = (rxId & 0xFC00) >> 10;
  int manuf = (rxId & 0xFF0000) >> 16;
  int dev_type = (rxId & 0x1F000000) >> 24;

  // We want to track how many msgs have been rx'd from the device in the last 100msec
  //  - to do that we would need to keep a list of rx times and discard them when they became more than 100msec old
  //  - what if we calc how long it took to get the last 10 messages?  That still requires maintaining a queue of the last ten tx times
  //  - instead keep a count of how many of each type have been rx'd and report and reset it each 100msec
  index = (dev_type << 6) | dev_num; // combine id and type into a single unique index
  rxCount[index]++;

  // Check if 100msec has elapsed 
  if ((millis() - startTime) < 100)
  {
    return;
  }
    
  //Serial.println();
  for (int i=0; i<NumDevices; i++)
  {
    int manuf = Devices[i].manuf;
    int dev_type = Devices[i].type;
    int dev_num = Devices[i].number;
    index = (dev_type << 6) | dev_num; // combine id and type into a single unique index
    //Serial.printf("%s %s ID %d: count = %d", manuf_str(manuf),  dev_type_str(dev_type), dev_num, rxCount[index]);
    
    // Draw the line on the graph
    if (rxCount[index] > MaxRx[i]) 
    {
      MaxRx[i] = rxCount[index];
      Serial.printf("\nNew max[%d] = %d\n", i, MaxRx[i]);

      if (MaxRx[i] > max_max)
      {
        max_max = MaxRx[i]; // Use a global max to set the scale for all 
      }
    }
    int offset = ((127-8-50-3) * rxCount[index] / max_max) + 1; // scale offset based on max reading for all
    // if this value us unusually low, draw bar in red
    if (rxCount[index] <= (MaxRx[i]/5))
    {
      Serial.print(" !! LOW !!!");
      //Canvas.fillRect(50+offset, i*8+1, 2, 6, ST77XX_RED);
    }
    else
    {
      Serial.print(" Normal");
      //Canvas.fillRect(50+offset, i*8+1, 2, 6, ST77XX_GREEN);
    }
    //Serial.println();
    rxCount[index] = 0; 
  } // for each device
 
  startTime = millis(); // start the next 100 msec timer
}


void loop() 
{
  static bool pushed = true;
  static int next_update = millis() + 500;
  static int deepsleep_time = millis() + 3000;
 
  // PHIL REMOVE THIS and use HACF in setup (write error to TFT) 
  if (!DriverInstalled) {
    // Driver not installed
    Serial.println("Driver not installed");
    delay(1000);
    return;
  }

  // Check if battery status needs to be updated?
  //   - Run a seperate thread that checks the battery status a few times per second?
  //    - we don't want to conflict with header draws from other threads so make it a thread just for drawing the header
  //   - for now this hack will updated it 
  if (millis() > next_update)
  {
    //draw_header();
    next_update = millis()+500;
  }
  
  if (Paused) return;

  uint32_t alerts_triggered;
  twai_read_alerts(&alerts_triggered, pdMS_TO_TICKS(POLLING_RATE_MS));
  twai_status_info_t twaistatus;
  twai_get_status_info(&twaistatus);

  // Handle alerts !!!!
  if (alerts_triggered & TWAI_ALERT_ERR_PASS) {
    Serial.println("Alert: TWAI controller has become error passive.");
  }
  if (alerts_triggered & TWAI_ALERT_BUS_ERROR) {
    //Serial.println("Alert: A (Bit, Stuff, CRC, Form, ACK) error has occurred on the bus.");
    //Serial.printf("Bus error count: %d\n", twaistatus.bus_error_count);
    BusErrCnt = twaistatus.bus_error_count;
  }
  if (alerts_triggered & TWAI_ALERT_RX_QUEUE_FULL) {
    //Serial.println("Alert: The RX queue is full causing a received frame to be lost.");
    //Serial.printf("RX buffered: %d\t", twaistatus.msgs_to_rx);
    //Serial.printf("RX missed: %d\t", twaistatus.rx_missed_count);
    //Serial.printf("RX overrun %d\n", twaistatus.rx_overrun_count);
    RxMissedCnt = twaistatus.rx_missed_count;
    RxOverrunCnt = twaistatus.rx_overrun_count;
  }

  // Check if message is received
  if (alerts_triggered & TWAI_ALERT_RX_DATA) {
    // One or more messages received. Handle all.
    twai_message_t message;
    while (twai_receive(&message, 0) == ESP_OK) {
      handle_rx_message(message);
    }
  }
}

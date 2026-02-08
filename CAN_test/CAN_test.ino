/* 
 This code will listen for CANduit messages and output all found.

 - based on TWAIreceive example in the library

  Set Board to "ESP32S3 Dev module" and PSRAM to disabled for this test
      
   
  See the REV protocol docs here 
         https://drive.google.com/drive/folders/1gfLjioWWnCgHdTAkMsPzkNUPPZ9W7C-F
*/

#include "driver/twai.h"

// **********************
// Constants
// **********************

#define POLLING_RATE_MS 100  // PHIL - is this too slow???

// Pins used to connect to CAN bus transceiver:
#define RX_PIN 47
#define TX_PIN 21

//#define NOT_PERIPH_EN   17




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
bool DriverInstalled = false;
uint32_t BusErrCnt = 0;
uint32_t RxMissedCnt = 0;
uint32_t RxOverrunCnt = 0;
int32_t  RotCount = 0; 
int LastInterrupt = millis();
int ErrCount = 0;


    
// **********************
// Functions
// **********************

// This assumes a 1 byte reply
void send_rtr_reply(uint32_t id, uint8_t val) {
    twai_message_t tx_msg;
    tx_msg.identifier = id;      // Match the requested ID
    tx_msg.extd = 1;            // 0 for Standard, 1 for Extended (FRC is extended?)
    tx_msg.rtr = 0;             // MUST be 0 to send actual data
    tx_msg.data_length_code = 1; // Number of bytes to send - should match the request
    tx_msg.data[0] = val;
#if 0
          twai_message_t tx_msg = {
          .identifier = rx_msg.identifier, // Use requested ID
          .extd = rx_msg.extd,             // Match standard/extended type
          .rtr = 0,                        // Data frame, NOT RTR
          .data_length_code = 4,           // Data size
          .data = {0x11, 0x22, 0x33, 0x44}
      };
#endif

    // Queue for transmission
    esp_err_t rval = twai_transmit(&tx_msg, pdMS_TO_TICKS(POLLING_RATE_MS));
    if (rval == ESP_OK) {
      //Serial.printf("RTR Reply sent for ID: 0x%X\n", id);
    }
    else
    {
      Serial.printf("Failed to send reply: 0x%x\n", rval);
    }
}

void scan_messages()
{
  long unsigned int rxId;
  int dev_num;
  int index;
  int api_class;
  int manuf;
  int dev_type;

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
#if 0
        // Process received message
        if (message.extd) {
          Serial.println("Message is in Extended Format");
        } else {
          Serial.println("Message is in Standard Format");
        }
#endif

#if 0
        if (!(message.rtr)) {
          for (int i = 0; i < message.data_length_code; i++) {
            Serial.printf(" %d = %02x,", i, message.data[i]);
          }
          Serial.println("");
        }
#endif

        // Parse the CAN ID according to FRC standard
        int dev_num = rxId & 0x3F;
        int index = (rxId & 0x3C0) >> 6;
        int api_class = (rxId & 0xFC00) >> 10;
        int manuf = (rxId & 0xFF0000) >> 16;
        int dev_type = (rxId & 0x1F000000) >> 24;

        if ((dev_type == 10) && (manuf == 8))
        {
          //Serial.printf("Got CANduit msg for dev id %d with API Class %d and index %d\n", dev_num, api_class, index);

          if (message.rtr)
          {
            //Serial.printf("Sender is waiting for a reply of length %d\n",  message.data_length_code);
            // Assume 1 byte reply for now
            send_rtr_reply(rxId, 0xA5);
          }
          //Serial.println("Got Device Type 8 - Misc");
          //Serial.println("Got Manufacturer 10 - Team");
        }
        
      } // While msgs
    } // if alert
}

void HACF()
{
  while(true)
  {
    Serial.println("Fatal error");
    delay(2000);
  }
}

/*************   SETUP   **************/
void setup() {
  // Start Serial:
  Serial.begin(115200);
  delay(1000);

  // Initialize configuration structures using macro initializers
  //twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)TX_PIN, (gpio_num_t)RX_PIN, TWAI_MODE_LISTEN_ONLY);
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)TX_PIN, (gpio_num_t)RX_PIN, TWAI_MODE_NORMAL);

  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_1MBITS(); 
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();  // PHIL - change this to filter out just CANduit messages

  // Install TWAI driver
  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    Serial.println("Driver installed");
  } else {
    Serial.println("Failed to install driver");
    //return;
    HACF();
  }

  // Start TWAI driver
  if (twai_start() == ESP_OK) {
    Serial.println("Driver started");
  } else {
    Serial.println("Failed to start driver");
    //return;
    HACF();
  }

  // Reconfigure alerts to detect frame receive, Bus-Off error and RX queue full states
  uint32_t alerts_to_enable = TWAI_ALERT_RX_DATA | TWAI_ALERT_ERR_PASS | TWAI_ALERT_BUS_ERROR | TWAI_ALERT_RX_QUEUE_FULL;
  if (twai_reconfigure_alerts(alerts_to_enable, NULL) == ESP_OK) {
    Serial.println("CAN Alerts reconfigured");
  } else {
    Serial.println("Failed to reconfigure alerts");
    //return;
    HACF();
  }

  // TWAI driver is now successfully installed and started
  Serial.println("CAN is good to go");
  //DriverInstalled = true;

  //scan_devices(); // Build a table of the detected devices on the bus
  Serial.println("Scanning CAN bus for messages....");
} // Setup



#if 0
static void handle_rx_message(twai_message_t& message) 
{
  long unsigned int rxId;
  int index;
  static int max_max = 1;
  static int rxCount[1<<11]; // there are 5 bits of type and 6 bits of number in the index
  static unsigned long startTime = millis();
  static int count = 0; // loop count for clearing every 10th time
  

  
}
#endif

void loop() 
{
  scan_messages();


#if 0
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
#endif
}
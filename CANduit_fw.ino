#include "src/CAN/CAN.h"
#include "src/gpio/gpio.h"
#include "src/device/device.h"
#include "driver/twai.h"

bool disabled = false;

void setup() {

    Serial.begin(115200); // Sets the baud rate at 9600 to match the software setting
    delay(1000);

    setupGPIO();

    // setup TWAI
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
    preferences.begin("CANduit",false);
    deviceID = getDeviceID();

    preferences.end();
}


void loop() {

  delay(10);
  if (disabled) {
    delay(1000);
    return;
  }
  
  // may refactor twai stuff to dedicated file but for now this is good
  uint32_t alerts_triggered;
  twai_read_alerts(&alerts_triggered, pdMS_TO_TICKS(POLLING_RATE_MS));
  twai_status_info_t twaistatus;
  twai_get_status_info(&twaistatus);


  

  if (alerts_triggered & TWAI_ALERT_ERR_PASS) {
    Serial.println("Alert: TWAI controller has become error passive.");
  }

  // Check if message is received
  if (alerts_triggered & TWAI_ALERT_RX_DATA) {
    // One or more messages received. Handle all.
    twai_message_t message;
    while (twai_receive(&message, 0) == ESP_OK) {
      handle_twai_message(message); // Leads to src/CAN/CAN.h 
    }
  }

}

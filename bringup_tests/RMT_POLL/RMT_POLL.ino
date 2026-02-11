#include "driver/rmt_rx.h"
#include "freertos/semphr.h"

const int GPIOPIN[] = {4, 7, 17, 19, 9, 11, 14, 2};
const int PWR[] = {5, 15, 18, 3, 10, 13, 45, 1};
const int DIR[] = {6, 16, 8, 20, 46, 12, 48, 38};

#define RX_PIN            GPIO_NUM_2
#define RMT_RESOLUTION_HZ 10000000 // 80MHz for high precision (12.5ns per tick)
#define IDLE_THRES_NS     2000000  // 2ms idle threshold (fits @ 10MHz)
#define BUFFER_SIZE 48

// 1. GLOBAL & ALIGNED: Use static to ensure the memory address never moves
//static rmt_symbol_word_t syms[48]; 
//static rmt_receive_config_t rx_cfg;
// 1. Force 4-byte alignment for the buffer and config
// 'static' keeps them in a fixed location; 'aligned(4)' prevents the LoadProhibited crash
static DMA_ATTR rmt_symbol_word_t syms[BUFFER_SIZE] __attribute__((aligned(4))); 
static rmt_receive_config_t rx_cfg __attribute__((aligned(4)));

rmt_channel_handle_t rx_chan = NULL;
SemaphoreHandle_t rx_done_sem = NULL;
//size_t last_captured_count = 0;  // REname pulse_count - PHIL
volatile size_t captured_symbols = 0;


void setup() {
  Serial.begin(115200);
  delay(1000);

  for (int g=0; g<8; g++) {
    pinMode(GPIOPIN[g], INPUT);
    pinMode(PWR[g], OUTPUT);
    pinMode(DIR[g], OUTPUT);
    digitalWrite(PWR[g], HIGH);
    digitalWrite(DIR[g], LOW); 
  }

  rmt_rx_channel_config_t rx_chan_config = {
    .gpio_num = RX_PIN,
    .clk_src = RMT_CLK_SRC_DEFAULT,
    .resolution_hz = RMT_RESOLUTION_HZ,
    .mem_block_symbols = 48,
  };
  ESP_ERROR_CHECK(rmt_new_rx_channel(&rx_chan_config, &rx_chan));
  ESP_ERROR_CHECK(rmt_enable(rx_chan));


   // Start Receive with Partial RX Enabled
  memset(&rx_cfg, 0, sizeof(rx_cfg)); // Zero out the struct
  //rx_cfg.signal_range_min_ns = 900;
  rx_cfg.signal_range_max_ns = 2000000;
  rx_cfg.signal_range_min_ns = 0; // is this necessary?
  //rx_cfg.signal_range_max_ns = 0;
  rx_cfg.flags.en_partial_rx = true; 
}


void loop() {
    // Start one-shot capture
    esp_err_t ret = rmt_receive(rx_chan, syms, BUFFER_SIZE, &rx_cfg); // pass num symbols
    if (ret == ESP_OK) {
        delay(100); //PHIL - use a shorter delay?
            
        // Stop the hardware immediately
        rmt_disable(rx_chan); 

        // The driver will NOT trigger the callback when disabled.
        // You must manually check the memory now.
        // Note: Without the callback, you don't know exactly how many 'captured_symbols'.
        // You will have to scan the buffer for the 'end' marker 
        for (int i = 0; i < BUFFER_SIZE; i++) {
            if (syms[i].duration0 == 0 && syms[i].duration1 == 0) {
                break; // Found the end of captured data
            }

            // Process your data here
            uint32_t ticks = (syms[0].level0 == 1) ? syms[0].duration0 : syms[0].duration1;
            float width_us = (float)ticks / 10.0;
            Serial.printf("Manual Read Sym: %.2f us\n", width_us);
            //float width_us = syms[i].duration0 / 10.0;
            //float width1_us = syms[i].duration1 / 10.0;
            //Serial.printf("Manual Read Sym %d: %.2f us  %.2f  level0: %d,  level1: %d\n", i, 
            //  width_us, width1_us, syms[i].level0, syms[i].level1);
        } // for

        // Re-enabling clears the internal driver state.  Clean flush
        //rmt_disable(rx_chan);
        rmt_enable(rx_chan);
        //Serial.printf("Callback count =%d\n", callback_count);
        
    } else {
        Serial.printf("rmt_receive error: %s\n", esp_err_to_name(ret));
    }
    delay(100); 
}

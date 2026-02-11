

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
size_t last_captured_count = 0;  // REname pulse_count - PHIL
volatile size_t captured_symbols = 0;
//static rmt_symbol_word_t rx_symbols[RX_BUFFER_SIZE];

int callback_count = 0;

// This is the "ISR-lite" callback the driver calls when hardware stops
static bool IRAM_ATTR rmt_rx_done_callback(rmt_channel_handle_t channel, const rmt_rx_done_event_data_t *edata, void *user_data) {
    captured_symbols = edata->num_symbols;
    callback_count++;
    //if (captured_symbols > 2) {
      BaseType_t high_task_wakeup = pdFALSE; 
      xSemaphoreGiveFromISR(rx_done_sem, &high_task_wakeup);
      //returning true instructs the CPU to perform a context switch immediately after the ISR finishes, ensuring the unblocked task runs without waiting for the next scheduler tick
      return high_task_wakeup == pdTRUE;
      //return(true);
    //}
    //return(false);
}

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

  rx_done_sem = xSemaphoreCreateBinary();

  rmt_rx_channel_config_t rx_chan_config = {
    .gpio_num = RX_PIN,
    .clk_src = RMT_CLK_SRC_DEFAULT,
    .resolution_hz = RMT_RESOLUTION_HZ,
    .mem_block_symbols = 48,
  };
  ESP_ERROR_CHECK(rmt_new_rx_channel(&rx_chan_config, &rx_chan));

  // Register the callback so we know when the hardware is done
  rmt_rx_event_callbacks_t cbs = { .on_recv_done = rmt_rx_done_callback };
  ESP_ERROR_CHECK(rmt_rx_register_event_callbacks(rx_chan, &cbs, NULL));
  ESP_ERROR_CHECK(rmt_enable(rx_chan));

   // 5. Start Receive with Partial RX Enabled
memset(&rx_cfg, 0, sizeof(rx_cfg)); // Zero out the struct
//rx_cfg.signal_range_min_ns = 900;
rx_cfg.signal_range_max_ns = 2000000;
rx_cfg.signal_range_min_ns = 0; // is this necessary?
//rx_cfg.signal_range_max_ns = 0;
rx_cfg.flags.en_partial_rx = true; 


// Check the actual channel allocated by the driver
//int channel_id;
//rmt_rx_get_channel_id(rx_chan, &channel_id);
//Serial.printf("Allocated Channel: %d\n", channel_id); // Should be 4, 5, 6, or 7

}

// THis is accessed outside of loop
    // Array of 2 symbols to capture both high and low transitions
    // must be 48 bytes to align properly
 // static  rmt_symbol_word_t syms[48]; 

void loop() {
    

    // Reset captured count before starting
    captured_symbols = 0;

    // Start one-shot capture
    //esp_err_t ret = rmt_receive(rx_chan, syms, sizeof(syms), &rx_cfg);
    esp_err_t ret = rmt_receive(rx_chan, syms, BUFFER_SIZE, &rx_cfg); // pass num symbols
    if (ret == ESP_OK) {
        delay(5);


#if 0
            // On S3, RX channels are typically 4, 5, 6, or 7
    // This sets the "Receive End" interrupt bit for the channel
    // effectively tricking the ISR into thinking the transaction is done.
    for (int channel_id = 4; channel_id <= 7; channel_id++) { // Hit all 4 for now
        SET_PERI_REG_MASK(RMT_INT_RAW_REG, (1 << (channel_id * 3 + 1))); 
    }
#endif

        // Wait for hardware to stop (Idle threshold or Buffer full)
        if (xSemaphoreTake(rx_done_sem, pdMS_TO_TICKS(100)) == pdTRUE) {
            if (captured_symbols > 0) {
                // syms[0].level tells us if the first captured part was High (1) or Low (0)
                uint32_t ticks = (syms[0].level0 == 1) ? syms[0].duration0 : syms[0].duration1;
                float width_us = (float)ticks / 10.0;
                Serial.printf("Pulse Width: %.2f us (Duty: %.3f%%)\n", width_us, (width_us / 1000.0) * 100.0);

                for (int i=0; i<captured_symbols; i++)
                {
                  Serial.printf("Sym %d level0 %d  level1 %d  duration0 %d  duration1 %d\n", i, syms[i].level0,
                    syms[i].level1, syms[i].duration0, syms[i].duration1);
                }
            }
        } else {
            Serial.println("Timed out - Resetting State");
              Serial.printf("Callback count =%d\n", callback_count);
        // 1. Stop the hardware immediately
rmt_disable(rx_chan); 

// 2. The driver will NOT trigger the callback when disabled.
// You must manually check the memory now.
// Note: Without the callback, you don't know exactly 'captured_symbols'.
// You will have to scan the buffer for the 'end' marker or use a known size.
for (int i = 0; i < BUFFER_SIZE; i++) {
    if (syms[i].duration0 == 0 && syms[i].duration1 == 0) {
        break; // Found the end of captured data
    }
    // Process your data here
    float width_us = syms[i].duration0 / 10.0;
    Serial.printf("Manual Read Sym %d: %.2f us\n", i, width_us);
}



            // If it times out, the hardware might still be 'armed'.
                        // 4. CRITICAL: Cancel the pending receive before exiting this loop iteration
            // This prevents the ISR from writing to rx_buffer while loop() is sleeping
            //rmt_receive(rx_chan, NULL, 0, NULL); 

            // Re-enabling clears the internal driver state.  Clean flush
            rmt_disable(rx_chan);
            rmt_enable(rx_chan);
            Serial.printf("Callback count =%d\n", callback_count);
        }
    } else {
        Serial.printf("rmt_receive error: %s\n", esp_err_to_name(ret));
    }

    delay(1000); 
}


#if 0
void loop() {
  rmt_symbol_word_t raw_symbols[64];
  rmt_receive_config_t rx_cfg = { .signal_range_max_ns = 2000000 }; // 2ms Idle stop

  // 1. Start the one-shot capture
  ESP_ERROR_CHECK(rmt_receive(rx_chan, raw_symbols, sizeof(raw_symbols), &rx_cfg));

  // 2. Wait (block) for the callback to trigger the semaphore
  if (xSemaphoreTake(rx_done_sem, pdMS_TO_TICKS(100)) == pdTRUE) {
    if (last_captured_count > 0) {
      // Precise conversion: (ticks / 80) = microseconds
      float width_us = (float)raw_symbols[0].duration0 / 10.0;
      Serial.printf("Captured %d symbols. Pulse Width: %.3f us\n", last_captured_count, width_us);
    }
  } else {
    Serial.println("Timed out waiting for pulse...");
  
    
  }

  delay(1000); 
}
#endif


#include <Arduino.h>
#include "stdint.h"
#include "./gpio.h"
#include "driver/rmt_rx.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

volatile uint32_t period[8];
volatile uint32_t highTime[8];
volatile uint32_t lowTime[8];

#define RMT_RESOLUTION_HZ 10000000 // 80MHz for high precision (12.5ns per tick)
#define IDLE_THRES_NS     2000000  // 2ms idle threshold (fits @ 10MHz)
#define BUFFER_SIZE 48

void PWMTask(void *pvParameters);

// 1. GLOBAL & ALIGNED: Use static to ensure the memory address never moves
//static rmt_symbol_word_t syms[48]; 
//static rmt_receive_config_t rx_cfg;
// 1. Force 4-byte alignment for the buffer and config
// 'static' keeps them in a fixed location; 'aligned(4)' prevents the LoadProhibited crash
static DMA_ATTR rmt_symbol_word_t syms[BUFFER_SIZE] __attribute__((aligned(4))); 
static rmt_receive_config_t rx_cfg __attribute__((aligned(4)));

//rmt_channel_handle_t rx_chan[8]; 
rmt_channel_handle_t rx_chan; 
SemaphoreHandle_t rx_done_sem = NULL;
//size_t last_captured_count = 0;  // REname pulse_count - PHIL
volatile size_t captured_symbols = 0;


void PWMSetup() {
    Serial.println("In PWMSetup");

    // Start Receive with Partial RX Enabled
    memset(&rx_cfg, 0, sizeof(rx_cfg)); // Zero out the struct
    //rx_cfg.signal_range_min_ns = 900;
    rx_cfg.signal_range_max_ns = 2000000;
    rx_cfg.signal_range_min_ns = 0; // is this necessary?
    //rx_cfg.signal_range_max_ns = 0;
    rx_cfg.flags.en_partial_rx = true;

    TaskHandle_t pwm_task;
    xTaskCreate(PWMTask, "PWMTask",4096,NULL,0, &pwm_task); // PHIL - enlarge stack for printf?  Try going back to 2048
}


void PWMTask(void *pvParameters) {
    // Start one-shot capture
    Serial.println("PWMTask started");


    while (1){
        
        for (int portId = 0; portId<7; portId++){
            Port* port = getGPIO(portId);  
            if (port == NULL){
		Serial.printf("Port %d not initialized?\n", portId);
	    }

            if (port->mode != GPIOMode.PWM_IN) continue; 

    	    // the hardware only supports up to 4 channels at a time so we switch channels
	    // between each GPIO
            rmt_rx_channel_config_t rx_chan_config = {
                .gpio_num = (gpio_num_t)GPIO[port->id],
                .clk_src = RMT_CLK_SRC_DEFAULT,
                .resolution_hz = RMT_RESOLUTION_HZ,
                .mem_block_symbols = 48,
            };
            esp_err_t err = rmt_new_rx_channel(&rx_chan_config, &rx_chan);  
            if (err != ESP_OK || rx_chan == NULL) {
                Serial.printf("Failed to create RMT channel: %s\n", esp_err_to_name(err));
                continue; // Don't try to use a null channel!
            }

            ESP_ERROR_CHECK(rmt_enable(rx_chan));
	    delay(10); // PHIL - remove?
            
	    Serial.println("calling receive");
            esp_err_t ret = rmt_receive(rx_chan, syms, BUFFER_SIZE, &rx_cfg); // pass num symbols
            if (ret == ESP_OK) {
                delay(100); //PHIL - use a shorter delay?
                    
                // Stop the hardware immediately
                rmt_disable(rx_chan);
                //rmt_del_channel(rx_chan);

                // The driver will NOT trigger the callback when disabled.
                // You must manually check the memory now.
                // Note: Without the callback, you don't know exactly how many 'captured_symbols'.
                // You will have to scan the buffer for the 'end' marker 
                for (int i = 0; i < BUFFER_SIZE; i++) {
                    if (syms[i].duration0 == 0 && syms[i].duration1 == 0) {
			//Serial.println("End of captured data");
                        break; // Found the end of captured data
                    }
		//Serial.println("process data");
                    // Process your data here
                    uint32_t ticks = (syms[0].level0 == 1) ? syms[0].duration0 : syms[0].duration1;
                    float width_us = (float)ticks / 10.0;
                    //Serial.printf("Manual Read Sym: %.2f us\n", width_us);
                    
                    // If fallTime;
                    if (syms[0].level0 == 1) {
                        highTime[port->id] = syms[0].duration0*100; // *100 to convert to nsec - switch to #define PHIL
                        lowTime[port->id] = syms[0].duration1*100;
                    } else {
                        highTime[port->id] = syms[0].duration1*100;
                        lowTime[port->id] = syms[0].duration0*100;
                    }
                    period[port->id] = (syms[0].duration0 + syms[0].duration1)*100;
		    //Serial.printf("high time %d\n", highTime[port->id]);
                } 
                
            } else {
                Serial.printf("rmt_receive error: %s\n", esp_err_to_name(ret));
            }
	    // Do this after reading out the data
	    rmt_del_channel(rx_chan);

	} // for
        delay(100); 
    } // while
}

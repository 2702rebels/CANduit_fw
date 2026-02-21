#include <Arduino.h>
#include "stdint.h"
#include "gpio.h"
#include "driver/rmt_rx.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "driver/mcpwm_cap.h"
#include "hal/mcpwm_ll.h"

//#define NSEC_PER_TICK       (1000/80)  /* 80MHz clock is 12.5nsec per tick but this rounds down to 12 !!*/

volatile uint32_t period[8];   // Why are these volatile?
volatile uint32_t highTime[8];
volatile uint32_t lowTime[8];
volatile uint32_t samplePeriod = 100; 

// PHIL - switch globals to camel case
volatile uint32_t period_ticks = 0; 
volatile uint32_t rise_tick = 0;
volatile uint32_t old_rise_tick = 0;
volatile uint32_t pulse_width_ticks = 0;
volatile int edge_count = 0;

// Semaphore used to signal when one full cycle has been measured (2 rising edges are reached)
SemaphoreHandle_t cycle_semaphore = NULL;


void PWMTask(void *pvParameters);

// Callback for Channel A: Rising Edge
static bool IRAM_ATTR on_rise_capture(mcpwm_cap_channel_handle_t cap_chan, const mcpwm_capture_event_data_t *edata, void *user_data) {
    rise_tick = edata->cap_value;
    period_ticks = rise_tick - old_rise_tick;
    old_rise_tick = rise_tick;

    BaseType_t high_task_wakeup = pdFALSE;
    edge_count++;
    if (edge_count == 2) {
        //edge_count = 0;
        // Signal the main task
        xSemaphoreGiveFromISR(cycle_semaphore, &high_task_wakeup);
    }
    return high_task_wakeup == pdTRUE;
}

// Callback for Channel B: Falling Edge
static bool IRAM_ATTR on_fall_capture(mcpwm_cap_channel_handle_t cap_chan, const mcpwm_capture_event_data_t *edata, void *user_data) {
    // MCPWM_UNIT_0 and MCPWM_SELECT_CAP0 depending on your setup.
    uint32_t t_rise = mcpwm_ll_capture_get_value(&MCPWM0, 0);  // The t_rise from the other callback can be set too late for very narrow pulses
    uint32_t t_fall = edata->cap_value;
    uint32_t width = t_fall - t_rise;
    // Check if rise time came after fall and if so use the rise time from the other callback
    if (width > 0x7FFFFFFF) {
        // Use the t_rise from the rise time callback
        pulse_width_ticks = t_fall - rise_tick;
    }
    else {
        pulse_width_ticks =  width;
    }
    return false;
}

// Global handles
mcpwm_cap_timer_handle_t cap_timer = NULL;
mcpwm_cap_channel_handle_t rise_chan = NULL;
mcpwm_cap_channel_handle_t fall_chan = NULL;

void PWMSetup() {
    Serial.println("In PWMSetup");

    // Configure Timer
    mcpwm_capture_timer_config_t timer_conf = {};
    timer_conf.group_id = 0;
    timer_conf.clk_src = MCPWM_CAPTURE_CLK_SRC_DEFAULT;
    mcpwm_new_capture_timer(&timer_conf, &cap_timer);

    // Enable and Start
    mcpwm_capture_timer_enable(cap_timer);
    mcpwm_capture_timer_start(cap_timer);

    cycle_semaphore = xSemaphoreCreateBinary();

    TaskHandle_t pwm_task;
    xTaskCreate(PWMTask, "PWMTask", 4096, NULL, 0, &pwm_task); // PHIL - enlarge stack for printf?  Try going back to 2048
}


void PWMTask(void *pvParameters) {
    Serial.println("PWMTask started");

    while (true) {
        unsigned long start_time = millis();

        for (int portId = 0; portId<7; portId++) {
            Port* port = getGPIO(portId);  
            if (port == NULL){
		        Serial.printf("Port %d not initialized?\n", portId);
	        }

            if (port->mode != GPIOMode.PWM_IN) continue; 

    	    // Configure Rising Channel for this port
            mcpwm_capture_channel_config_t rise_conf = {};
            rise_conf.gpio_num = GPIO[portId];
            rise_conf.prescale = 1;
            rise_conf.flags.pos_edge = true; // Use 1 for true in some versions
            mcpwm_new_capture_channel(cap_timer, &rise_conf, &rise_chan);

            // Configure Falling Channel
            mcpwm_capture_channel_config_t fall_conf = {};
            fall_conf.gpio_num = GPIO[portId];
            fall_conf.prescale = 1;
            fall_conf.flags.neg_edge = true;
            mcpwm_new_capture_channel(cap_timer, &fall_conf, &fall_chan);

            // Re-Register Callbacks (using named variables to avoid rvalue error)
            mcpwm_capture_event_callbacks_t rise_cbs = {};
            rise_cbs.on_cap = on_rise_capture;
            mcpwm_capture_channel_register_event_callbacks(rise_chan, &rise_cbs, NULL);

            mcpwm_capture_event_callbacks_t fall_cbs = {};
            fall_cbs.on_cap = on_fall_capture;
            mcpwm_capture_channel_register_event_callbacks(fall_chan, &fall_cbs, NULL);
        
            // Enable channels
            mcpwm_capture_channel_enable(rise_chan);
            mcpwm_capture_channel_enable(fall_chan);

            // Wait for at least one full period - with timeout
            // Convert timeout to system ticks
            TickType_t timeout_ticks = pdMS_TO_TICKS(samplePeriod); 
            edge_count = 0; // Wait for 2 edges
            if (xSemaphoreTake(cycle_semaphore, timeout_ticks) == pdTRUE) {
                //Serial.println("Success: 2 edges detected ..");
                highTime[portId] = (pulse_width_ticks * 1000) / 80; // Assumes 80MHz
                period[portId] = (period_ticks * 1000) / 80;
                lowTime[portId] = ((period_ticks - pulse_width_ticks) * 1000) / 80;
                //if (portId == 2) Serial.printf("port %d, high time %d, period %d, low time %d \n", port->id, 
                //                highTime[portId], period[portId], lowTime[portId]);
            }    
            else {
                // No pulse received so send -1
                highTime[portId] = 0xFFFFFFFF;
                period[portId] = 0xFFFFFFFF;
                lowTime[portId] = 0xFFFFFFFF;
            }   
                
            // Disable and delete channels
            mcpwm_capture_channel_disable(rise_chan);
            mcpwm_capture_channel_disable(fall_chan);
            mcpwm_del_capture_channel(rise_chan);
            mcpwm_del_capture_channel(fall_chan);
	    } // for

        // delay only long enough to complete sample period
        if ((millis() - start_time) < samplePeriod)
        {
            delay(samplePeriod - (millis() - start_time));
        }
        //delay(samplePeriod); 
    } // while
}

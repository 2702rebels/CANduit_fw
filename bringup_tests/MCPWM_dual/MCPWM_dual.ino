/*
  MCPWM is slower than RMT but allows for low latency measurement of slow pulses
  Using two channels allows us to capture both edges of sub usec pulses

*/
//#include "driver/mcpwm_prelude.h"

#include "driver/mcpwm_cap.h"
#include "hal/mcpwm_ll.h"


const int GPIOPIN[] = {4, 7, 17, 19, 9, 11, 14, 2};
const int PWR[] = {5, 15, 18, 3, 10, 13, 45, 1};
const int DIR[] = {6, 16, 8, 20, 46, 12, 48, 38};

#define CAPTURE_GPIO GPIOPIN[6]  // Capture PWM on CANduit channel 6

mcpwm_cap_timer_handle_t cap_timer;
mcpwm_cap_channel_handle_t rise_chan;
mcpwm_cap_channel_handle_t fall_chan;

//mcpwm_cap_channel_handle_t cap_chan = NULL;
volatile uint32_t period = 0;
volatile uint32_t rise_tick = 0;
volatile uint32_t old_rise_tick = 0;
volatile uint32_t pulse_width_ticks = 0;


// Callback for Channel A: Rising Edge
static bool IRAM_ATTR on_rise_capture(mcpwm_cap_channel_handle_t cap_chan, const mcpwm_capture_event_data_t *edata, void *user_data) {
    rise_tick = edata->cap_value;
    period = rise_tick - old_rise_tick;
    old_rise_tick = rise_tick;
    return false;
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

void setup() {
    Serial.begin(115200);
    delay(1000);

    // Init the ESP gpios and configure the initial state for the CANduit gpios
    //  to digital inputs
    for (int g=0; g<8; g++)
    {
        pinMode(GPIOPIN[g], INPUT);
        pinMode(PWR[g], OUTPUT);
        pinMode(DIR[g], OUTPUT);

        //digitalWrite(GPIOPIN[g], HIGH);
        digitalWrite(PWR[g], HIGH);
        digitalWrite(DIR[g], LOW); // Set direction to input
    }

    // 1. Configure Timer
    mcpwm_capture_timer_config_t timer_conf = {};
    timer_conf.group_id = 0;
    timer_conf.clk_src = MCPWM_CAPTURE_CLK_SRC_DEFAULT;
    mcpwm_new_capture_timer(&timer_conf, &cap_timer);

    // 2. Configure Rising Channel
    mcpwm_capture_channel_config_t rise_conf = {};
    rise_conf.gpio_num = CAPTURE_GPIO;
    rise_conf.prescale = 1;
    rise_conf.flags.pos_edge = true; // Use 1 for true in some versions
    mcpwm_new_capture_channel(cap_timer, &rise_conf, &rise_chan);

    // 3. Configure Falling Channel
    mcpwm_capture_channel_config_t fall_conf = {};
    fall_conf.gpio_num = CAPTURE_GPIO;
    fall_conf.prescale = 1;
    fall_conf.flags.neg_edge = true;
    mcpwm_new_capture_channel(cap_timer, &fall_conf, &fall_chan);

    // 4. Register Callbacks (using named variables to avoid rvalue error)
    mcpwm_capture_event_callbacks_t rise_cbs = {};
    rise_cbs.on_cap = on_rise_capture;
    mcpwm_capture_channel_register_event_callbacks(rise_chan, &rise_cbs, NULL);

    mcpwm_capture_event_callbacks_t fall_cbs = {};
    fall_cbs.on_cap = on_fall_capture;
    mcpwm_capture_channel_register_event_callbacks(fall_chan, &fall_cbs, NULL);

    // 5. Enable and Start
    mcpwm_capture_timer_enable(cap_timer);
    mcpwm_capture_timer_start(cap_timer);
    mcpwm_capture_channel_enable(rise_chan);
    mcpwm_capture_channel_enable(fall_chan);
} // setup

void loop() {
    Serial.printf("Pulse Width: %.2f us  Period: %.2f us\n", pulse_width_ticks/80.0, period/80.0); // Assuming 80Mhz clock
    pulse_width_ticks = 0; 
    period = 0;
    delay(100);
}


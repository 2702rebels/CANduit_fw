#ifndef CAN_H
#define CAN_H

#include <cstdint>
#include <stdint.h>
#include "driver/twai.h"

// Pins used to connect to CAN bus transceiver:
#define RX_PIN 47
#define TX_PIN 21

// Header constant values to check if addressed to 
#define BROADCAST 0;
#define MANUFACTURER 8;
#define DEVICE_TYPE 10;

struct [[gnu::packed]] CANHeader {
    uint32_t devNum:5;
    uint32_t apiIndex:4;
    uint32_t apiClass:6;
    uint32_t manuf:8;
    uint32_t devType:5;
};

constexpr struct [[gnu::packed]] {
    const uint32_t broadcast = 0;
    const uint32_t mode = 1;
    const uint32_t digState = 2;
    const uint32_t period = 3;
    const uint32_t highTime = 4;
    const uint32_t lowTime = 5;
} APIClass;

void handle_twai_message(twai_message_t);
uint32_t get_int_from_message(uint8_t (*data)[8], int startByte, int endByte);

#endif

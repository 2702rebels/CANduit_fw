#ifndef CAN_H
#define CAN_H

#include <cstdint>
#include <stdint.h>
#include "driver/twai.h"
#include "array"

#define POLLING_RATE_MS 100

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
    const uint32_t pwm = 3;
} APIClass;

void handle_twai_message(twai_message_t);
uint32_t get_int_from_message(uint8_t (*data)[8], int startByte, int endByte);
std::array<uint8_t,8> get_message_from_int(uint32_t dataInt);
void send_rtr_reply(uint32_t rtrID, int DLC, std::array<uint8_t,8> data);

#endif

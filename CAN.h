#ifndef CAN_H
#define CAN_H

#include <cstdint>
#include <stdint.h>
#include "driver/twai.h"
#include "array"
#include "vector"
#include "bitset"

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


extern uint32_t broadcastPeriod;

/** Custom implementation of the ByteBuffer class from java for managing bits
 */
class PackedBuffer{    
    
    public:
        static PackedBuffer wrap(uint8_t (*data)[8]);
        static PackedBuffer wrap(unsigned long data, int bitlength);
        PackedBuffer();

        void putBits(int bits, int data);
        void putBool(bool val);
        void putByte(uint8_t byte);
        void putWord(uint32_t word);

        unsigned int consumeBits(int bits);
        bool consumeBool();
        uint8_t consumeByte();
        uint32_t consumeWord();
 
    private:
        //We only need a bitset of 64 for this implementation, so it can stay more efficiently as a long. Anything above should never be needed.
        long buf;

        // Keeps track of the current index to add the lsb of a number to in the buffer
        int cursor;
};



void setupBroadcast(); 
void handle_twai_message(twai_message_t);
void send_data_frame(long unsigned int identifier, int DLC, PackedBuffer* data);



# endif

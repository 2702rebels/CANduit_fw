#include "src/CAN/CAN.h"
#include "stdint.h"
#include "driver/twai.h"
#include <array>



/** @brief given a little endian array, gets a uint32_t from a range of bytes, indexed at 0
 * @param startByte inclusive starting byte
 * @param endByte inclusive ending byte
 */

uint32_t get_int_from_message(uint8_t (*data)[8], int startByte, int endByte){
    if (startByte < 0 || endByte > 8 || endByte-startByte >= 4){
        Serial.print("Attempted to retrieve non-existant bytes from message");
        return 0;
    }

    uint32_t result = 0;
    for (int i = endByte; i>=startByte;i--){ 
        result <<= 8; 
        result += (*data)[i]; 
    }
    

    return result;
}

/**@brief returns a little endian message containing a number up to 4 bytes.
 * 
 */
std::array<uint8_t,8> get_message_from_int(uint32_t dataInt) {
    // function could be made more flexible, to perhaps support multiple arguments, but functions for our purposes
    std::array<uint8_t, 8> data{};
    for (int i = 0; i<4; i++){
        uint8_t dataByte = dataInt & 0xFF;
        data[i] = dataByte;
        dataInt >>= 8;
    }

    return data;
}


/** @brief sends an rtr reply in little endian
 */
void send_rtr_reply(uint32_t rtrID, std::array<uint8_t,8> data){
    twai_message_t tx_msg;
    tx_msg.identifier = rtrID;      // Match the requested ID
    tx_msg.extd = 1;            // 0 for Standard, 1 for Extended (FRC is extended?)
    tx_msg.rtr = 0;             // MUST be 0 to send actual data
    tx_msg.data_length_code = 1; // Number of bytes to send - should match the request
    for (int i = 0; i<8;i++)
        tx_msg.data[i] = data[i];


    esp_err_t rval = twai_transmit(&tx_msg, pdMS_TO_TICKS(POLLING_RATE_MS));
    if (rval != ESP_OK) {
      Serial.printf("Failed to send reply: 0x%x\n", rval);
    }
    
}

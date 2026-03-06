/**
 * @file isotp.h
 * @brief ISO-TP (ISO 15765-2) Transport Layer
 */

#ifndef ISOTP_H
#define ISOTP_H

#include <Arduino.h>
#include "driver/twai.h"
#include "config.h"
#include "can_interface.h"

class ISOTP {
public:
    ISOTP();
    
    /**
     * @brief Initialisiert ISO-TP
     * @param can_interface Referenz zum CAN-Interface
     */
    void begin(CANInterface& can_interface);
    
    /**
     * @brief Sendet einen UDS-Request via ISO-TP
     * @param request_id CAN-ID für Request
     * @param response_id CAN-ID für Response (wird gespeichert)
     * @param data Nutzdaten
     * @param len Länge der Nutzdaten
     * @param timeout_ms Timeout
     * @return true bei Erfolg
     */
    bool sendRequest(uint32_t request_id, uint32_t response_id, 
                     const uint8_t* data, size_t len, uint32_t timeout_ms);
    
    /**
     * @brief Empfängt eine ISO-TP-Response
     * @param data Output-Buffer für Nutzdaten
     * @param len Output: Länge der Nutzdaten
     * @param timeout_ms Timeout
     * @return true bei Erfolg
     */
    bool receiveResponse(uint8_t* data, size_t& len, uint32_t timeout_ms);

private:
    CANInterface* can_interface_;
    uint32_t expected_response_id_;
    
    // Single-Frame senden
    bool sendSingleFrame(uint32_t can_id, const uint8_t* data, size_t len);
    
    // Multi-Frame empfangen
    bool receiveMultiFrame(uint8_t* data, size_t& len, uint16_t total_len, uint32_t timeout_ms);
    
    // Flow Control senden
    bool sendFlowControl(uint32_t can_id, uint8_t status, uint8_t block_size, uint8_t st_min);
};

#endif // ISOTP_H

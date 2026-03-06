/**
 * @file can_decoder.h
 * @brief Dekodierung passiver CAN-Frames
 */

#ifndef CAN_DECODER_H
#define CAN_DECODER_H

#include <Arduino.h>
#include "driver/twai.h"
#include "config.h"
#include "telemetry.h"

class CANDecoder {
public:
    CANDecoder();
    
    /**
     * @brief Initialisiert den Decoder
     * @param telemetry Referenz zum Telemetrie-Manager
     */
    void begin(TelemetryManager& telemetry);
    
    /**
     * @brief Dekodiert einen empfangenen CAN-Frame
     * @param frame Empfangener Frame
     */
    void decodeFrame(const twai_message_t& frame);

private:
    TelemetryManager* telemetry_;
    
    // Decoder-Funktionen für bekannte Frames
    void decodeFrame_0x427(const twai_message_t& frame);
    void decodeFrame_0x5D7(const twai_message_t& frame);
};

#endif // CAN_DECODER_H

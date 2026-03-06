/**
 * @file can_interface.h
 * @brief Low-Level CAN/TWAI-Interface für ESP32-C6
 */

#ifndef CAN_INTERFACE_H
#define CAN_INTERFACE_H

#include <Arduino.h>
#include "driver/twai.h"
#include "config.h"

class CANInterface {
public:
    CANInterface();
    
    /**
     * @brief Initialisiert das TWAI-Interface
     * @return true bei Erfolg, false bei Fehler
     */
    bool begin();
    
    /**
     * @brief Empfängt einen CAN-Frame (non-blocking)
     * @param frame Empfangener Frame (Output)
     * @param timeout_ms Timeout in Millisekunden (0 = non-blocking)
     * @return true wenn Frame empfangen, false bei Timeout/Fehler
     */
    bool receive(twai_message_t& frame, uint32_t timeout_ms = 0);
    
    /**
     * @brief Sendet einen CAN-Frame
     * @param frame Zu sendender Frame
     * @param timeout_ms Timeout in Millisekunden
     * @return true bei Erfolg, false bei Fehler
     */
    bool send(const twai_message_t& frame, uint32_t timeout_ms = 100);
    
    /**
     * @brief Prüft ob Bus aktiv ist
     * @return true wenn Bus OK, false bei Bus-Off
     */
    bool isBusHealthy();
    
    /**
     * @brief Versucht Bus-Recovery bei Bus-Off
     * @return true bei Erfolg
     */
    bool recoverBus();

private:
    bool initialized_;
    uint32_t error_count_;
    uint32_t last_error_time_;
};

#endif // CAN_INTERFACE_H

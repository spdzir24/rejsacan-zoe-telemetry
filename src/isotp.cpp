/**
 * @file isotp.cpp
 * @brief Implementierung des ISO-TP Transport Layers
 */

#include "isotp.h"

// ISO-TP Frame-Typen (obere 4 Bits von Byte 0)
#define ISOTP_FRAME_TYPE_SINGLE       0x00
#define ISOTP_FRAME_TYPE_FIRST        0x10
#define ISOTP_FRAME_TYPE_CONSECUTIVE  0x20
#define ISOTP_FRAME_TYPE_FLOW_CONTROL 0x30

// Flow Control Status
#define ISOTP_FC_STATUS_CTS      0x00  // ContinueToSend
#define ISOTP_FC_STATUS_WAIT     0x01  // Wait
#define ISOTP_FC_STATUS_OVERFLOW 0x02  // Overflow

ISOTP::ISOTP() : can_interface_(nullptr), expected_response_id_(0) {
}

void ISOTP::begin(CANInterface& can_interface) {
    can_interface_ = &can_interface;
}

bool ISOTP::sendRequest(uint32_t request_id, uint32_t response_id, 
                        const uint8_t* data, size_t len, uint32_t timeout_ms) {
    if (!can_interface_) {
        return false;
    }

    expected_response_id_ = response_id;

    // Single-Frame: Länge ≤ 7 Bytes
    if (len <= ISOTP_SINGLE_FRAME_MAX_LEN) {
        return sendSingleFrame(request_id, data, len);
    }

    // Multi-Frame (First + Consecutive)
    // TODO: Implementierung für Senden von Multi-Frame
    // Aktuell nur Single-Frame unterstützt, da alle UDS-Requests ≤ 7 Bytes sind
    DEBUG_VERBOSE_PRINTLN("ERROR: Multi-Frame send not implemented");
    return false;
}

bool ISOTP::sendSingleFrame(uint32_t can_id, const uint8_t* data, size_t len) {
    if (len > ISOTP_SINGLE_FRAME_MAX_LEN) {
        return false;
    }

    twai_message_t frame;
    frame.identifier = can_id;
    frame.data_length_code = len + 1;  // PCI + Daten
    frame.extd = 0;  // Standard-ID
    frame.rtr = 0;
    frame.ss = 0;
    frame.self = 0;
    frame.dlc_non_comp = 0;

    // PCI: Single-Frame (0x0N, N = Länge)
    frame.data[0] = ISOTP_FRAME_TYPE_SINGLE | (len & 0x0F);

    // Nutzdaten kopieren
    for (size_t i = 0; i < len; i++) {
        frame.data[i + 1] = data[i];
    }

    // Restliche Bytes mit 0x00 füllen (optional)
    for (size_t i = len + 1; i < 8; i++) {
        frame.data[i] = 0x00;
    }

    return can_interface_->send(frame, 100);
}

bool ISOTP::receiveResponse(uint8_t* data, size_t& len, uint32_t timeout_ms) {
    if (!can_interface_) {
        return false;
    }

    uint32_t start_time = millis();

    // Ersten Frame empfangen
    twai_message_t frame;
    while (millis() - start_time < timeout_ms) {
        if (can_interface_->receive(frame, 10)) {
            // Nur Frames mit erwarteter Response-ID
            if (frame.identifier == expected_response_id_) {
                // PCI (Protocol Control Information) extrahieren
                uint8_t pci = frame.data[0];
                uint8_t frame_type = pci & 0xF0;

                if (frame_type == ISOTP_FRAME_TYPE_SINGLE) {
                    // Single-Frame: Länge aus PCI
                    uint8_t data_len = pci & 0x0F;
                    if (data_len > ISOTP_SINGLE_FRAME_MAX_LEN) {
                        DEBUG_VERBOSE_PRINTLN("ERROR: Invalid SF length");
                        return false;
                    }

                    // Nutzdaten kopieren
                    for (uint8_t i = 0; i < data_len; i++) {
                        data[i] = frame.data[i + 1];
                    }
                    len = data_len;
                    return true;

                } else if (frame_type == ISOTP_FRAME_TYPE_FIRST) {
                    // First-Frame: Multi-Frame-Empfang
                    uint16_t total_len = ((pci & 0x0F) << 8) | frame.data[1];
                    
                    if (total_len > ISOTP_MAX_DATA_LEN) {
                        DEBUG_VERBOSE_PRINTLN("ERROR: Multi-Frame too long");
                        return false;
                    }

                    // Erste 6 Bytes kopieren
                    for (uint8_t i = 0; i < ISOTP_FIRST_FRAME_MAX_LEN; i++) {
                        data[i] = frame.data[i + 2];
                    }

                    // Flow Control senden: ContinueToSend
                    if (!sendFlowControl(expected_response_id_ - 8, ISOTP_FC_STATUS_CTS, 0, 0)) {
                        DEBUG_VERBOSE_PRINTLN("ERROR: Flow Control send failed");
                        return false;
                    }

                    // Consecutive Frames empfangen
                    return receiveMultiFrame(data + ISOTP_FIRST_FRAME_MAX_LEN, len, 
                                            total_len - ISOTP_FIRST_FRAME_MAX_LEN, timeout_ms);
                }
            }
        }
    }

    // Timeout
    DEBUG_VERBOSE_PRINTLN("ERROR: ISO-TP receive timeout");
    return false;
}

bool ISOTP::receiveMultiFrame(uint8_t* data, size_t& len, uint16_t remaining_len, uint32_t timeout_ms) {
    uint8_t sequence_number = 1;  // Startet bei 1
    size_t received = 0;
    uint32_t start_time = millis();

    while (received < remaining_len && (millis() - start_time < timeout_ms)) {
        twai_message_t frame;
        if (can_interface_->receive(frame, 10)) {
            if (frame.identifier == expected_response_id_) {
                uint8_t pci = frame.data[0];
                uint8_t frame_type = pci & 0xF0;

                if (frame_type == ISOTP_FRAME_TYPE_CONSECUTIVE) {
                    uint8_t sn = pci & 0x0F;
                    
                    // Sequence Number prüfen
                    if (sn != sequence_number) {
                        DEBUG_VERBOSE_PRINTLN("ERROR: Sequence number mismatch");
                        return false;
                    }

                    // Nutzdaten kopieren (max. 7 Bytes)
                    size_t copy_len = min((size_t)ISOTP_CONSECUTIVE_FRAME_LEN, remaining_len - received);
                    for (size_t i = 0; i < copy_len; i++) {
                        data[received + i] = frame.data[i + 1];
                    }

                    received += copy_len;
                    sequence_number = (sequence_number + 1) & 0x0F;  // Wrap bei 15
                }
            }
        }
    }

    if (received < remaining_len) {
        DEBUG_VERBOSE_PRINTLN("ERROR: Multi-Frame incomplete");
        return false;
    }

    len = received + ISOTP_FIRST_FRAME_MAX_LEN;  // Gesamt = FF + CFs
    return true;
}

bool ISOTP::sendFlowControl(uint32_t can_id, uint8_t status, uint8_t block_size, uint8_t st_min) {
    twai_message_t frame;
    frame.identifier = can_id;
    frame.data_length_code = 3;
    frame.extd = 0;
    frame.rtr = 0;
    frame.ss = 0;
    frame.self = 0;
    frame.dlc_non_comp = 0;

    // PCI: Flow Control
    frame.data[0] = ISOTP_FRAME_TYPE_FLOW_CONTROL | (status & 0x0F);
    frame.data[1] = block_size;
    frame.data[2] = st_min;

    // Restliche Bytes mit 0x00
    for (int i = 3; i < 8; i++) {
        frame.data[i] = 0x00;
    }

    return can_interface_->send(frame, 50);
}

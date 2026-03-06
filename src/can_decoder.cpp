/**
 * @file can_decoder.cpp
 * @brief Implementierung der passiven Frame-Dekodierung
 */

#include "can_decoder.h"

CANDecoder::CANDecoder() : telemetry_(nullptr) {
}

void CANDecoder::begin(TelemetryManager& telemetry) {
    telemetry_ = &telemetry;
    DEBUG_PRINTLN("CANDecoder initialized.");
}

void CANDecoder::decodeFrame(const twai_message_t& frame) {
    if (!telemetry_) {
        return;
    }

    // Frame-ID prüfen und entsprechenden Decoder aufrufen
    switch (frame.identifier) {
        case FRAME_ID_AVAILABLE_ENERGY:
            decodeFrame_0x427(frame);
            break;
            
        case FRAME_ID_CELL_VOLTAGES:
            decodeFrame_0x5D7(frame);
            break;
            
        default:
            // Unbekannter Frame - ignorieren
            break;
    }
}

// =============================================================================
// Frame 0x427: Verfügbare Batterieenergie
// =============================================================================
void CANDecoder::decodeFrame_0x427(const twai_message_t& frame) {
    if (frame.data_length_code < 6) {
        DEBUG_VERBOSE_PRINTLN("WARNING: Frame 0x427 too short");
        return;
    }

    // Byte 4-5: Verfügbare Energie in 0.01 kWh
    // Format: Big-Endian, unsigned 16-bit
    uint16_t raw_energy = (frame.data[4] << 8) | frame.data[5];
    float available_energy_kwh = raw_energy * 0.01f;

    // An Telemetrie übergeben
    telemetry_->updateDataPoint(
        "available_energy_kwh",
        available_energy_kwh,
        "kWh",
        TELEMETRY_SOURCE_PASSIVE
    );

    DEBUG_VERBOSE_PRINT("Frame 0x427: Available Energy = ");
    DEBUG_VERBOSE_PRINT(available_energy_kwh);
    DEBUG_VERBOSE_PRINTLN(" kWh");
}

// =============================================================================
// Frame 0x5D7: Zellspannungen (multiplexed)
// =============================================================================
void CANDecoder::decodeFrame_0x5D7(const twai_message_t& frame) {
    if (frame.data_length_code < 8) {
        DEBUG_VERBOSE_PRINTLN("WARNING: Frame 0x5D7 too short");
        return;
    }

    // TODO: Vollständige Dekodierung des Multiplex-Protokolls
    // Frame 0x5D7 enthält Zellspannungen in einem multiplexten Format.
    // Jedes Frame enthält Daten für mehrere Zellen, abhängig vom Multiplex-Index.
    // 
    // Vorläufig: Frame empfangen, aber nicht dekodiert
    // Grund: Vollständiges Reverse-Engineering des Multiplex-Protokolls steht noch aus.
    //
    // Multiplex-Index ist vermutlich in Byte 0
    uint8_t multiplex_index = frame.data[0];
    
    DEBUG_VERBOSE_PRINT("Frame 0x5D7: Multiplex Index = ");
    DEBUG_VERBOSE_PRINTLN(multiplex_index);
    
    // Platzhalter für zukünftige Implementierung
    // TODO: Zellspannungen extrahieren und an Telemetrie übergeben
}

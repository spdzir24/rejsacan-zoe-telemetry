/**
 * @file uds_scanner.h
 * @brief UDS-Diagnose-Scanner (ISO 14229)
 */

#ifndef UDS_SCANNER_H
#define UDS_SCANNER_H

#include <Arduino.h>
#include "driver/twai.h"
#include "config.h"
#include "can_interface.h"
#include "telemetry.h"
#include "isotp.h"

// UDS-Job-Kategorien (Polling-Frequenz)
enum UDSPollClass {
    UDS_POLL_FAST,     // 1s - Dynamische Fahrdaten
    UDS_POLL_MEDIUM,   // 5s - Langsam ändernde Werte
    UDS_POLL_SLOW      // 30s - Statische Daten
};

// UDS-Datenpunkt-Descriptor
struct UDSDataItem {
    uint32_t request_id;        // CAN-ID für Request (z.B. 0x7E2)
    uint32_t response_id;       // CAN-ID für Response (z.B. 0x7EA)
    uint16_t did;               // Data Identifier (z.B. 0x20BE)
    const char* key;            // Eindeutiger Schlüssel (z.B. "real_soc_pct")
    const char* ecu_name;       // ECU-Name (z.B. "EVC")
    UDSPollClass poll_class;    // Polling-Frequenz
    void (*decoder)(const uint8_t* data, size_t len, TelemetryManager& telemetry);  // Decoder-Funktion
};

class UDSScanner {
public:
    UDSScanner();
    
    /**
     * @brief Initialisiert den UDS-Scanner
     * @param can_interface Referenz zum CAN-Interface
     * @param telemetry Referenz zum Telemetrie-Manager
     */
    void begin(CANInterface& can_interface, TelemetryManager& telemetry);
    
    /**
     * @brief Fragt ein spezifisches DID ab
     * @param item UDS-Datenpunkt-Descriptor
     * @return true bei Erfolg, false bei Fehler
     */
    bool requestDID(const UDSDataItem& item);
    
    /**
     * @brief Gibt alle UDS-Datenpunkte zurück
     * @param count Output: Anzahl der Datenpunkte
     * @return Zeiger auf Array von UDSDataItem
     */
    static const UDSDataItem* getDataItems(size_t& count);

private:
    CANInterface* can_interface_;
    TelemetryManager* telemetry_;
    ISOTP isotp_;
    
    uint32_t request_count_;
    uint32_t error_count_;
};

#endif // UDS_SCANNER_H

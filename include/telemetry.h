/**
 * @file telemetry.h
 * @brief Zentrales Telemetrie-Datenmodell
 */

#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <Arduino.h>
#include "config.h"

// Telemetrie-Datenquellen
enum TelemetrySource {
    TELEMETRY_SOURCE_PASSIVE,    // Passive CAN-Frames
    TELEMETRY_SOURCE_UDS_FAST,   // UDS Fast (1s)
    TELEMETRY_SOURCE_UDS_MEDIUM, // UDS Medium (5s)
    TELEMETRY_SOURCE_UDS_SLOW    // UDS Slow (30s)
};

// Telemetrie-Datenpunkt
struct TelemetryDataPoint {
    const char* key;         // Eindeutiger Schlüssel
    float value;             // Numerischer Wert
    const char* unit;        // Einheit (optional)
    uint32_t timestamp;      // Millisekunden seit Boot
    TelemetrySource source;  // Datenquelle
    bool valid;              // Gültigkeit
    bool updated;            // Flag: Wert wurde aktualisiert (für UART-Output)
};

class TelemetryManager {
public:
    TelemetryManager();
    
    /**
     * @brief Initialisiert den Telemetrie-Manager
     */
    void begin();
    
    /**
     * @brief Aktualisiert einen Datenpunkt
     * @param key Eindeutiger Schlüssel
     * @param value Neuer Wert
     * @param unit Einheit (optional)
     * @param source Datenquelle
     */
    void updateDataPoint(const char* key, float value, const char* unit, TelemetrySource source);
    
    /**
     * @brief Gibt alle Datenpunkte zurück
     * @param count Output: Anzahl der Datenpunkte
     * @return Zeiger auf Array von TelemetryDataPoint
     */
    const TelemetryDataPoint* getDataPoints(size_t& count) const;
    
    /**
     * @brief Setzt alle "updated"-Flags zurück
     */
    void clearUpdatedFlags();

private:
    static const size_t MAX_DATA_POINTS = 50;
    TelemetryDataPoint data_points_[MAX_DATA_POINTS];
    size_t data_point_count_;
    
    // Findet Index eines Datenpunkts oder legt neuen an
    int findOrCreateDataPoint(const char* key);
};

#endif // TELEMETRY_H

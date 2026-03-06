/**
 * @file uart_output.cpp
 * @brief Implementierung der UART-Telemetrie-Ausgabe
 */

#include "uart_output.h"

UARTOutput::UARTOutput() : telemetry_(nullptr), last_output_time_(0) {
}

void UARTOutput::begin(TelemetryManager& telemetry) {
    telemetry_ = &telemetry;
}

void UARTOutput::update() {
    if (!telemetry_) {
        return;
    }

    // Alle Datenpunkte abrufen
    size_t count = 0;
    const TelemetryDataPoint* data_points = telemetry_->getDataPoints(count);

    // Nur aktualisierte Datenpunkte ausgeben
    for (size_t i = 0; i < count; i++) {
        const TelemetryDataPoint& dp = data_points[i];
        
        if (dp.updated && dp.valid) {
#if TELEMETRY_FORMAT_JSON
            // JSON Lines Format
            JsonDocument doc;
            doc["type"] = "telemetry";
            doc["ts"] = dp.timestamp;
            doc["source"] = sourceToString(dp.source);
            doc["key"] = dp.key;
            doc["value"] = dp.value;
            if (strlen(dp.unit) > 0) {
                doc["unit"] = dp.unit;
            }
            
            serializeJson(doc, Serial);
            Serial.println();  // Zeilenumbruch für JSON Lines
#else
            // Human-readable Format (Fallback)
            Serial.print("[TELEMETRY] ");
            Serial.print(dp.key);
            Serial.print(" = ");
            Serial.print(dp.value);
            if (strlen(dp.unit) > 0) {
                Serial.print(" ");
                Serial.print(dp.unit);
            }
            Serial.print(" (source: ");
            Serial.print(sourceToString(dp.source));
            Serial.println(")");
#endif
        }
    }

    // Updated-Flags zurücksetzen
    telemetry_->clearUpdatedFlags();
}

void UARTOutput::sendHeartbeat() {
#if TELEMETRY_FORMAT_JSON
    JsonDocument doc;
    doc["type"] = "heartbeat";
    doc["ts"] = millis();
    doc["uptime"] = millis();
    
    serializeJson(doc, Serial);
    Serial.println();
#else
    Serial.print("[HEARTBEAT] Uptime: ");
    Serial.print(millis() / 1000);
    Serial.println(" s");
#endif
}

void UARTOutput::sendStartup() {
#if TELEMETRY_FORMAT_JSON
    JsonDocument doc;
    doc["type"] = "startup";
    doc["ts"] = millis();
    doc["system"] = SYSTEM_NAME;
    doc["version"] = SYSTEM_VERSION;
    doc["build"] = SYSTEM_BUILD_DATE;
    doc["can_bitrate"] = CAN_BITRATE;
    doc["passive_enabled"] = ENABLE_PASSIVE_SNIFFING;
    doc["uds_enabled"] = ENABLE_UDS_DIAGNOSTICS;
    
    serializeJson(doc, Serial);
    Serial.println();
#else
    Serial.println("[STARTUP] System started.");
#endif
}

void UARTOutput::sendError(const char* module, const char* message) {
#if TELEMETRY_FORMAT_JSON
    JsonDocument doc;
    doc["type"] = "error";
    doc["ts"] = millis();
    doc["module"] = module;
    doc["message"] = message;
    
    serializeJson(doc, Serial);
    Serial.println();
#else
    Serial.print("[ERROR] ");
    Serial.print(module);
    Serial.print(": ");
    Serial.println(message);
#endif
}

const char* UARTOutput::sourceToString(TelemetrySource source) {
    switch (source) {
        case TELEMETRY_SOURCE_PASSIVE:
            return "passive";
        case TELEMETRY_SOURCE_UDS_FAST:
            return "uds_fast";
        case TELEMETRY_SOURCE_UDS_MEDIUM:
            return "uds_medium";
        case TELEMETRY_SOURCE_UDS_SLOW:
            return "uds_slow";
        default:
            return "unknown";
    }
}

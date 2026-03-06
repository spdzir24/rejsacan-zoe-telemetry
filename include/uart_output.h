/**
 * @file uart_output.h
 * @brief UART-Telemetrie-Ausgabe (JSON Lines)
 */

#ifndef UART_OUTPUT_H
#define UART_OUTPUT_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"
#include "telemetry.h"

class UARTOutput {
public:
    UARTOutput();
    
    /**
     * @brief Initialisiert UART-Output
     * @param telemetry Referenz zum Telemetrie-Manager
     */
    void begin(TelemetryManager& telemetry);
    
    /**
     * @brief Aktualisiert UART-Output (ruft zyklisch in loop() auf)
     */
    void update();
    
    /**
     * @brief Sendet Heartbeat-Nachricht
     */
    void sendHeartbeat();
    
    /**
     * @brief Sendet Startup-Nachricht
     */
    void sendStartup();
    
    /**
     * @brief Sendet Fehlermeldung
     * @param module Modul-Name
     * @param message Fehlermeldung
     */
    void sendError(const char* module, const char* message);

private:
    TelemetryManager* telemetry_;
    uint32_t last_output_time_;
    
    // Konvertiert TelemetrySource zu String
    const char* sourceToString(TelemetrySource source);
};

#endif // UART_OUTPUT_H

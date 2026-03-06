/**
 * @file main.cpp
 * @brief Hauptprogramm für RejsaCAN Zoe Telemetry System
 * 
 * ESP32-C6 CAN-Telemetriesystem für Renault Zoe ZE50 Phase 2
 * - Passive CAN-Frame-Erfassung
 * - Optionale UDS-Diagnose (konfigurierbar)
 * - UART-Datenausgabe (JSON Lines)
 * - Dreistufiger Scheduler (Fast/Medium/Slow)
 */

#include <Arduino.h>
#include "config.h"
#include "can_interface.h"
#include "can_decoder.h"
#include "uds_scanner.h"
#include "telemetry.h"
#include "uart_output.h"
#include "scheduler.h"

// =============================================================================
// Globale Objekte
// =============================================================================

CANInterface can_interface;
CANDecoder can_decoder;
TelemetryManager telemetry;
UARTOutput uart_output;
Scheduler scheduler;

#if ENABLE_UDS_DIAGNOSTICS
UDSScanner uds_scanner;
#endif

// =============================================================================
// Setup
// =============================================================================

void setup() {
    // UART initialisieren (für Telemetrie und Debug)
    Serial.begin(UART_BAUDRATE);
    delay(1000);  // Kurze Verzögerung für Serial-Stabilisierung

    // Startup-Meldung
    DEBUG_PRINTLN("\n\n=====================================");
    DEBUG_PRINTLN(SYSTEM_NAME);
    DEBUG_PRINT("Version: ");
    DEBUG_PRINTLN(SYSTEM_VERSION);
    DEBUG_PRINT("Build: ");
    DEBUG_PRINTLN(SYSTEM_BUILD_DATE);
    DEBUG_PRINTLN("=====================================");

    // Konfiguration ausgeben
    DEBUG_PRINT("CAN Bitrate: ");
    DEBUG_PRINT(CAN_BITRATE / 1000);
    DEBUG_PRINTLN(" kbit/s");
    
    DEBUG_PRINT("Passive Sniffing: ");
    DEBUG_PRINTLN(ENABLE_PASSIVE_SNIFFING ? "ENABLED" : "DISABLED");
    
    DEBUG_PRINT("UDS Diagnostics: ");
    DEBUG_PRINTLN(ENABLE_UDS_DIAGNOSTICS ? "ENABLED" : "DISABLED");
    
    DEBUG_PRINT("UART Baudrate: ");
    DEBUG_PRINTLN(UART_BAUDRATE);
    
    DEBUG_PRINTLN("=====================================");

    // CAN-Interface initialisieren
    DEBUG_PRINTLN("Initializing TWAI/CAN interface...");
    if (!can_interface.begin()) {
        DEBUG_PRINTLN("ERROR: CAN initialization failed!");
        uart_output.sendError("can_init", "TWAI initialization failed");
        // System hält an - ohne CAN kein Betrieb möglich
        while (true) {
            delay(1000);
        }
    }
    DEBUG_PRINTLN("CAN interface ready.");

    // Telemetrie-Manager initialisieren
    telemetry.begin();
    DEBUG_PRINTLN("Telemetry manager ready.");

    // UART-Output initialisieren
    uart_output.begin(telemetry);
    DEBUG_PRINTLN("UART output ready.");

    // CAN-Decoder initialisieren
    can_decoder.begin(telemetry);
    DEBUG_PRINTLN("CAN decoder ready.");

#if ENABLE_UDS_DIAGNOSTICS
    // UDS-Scanner initialisieren (nur wenn aktiviert)
    uds_scanner.begin(can_interface, telemetry);
    DEBUG_PRINTLN("UDS scanner ready.");
    
    // UDS-Jobs zum Scheduler hinzufügen
    scheduler.begin();
    DEBUG_PRINTLN("Scheduler ready.");
#endif

    DEBUG_PRINTLN("=====================================");
    DEBUG_PRINTLN("System startup complete.");
    DEBUG_PRINTLN("Waiting for CAN data...");
    DEBUG_PRINTLN("=====================================");

    // Startup-Meldung über UART (JSON)
    uart_output.sendStartup();
}

// =============================================================================
// Hauptschleife
// =============================================================================

void loop() {
    static uint32_t last_heartbeat = 0;
    uint32_t now = millis();

    // ---------------------------------------------------------
    // 1. Passive CAN-Frames empfangen und dekodieren
    // ---------------------------------------------------------
#if ENABLE_PASSIVE_SNIFFING
    twai_message_t rx_frame;
    if (can_interface.receive(rx_frame, 0)) {  // Non-blocking
        // Frame dekodieren (falls bekannt)
        can_decoder.decodeFrame(rx_frame);
        
        // Optional: Raw-Frame ausgeben (nur im Debug-Modus)
#if ENABLE_DEBUG_OUTPUT >= 2
        DEBUG_VERBOSE_PRINT("RX: ID=0x");
        DEBUG_VERBOSE_PRINT(rx_frame.identifier, HEX);
        DEBUG_VERBOSE_PRINT(" DLC=");
        DEBUG_VERBOSE_PRINT(rx_frame.data_length_code);
        DEBUG_VERBOSE_PRINT(" Data=");
        for (int i = 0; i < rx_frame.data_length_code; i++) {
            DEBUG_VERBOSE_PRINT(" ");
            DEBUG_VERBOSE_PRINT(rx_frame.data[i], HEX);
        }
        DEBUG_VERBOSE_PRINTLN();
#endif
    }
#endif

    // ---------------------------------------------------------
    // 2. UDS-Scheduler aktualisieren (falls aktiviert)
    // ---------------------------------------------------------
#if ENABLE_UDS_DIAGNOSTICS
    scheduler.update(uds_scanner);
#endif

    // ---------------------------------------------------------
    // 3. Telemetrie über UART ausgeben
    // ---------------------------------------------------------
    uart_output.update();

    // ---------------------------------------------------------
    // 4. Heartbeat senden (periodisch)
    // ---------------------------------------------------------
#if ENABLE_HEARTBEAT
    if (now - last_heartbeat >= HEARTBEAT_INTERVAL_MS) {
        uart_output.sendHeartbeat();
        last_heartbeat = now;
    }
#endif

    // Kurze Verzögerung, um Watchdog zu füttern
    delay(1);
}

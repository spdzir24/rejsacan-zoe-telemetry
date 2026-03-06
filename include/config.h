/**
 * @file config.h
 * @brief Zentrale Konfiguration für RejsaCAN Zoe Telemetry System
 * 
 * Alle Compile-Time-Konfigurationen werden über platformio.ini als Defines gesetzt.
 * Diese Datei enthält nur Fallback-Werte und Konstanten.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include "driver/twai.h"

// =============================================================================
// CAN/TWAI-Konfiguration
// =============================================================================

#ifndef CAN_BITRATE
#define CAN_BITRATE 500000  // 500 kbit/s (Standard für Renault Zoe)
#endif

#ifndef TWAI_TX_GPIO
#define TWAI_TX_GPIO GPIO_NUM_5  // RejsaCAN v6.x Standard
#endif

#ifndef TWAI_RX_GPIO
#define TWAI_RX_GPIO GPIO_NUM_4  // RejsaCAN v6.x Standard
#endif

#ifndef CAN_RX_QUEUE_SIZE
#define CAN_RX_QUEUE_SIZE 50  // Queue-Größe für empfangene Frames
#endif

// =============================================================================
// Feature-Flags
// =============================================================================

#ifndef ENABLE_PASSIVE_SNIFFING
#define ENABLE_PASSIVE_SNIFFING 1  // Passive CAN-Frames lesen
#endif

#ifndef ENABLE_UDS_DIAGNOSTICS
#define ENABLE_UDS_DIAGNOSTICS 0  // UDS-Anfragen (Standard: AUS!)
#endif

#ifndef ENABLE_DEBUG_OUTPUT
#define ENABLE_DEBUG_OUTPUT 1  // Debug-Meldungen über UART
#endif

#ifndef ENABLE_HEARTBEAT
#define ENABLE_HEARTBEAT 1  // Periodischer Heartbeat über UART
#endif

#ifndef TELEMETRY_FORMAT_JSON
#define TELEMETRY_FORMAT_JSON 1  // JSON Lines Format (empfohlen)
#endif

// =============================================================================
// UART-Konfiguration
// =============================================================================

#ifndef UART_BAUDRATE
#define UART_BAUDRATE 115200  // Standard-Baudrate
#endif

#ifndef TELEMETRY_BUFFER_SIZE
#define TELEMETRY_BUFFER_SIZE 512  // Puffer für JSON-Serialisierung
#endif

#ifndef HEARTBEAT_INTERVAL_MS
#define HEARTBEAT_INTERVAL_MS 5000  // Heartbeat alle 5 Sekunden
#endif

// =============================================================================
// UDS-Timing-Konfiguration
// =============================================================================

#ifndef UDS_POLL_INTERVAL_FAST_MS
#define UDS_POLL_INTERVAL_FAST_MS 1000  // Schnelle Datenpunkte: 1s
#endif

#ifndef UDS_POLL_INTERVAL_MEDIUM_MS
#define UDS_POLL_INTERVAL_MEDIUM_MS 5000  // Mittlere Datenpunkte: 5s
#endif

#ifndef UDS_POLL_INTERVAL_SLOW_MS
#define UDS_POLL_INTERVAL_SLOW_MS 30000  // Langsame Datenpunkte: 30s
#endif

#ifndef UDS_TIMEOUT_MS
#define UDS_TIMEOUT_MS 200  // Timeout für UDS-Antwort
#endif

#ifndef UDS_MAX_RETRIES
#define UDS_MAX_RETRIES 2  // Maximale Wiederholungen bei Timeout
#endif

// =============================================================================
// ISO-TP-Konfiguration
// =============================================================================

#define ISOTP_SINGLE_FRAME_MAX_LEN 7  // Max. Nutzdaten in Single-Frame
#define ISOTP_FIRST_FRAME_MAX_LEN 6   // Max. Nutzdaten in First-Frame
#define ISOTP_CONSECUTIVE_FRAME_LEN 7 // Nutzdaten pro Consecutive-Frame
#define ISOTP_MAX_DATA_LEN 4095       // Max. ISO-TP-Nachrichtenlänge

#define ISOTP_FLOW_CONTROL_TIMEOUT_MS 100  // Timeout für Flow Control
#define ISOTP_CONSECUTIVE_TIMEOUT_MS 100   // Timeout zwischen Consecutive Frames

// =============================================================================
// Renault Zoe ZE50 Phase 2 - CAN-IDs und DIDs
// =============================================================================

// Passive CAN-Frame-IDs
#define FRAME_ID_AVAILABLE_ENERGY 0x427  // Verfügbare Batterieenergie
#define FRAME_ID_CELL_VOLTAGES    0x5D7  // Zellspannungen (multiplexed)

// UDS-Request/Response-Paare
#define UDS_REQ_EVC 0x7E2  // EVC/VCM Request
#define UDS_RES_EVC 0x7EA  // EVC/VCM Response

#define UDS_REQ_LBC 0x7E4  // LBC/BMS Request
#define UDS_RES_LBC 0x7EC  // LBC/BMS Response

#define UDS_REQ_PEC 0x7E3  // PEC/Inverter Request
#define UDS_RES_PEC 0x7EB  // PEC/Inverter Response

#define UDS_REQ_BCM 0x771  // BCM Request
#define UDS_RES_BCM 0x779  // BCM Response

#define UDS_REQ_MULTIMEDIA 0x7B5  // Multimedia Request
#define UDS_RES_MULTIMEDIA 0x7BD  // Multimedia Response

// UDS-Service-Codes
#define UDS_SERVICE_READ_DATA_BY_ID 0x22        // ReadDataByIdentifier
#define UDS_SERVICE_POSITIVE_RESPONSE_OFFSET 0x40  // Positive Response: Service + 0x40
#define UDS_SERVICE_NEGATIVE_RESPONSE 0x7F      // Negative Response

// UDS-DIDs (Data Identifiers)
// EVC (0x7E2)
#define DID_REAL_SOC              0x20BE  // Real State of Charge [%]
#define DID_ENERGY_PER_SOC        0x303E  // Energie pro SOC-Prozent [Wh/%]
#define DID_HV_BATTERY_VOLTAGE    0x20FE  // HV-Batteriespannung [V]
#define DID_HV_BATTERY_CURRENT    0x21CC  // HV-Batteriestrom [A]
#define DID_MOTOR_RPM             0x3064  // Motordrehzahl [rpm]

// LBC (0x7E4)
#define DID_LBC_BATTERY_VOLTAGE   0x3203  // Batteriespannung intern [V]
#define DID_LBC_BATTERY_CURRENT   0x3204  // Batteriestrom intern [A]
#define DID_AVAILABLE_DISCHARGE   0x502C  // Verfügbare Entladeenergie [Wh]
#define DID_STATE_OF_HEALTH       0x0101  // State of Health [%]
#define DID_MAX_CHARGE_POWER      0x3206  // Maximale Ladeleistung [W]
#define DID_MAX_CELL_TEMP         0x320B  // Maximale Zelltemperatur [°C]

// BCM (0x771)
#define DID_12V_BATTERY_VOLTAGE   0x2002  // 12V-Batteriespannung [V]

// Multimedia (0x7B5)
#define DID_VIN                   0xF190  // Fahrzeug-Identifikationsnummer

// =============================================================================
// Debug-Makros
// =============================================================================

#if ENABLE_DEBUG_OUTPUT >= 1
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif

#if ENABLE_DEBUG_OUTPUT >= 2
#define DEBUG_VERBOSE_PRINT(x) Serial.print(x)
#define DEBUG_VERBOSE_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_VERBOSE_PRINT(x)
#define DEBUG_VERBOSE_PRINTLN(x)
#endif

// =============================================================================
// System-Konstanten
// =============================================================================

#define SYSTEM_NAME "RejsaCAN Zoe Telemetry"
#define SYSTEM_VERSION "1.0.0"
#define SYSTEM_BUILD_DATE __DATE__ " " __TIME__

#endif // CONFIG_H

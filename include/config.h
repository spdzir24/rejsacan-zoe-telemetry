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
#define CAN_BITRATE 500000
#endif

#ifndef TWAI_TX_GPIO
#define TWAI_TX_GPIO GPIO_NUM_5
#endif

#ifndef TWAI_RX_GPIO
#define TWAI_RX_GPIO GPIO_NUM_4
#endif

#ifndef CAN_RX_QUEUE_SIZE
#define CAN_RX_QUEUE_SIZE 50
#endif

// =============================================================================
// Feature-Flags
// =============================================================================

#ifndef ENABLE_PASSIVE_SNIFFING
#define ENABLE_PASSIVE_SNIFFING 1
#endif

#ifndef ENABLE_UDS_DIAGNOSTICS
#define ENABLE_UDS_DIAGNOSTICS 0
#endif

#ifndef ENABLE_DEBUG_OUTPUT
#define ENABLE_DEBUG_OUTPUT 1
#endif

#ifndef ENABLE_HEARTBEAT
#define ENABLE_HEARTBEAT 1
#endif

#ifndef TELEMETRY_FORMAT_JSON
#define TELEMETRY_FORMAT_JSON 1
#endif

// =============================================================================
// UART-Konfiguration
// =============================================================================

#ifndef UART_BAUDRATE
#define UART_BAUDRATE 115200
#endif

#ifndef TELEMETRY_BUFFER_SIZE
#define TELEMETRY_BUFFER_SIZE 512
#endif

#ifndef HEARTBEAT_INTERVAL_MS
#define HEARTBEAT_INTERVAL_MS 5000
#endif

// =============================================================================
// UDS-Timing-Konfiguration
// =============================================================================

#ifndef UDS_POLL_INTERVAL_FAST_MS
#define UDS_POLL_INTERVAL_FAST_MS 1000
#endif

#ifndef UDS_POLL_INTERVAL_MEDIUM_MS
#define UDS_POLL_INTERVAL_MEDIUM_MS 5000
#endif

#ifndef UDS_POLL_INTERVAL_SLOW_MS
#define UDS_POLL_INTERVAL_SLOW_MS 30000
#endif

#ifndef UDS_TIMEOUT_MS
#define UDS_TIMEOUT_MS 200
#endif

#ifndef UDS_MAX_RETRIES
#define UDS_MAX_RETRIES 2
#endif

// =============================================================================
// ISO-TP-Konfiguration
// =============================================================================

#define ISOTP_SINGLE_FRAME_MAX_LEN 7
#define ISOTP_FIRST_FRAME_MAX_LEN 6
#define ISOTP_CONSECUTIVE_FRAME_LEN 7
#define ISOTP_MAX_DATA_LEN 4095

#define ISOTP_FLOW_CONTROL_TIMEOUT_MS 100
#define ISOTP_CONSECUTIVE_TIMEOUT_MS 100

// =============================================================================
// Renault Zoe ZE50 Phase 2 - CAN-IDs und DIDs
// =============================================================================

#define FRAME_ID_AVAILABLE_ENERGY 0x427
#define FRAME_ID_CELL_VOLTAGES    0x5D7

#define UDS_REQ_EVC 0x7E2
#define UDS_RES_EVC 0x7EA

#define UDS_REQ_LBC 0x7E4
#define UDS_RES_LBC 0x7EC

#define UDS_REQ_PEC 0x7E3
#define UDS_RES_PEC 0x7EB

#define UDS_REQ_BCM 0x771
#define UDS_RES_BCM 0x779

#define UDS_REQ_MULTIMEDIA 0x7B5
#define UDS_RES_MULTIMEDIA 0x7BD

#define UDS_SERVICE_READ_DATA_BY_ID 0x22
#define UDS_SERVICE_POSITIVE_RESPONSE_OFFSET 0x40
#define UDS_SERVICE_NEGATIVE_RESPONSE 0x7F

#define DID_REAL_SOC              0x20BE
#define DID_ENERGY_PER_SOC        0x303E
#define DID_HV_BATTERY_VOLTAGE    0x20FE
#define DID_HV_BATTERY_CURRENT    0x21CC
#define DID_MOTOR_RPM             0x3064

#define DID_LBC_BATTERY_VOLTAGE   0x3203
#define DID_LBC_BATTERY_CURRENT   0x3204
#define DID_AVAILABLE_DISCHARGE   0x502C
#define DID_STATE_OF_HEALTH       0x0101
#define DID_MAX_CHARGE_POWER      0x3206
#define DID_MAX_CELL_TEMP         0x320B

#define DID_12V_BATTERY_VOLTAGE   0x2002
#define DID_VIN                   0xF190

// =============================================================================
// Debug-Makros
// =============================================================================

#if ENABLE_DEBUG_OUTPUT >= 1
#define DEBUG_PRINT(...) do { Serial.print(__VA_ARGS__); } while (0)
#define DEBUG_PRINTLN(...) do { Serial.println(__VA_ARGS__); } while (0)
#else
#define DEBUG_PRINT(...) do {} while (0)
#define DEBUG_PRINTLN(...) do {} while (0)
#endif

#if ENABLE_DEBUG_OUTPUT >= 2
#define DEBUG_VERBOSE_PRINT(...) do { Serial.print(__VA_ARGS__); } while (0)
#define DEBUG_VERBOSE_PRINTLN(...) do { Serial.println(__VA_ARGS__); } while (0)
#else
#define DEBUG_VERBOSE_PRINT(...) do {} while (0)
#define DEBUG_VERBOSE_PRINTLN(...) do {} while (0)
#endif

// =============================================================================
// System-Konstanten
// =============================================================================

#define SYSTEM_NAME "RejsaCAN Zoe Telemetry"
#define SYSTEM_VERSION "1.0.1"
#define SYSTEM_BUILD_DATE __DATE__ " " __TIME__

#endif

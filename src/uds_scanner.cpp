/**
 * @file uds_scanner.cpp
 * @brief Implementierung des UDS-Scanners
 */

#include "uds_scanner.h"

// =============================================================================
// Decoder-Funktionen für UDS-DIDs
// =============================================================================

// EVC: Real SOC (0x20BE)
void decodeRealSOC(const uint8_t* data, size_t len, TelemetryManager& telemetry) {
    if (len < 2) return;
    uint16_t raw = (data[0] << 8) | data[1];
    float soc = raw * 0.01f;  // 0.01% Auflösung
    telemetry.updateDataPoint("real_soc_pct", soc, "%", TELEMETRY_SOURCE_UDS_FAST);
}

// EVC: Energie pro SOC% (0x303E)
void decodeEnergyPerSOC(const uint8_t* data, size_t len, TelemetryManager& telemetry) {
    if (len < 2) return;
    uint16_t raw = (data[0] << 8) | data[1];
    float energy_per_soc = raw * 0.1f;  // 0.1 Wh/% Auflösung
    telemetry.updateDataPoint("energy_per_soc_wh", energy_per_soc, "Wh/%", TELEMETRY_SOURCE_UDS_SLOW);
}

// EVC: HV-Batteriespannung (0x20FE)
void decodeHVVoltage(const uint8_t* data, size_t len, TelemetryManager& telemetry) {
    if (len < 2) return;
    uint16_t raw = (data[0] << 8) | data[1];
    float voltage = raw * 0.1f;  // 0.1V Auflösung
    telemetry.updateDataPoint("hv_battery_voltage_v", voltage, "V", TELEMETRY_SOURCE_UDS_FAST);
}

// EVC: HV-Batteriestrom (0x21CC)
void decodeHVCurrent(const uint8_t* data, size_t len, TelemetryManager& telemetry) {
    if (len < 2) return;
    int16_t raw = (data[0] << 8) | data[1];  // Signed!
    float current = raw * 0.1f;  // 0.1A Auflösung
    telemetry.updateDataPoint("hv_battery_current_a", current, "A", TELEMETRY_SOURCE_UDS_FAST);
}

// EVC: Motordrehzahl (0x3064)
void decodeMotorRPM(const uint8_t* data, size_t len, TelemetryManager& telemetry) {
    if (len < 2) return;
    uint16_t raw = (data[0] << 8) | data[1];
    float rpm = raw * 1.0f;  // 1 rpm Auflösung
    telemetry.updateDataPoint("motor_rpm", rpm, "rpm", TELEMETRY_SOURCE_UDS_FAST);
}

// LBC: Batteriespannung (0x3203)
void decodeLBCVoltage(const uint8_t* data, size_t len, TelemetryManager& telemetry) {
    if (len < 2) return;
    uint16_t raw = (data[0] << 8) | data[1];
    float voltage = raw * 0.01f;  // 0.01V Auflösung
    telemetry.updateDataPoint("lbc_battery_voltage_v", voltage, "V", TELEMETRY_SOURCE_UDS_MEDIUM);
}

// LBC: Batteriestrom (0x3204)
void decodeLBCCurrent(const uint8_t* data, size_t len, TelemetryManager& telemetry) {
    if (len < 2) return;
    int16_t raw = (data[0] << 8) | data[1];  // Signed!
    float current = raw * 0.1f;  // 0.1A Auflösung
    telemetry.updateDataPoint("lbc_battery_current_a", current, "A", TELEMETRY_SOURCE_UDS_MEDIUM);
}

// LBC: Verfügbare Entladeenergie (0x502C)
void decodeAvailableDischarge(const uint8_t* data, size_t len, TelemetryManager& telemetry) {
    if (len < 2) return;
    uint16_t raw = (data[0] << 8) | data[1];
    float energy = raw * 0.1f;  // 0.1 Wh Auflösung
    telemetry.updateDataPoint("available_discharge_wh", energy, "Wh", TELEMETRY_SOURCE_UDS_MEDIUM);
}

// LBC: State of Health (0x0101)
void decodeSOH(const uint8_t* data, size_t len, TelemetryManager& telemetry) {
    if (len < 2) return;
    uint16_t raw = (data[0] << 8) | data[1];
    float soh = raw * 0.01f;  // 0.01% Auflösung
    telemetry.updateDataPoint("state_of_health_pct", soh, "%", TELEMETRY_SOURCE_UDS_SLOW);
}

// LBC: Max. Ladeleistung (0x3206)
void decodeMaxChargePower(const uint8_t* data, size_t len, TelemetryManager& telemetry) {
    if (len < 2) return;
    uint16_t raw = (data[0] << 8) | data[1];
    float power = raw * 10.0f;  // 10W Auflösung
    telemetry.updateDataPoint("max_charge_power_w", power, "W", TELEMETRY_SOURCE_UDS_MEDIUM);
}

// LBC: Max. Zelltemperatur (0x320B)
void decodeMaxCellTemp(const uint8_t* data, size_t len, TelemetryManager& telemetry) {
    if (len < 1) return;
    int8_t raw = data[0];  // Signed!
    float temp = raw * 1.0f;  // 1°C Auflösung
    telemetry.updateDataPoint("max_cell_temp_c", temp, "°C", TELEMETRY_SOURCE_UDS_MEDIUM);
}

// BCM: 12V-Batteriespannung (0x2002)
void decode12VVoltage(const uint8_t* data, size_t len, TelemetryManager& telemetry) {
    if (len < 2) return;
    uint16_t raw = (data[0] << 8) | data[1];
    float voltage = raw * 0.01f;  // 0.01V Auflösung
    telemetry.updateDataPoint("12v_battery_voltage_v", voltage, "V", TELEMETRY_SOURCE_UDS_MEDIUM);
}

// Multimedia: VIN (0xF190)
void decodeVIN(const uint8_t* data, size_t len, TelemetryManager& telemetry) {
    if (len < 17) return;  // VIN ist 17 Zeichen
    // VIN als String speichern (TODO: String-Unterstützung in Telemetrie)
    DEBUG_PRINT("VIN: ");
    for (size_t i = 0; i < 17; i++) {
        DEBUG_PRINT((char)data[i]);
    }
    DEBUG_PRINTLN();
    // Vorerst nicht in Telemetrie speichern (nur Debug)
}

// =============================================================================
// UDS-Datenpunkt-Tabelle
// =============================================================================

static const UDSDataItem uds_data_items[] = {
    // EVC (0x7E2 -> 0x7EA)
    {UDS_REQ_EVC, UDS_RES_EVC, DID_REAL_SOC, "real_soc_pct", "EVC", UDS_POLL_FAST, decodeRealSOC},
    {UDS_REQ_EVC, UDS_RES_EVC, DID_HV_BATTERY_VOLTAGE, "hv_battery_voltage_v", "EVC", UDS_POLL_FAST, decodeHVVoltage},
    {UDS_REQ_EVC, UDS_RES_EVC, DID_HV_BATTERY_CURRENT, "hv_battery_current_a", "EVC", UDS_POLL_FAST, decodeHVCurrent},
    {UDS_REQ_EVC, UDS_RES_EVC, DID_MOTOR_RPM, "motor_rpm", "EVC", UDS_POLL_FAST, decodeMotorRPM},
    {UDS_REQ_EVC, UDS_RES_EVC, DID_ENERGY_PER_SOC, "energy_per_soc_wh", "EVC", UDS_POLL_SLOW, decodeEnergyPerSOC},
    
    // LBC (0x7E4 -> 0x7EC)
    {UDS_REQ_LBC, UDS_RES_LBC, DID_LBC_BATTERY_VOLTAGE, "lbc_battery_voltage_v", "LBC", UDS_POLL_MEDIUM, decodeLBCVoltage},
    {UDS_REQ_LBC, UDS_RES_LBC, DID_LBC_BATTERY_CURRENT, "lbc_battery_current_a", "LBC", UDS_POLL_MEDIUM, decodeLBCCurrent},
    {UDS_REQ_LBC, UDS_RES_LBC, DID_AVAILABLE_DISCHARGE, "available_discharge_wh", "LBC", UDS_POLL_MEDIUM, decodeAvailableDischarge},
    {UDS_REQ_LBC, UDS_RES_LBC, DID_MAX_CHARGE_POWER, "max_charge_power_w", "LBC", UDS_POLL_MEDIUM, decodeMaxChargePower},
    {UDS_REQ_LBC, UDS_RES_LBC, DID_MAX_CELL_TEMP, "max_cell_temp_c", "LBC", UDS_POLL_MEDIUM, decodeMaxCellTemp},
    {UDS_REQ_LBC, UDS_RES_LBC, DID_STATE_OF_HEALTH, "state_of_health_pct", "LBC", UDS_POLL_SLOW, decodeSOH},
    
    // BCM (0x771 -> 0x779)
    {UDS_REQ_BCM, UDS_RES_BCM, DID_12V_BATTERY_VOLTAGE, "12v_battery_voltage_v", "BCM", UDS_POLL_MEDIUM, decode12VVoltage},
    
    // Multimedia (0x7B5 -> 0x7BD)
    {UDS_REQ_MULTIMEDIA, UDS_RES_MULTIMEDIA, DID_VIN, "vin", "Multimedia", UDS_POLL_SLOW, decodeVIN},
};

static const size_t uds_data_items_count = sizeof(uds_data_items) / sizeof(uds_data_items[0]);

// =============================================================================
// UDSScanner-Implementierung
// =============================================================================

UDSScanner::UDSScanner() 
    : can_interface_(nullptr), telemetry_(nullptr), 
      request_count_(0), error_count_(0) {
}

void UDSScanner::begin(CANInterface& can_interface, TelemetryManager& telemetry) {
    can_interface_ = &can_interface;
    telemetry_ = &telemetry;
    isotp_.begin(can_interface);
    DEBUG_PRINTLN("UDSScanner initialized.");
}

bool UDSScanner::requestDID(const UDSDataItem& item) {
    if (!can_interface_ || !telemetry_) {
        return false;
    }

    request_count_++;

    // UDS-Request aufbauen: Service 0x22 + DID (2 Bytes)
    uint8_t request[3];
    request[0] = UDS_SERVICE_READ_DATA_BY_ID;
    request[1] = (item.did >> 8) & 0xFF;  // DID High-Byte
    request[2] = item.did & 0xFF;         // DID Low-Byte

    DEBUG_VERBOSE_PRINT("UDS Request: ECU=");
    DEBUG_VERBOSE_PRINT(item.ecu_name);
    DEBUG_VERBOSE_PRINT(" DID=0x");
    DEBUG_VERBOSE_PRINTLN(item.did, HEX);

    // Request senden via ISO-TP
    if (!isotp_.sendRequest(item.request_id, item.response_id, request, 3, UDS_TIMEOUT_MS)) {
        DEBUG_VERBOSE_PRINTLN("ERROR: ISO-TP request failed");
        error_count_++;
        return false;
    }

    // Response empfangen via ISO-TP
    uint8_t response[ISOTP_MAX_DATA_LEN];
    size_t response_len = 0;
    
    if (!isotp_.receiveResponse(response, response_len, UDS_TIMEOUT_MS)) {
        DEBUG_VERBOSE_PRINTLN("ERROR: ISO-TP response timeout or error");
        error_count_++;
        return false;
    }

    // Response validieren
    if (response_len < 3) {
        DEBUG_VERBOSE_PRINTLN("ERROR: Response too short");
        error_count_++;
        return false;
    }

    // Negative Response?
    if (response[0] == UDS_SERVICE_NEGATIVE_RESPONSE) {
        DEBUG_VERBOSE_PRINT("UDS Negative Response: NRC=0x");
        DEBUG_VERBOSE_PRINTLN(response[2], HEX);
        error_count_++;
        return false;
    }

    // Positive Response validieren
    uint8_t expected_service = UDS_SERVICE_READ_DATA_BY_ID + UDS_SERVICE_POSITIVE_RESPONSE_OFFSET;
    if (response[0] != expected_service) {
        DEBUG_VERBOSE_PRINTLN("ERROR: Unexpected service in response");
        error_count_++;
        return false;
    }

    // DID in Response validieren
    uint16_t response_did = (response[1] << 8) | response[2];
    if (response_did != item.did) {
        DEBUG_VERBOSE_PRINTLN("ERROR: DID mismatch in response");
        error_count_++;
        return false;
    }

    // Nutzdaten extrahieren (ab Byte 3)
    const uint8_t* data = &response[3];
    size_t data_len = response_len - 3;

    // Decoder aufrufen
    if (item.decoder) {
        item.decoder(data, data_len, *telemetry_);
    }

    DEBUG_VERBOSE_PRINT("UDS Response OK: DID=0x");
    DEBUG_VERBOSE_PRINT(item.did, HEX);
    DEBUG_VERBOSE_PRINT(" DataLen=");
    DEBUG_VERBOSE_PRINTLN(data_len);

    return true;
}

const UDSDataItem* UDSScanner::getDataItems(size_t& count) {
    count = uds_data_items_count;
    return uds_data_items;
}

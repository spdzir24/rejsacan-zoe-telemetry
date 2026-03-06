/**
 * @file telemetry.cpp
 * @brief Implementierung des Telemetrie-Managers
 */

#include "telemetry.h"
#include <string.h>

TelemetryManager::TelemetryManager() : data_point_count_(0) {
    memset(data_points_, 0, sizeof(data_points_));
}

void TelemetryManager::begin() {
    DEBUG_PRINTLN("TelemetryManager initialized.");
}

void TelemetryManager::updateDataPoint(const char* key, float value, const char* unit, TelemetrySource source) {
    int index = findOrCreateDataPoint(key);
    
    if (index < 0) {
        DEBUG_VERBOSE_PRINTLN("ERROR: Telemetry data point limit reached");
        return;
    }

    TelemetryDataPoint& dp = data_points_[index];
    dp.value = value;
    dp.unit = unit;
    dp.timestamp = millis();
    dp.source = source;
    dp.valid = true;
    dp.updated = true;  // Flag setzen für UART-Output
}

const TelemetryDataPoint* TelemetryManager::getDataPoints(size_t& count) const {
    count = data_point_count_;
    return data_points_;
}

void TelemetryManager::clearUpdatedFlags() {
    for (size_t i = 0; i < data_point_count_; i++) {
        data_points_[i].updated = false;
    }
}

int TelemetryManager::findOrCreateDataPoint(const char* key) {
    // Suche nach existierendem Datenpunkt
    for (size_t i = 0; i < data_point_count_; i++) {
        if (strcmp(data_points_[i].key, key) == 0) {
            return i;
        }
    }

    // Neuen Datenpunkt anlegen
    if (data_point_count_ >= MAX_DATA_POINTS) {
        return -1;  // Limit erreicht
    }

    int index = data_point_count_++;
    data_points_[index].key = key;
    data_points_[index].value = 0.0f;
    data_points_[index].unit = "";
    data_points_[index].timestamp = 0;
    data_points_[index].source = TELEMETRY_SOURCE_PASSIVE;
    data_points_[index].valid = false;
    data_points_[index].updated = false;

    return index;
}

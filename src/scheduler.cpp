/**
 * @file scheduler.cpp
 * @brief Implementierung des Task-Schedulers
 */

#include "scheduler.h"

Scheduler::Scheduler() 
    : last_fast_poll_(0), last_medium_poll_(0), last_slow_poll_(0) {
}

void Scheduler::begin() {
    DEBUG_PRINTLN("Scheduler initialized.");
}

void Scheduler::update(UDSScanner& scanner) {
    uint32_t now = millis();

    // Alle UDS-Datenpunkte abrufen
    size_t count = 0;
    const UDSDataItem* items = UDSScanner::getDataItems(count);

    // ---------------------------------------------------------
    // Fast-Poll (1s)
    // ---------------------------------------------------------
    if (now - last_fast_poll_ >= UDS_POLL_INTERVAL_FAST_MS) {
        for (size_t i = 0; i < count; i++) {
            if (items[i].poll_class == UDS_POLL_FAST) {
                scanner.requestDID(items[i]);
                delay(50);  // Kurze Pause zwischen Requests
            }
        }
        last_fast_poll_ = now;
    }

    // ---------------------------------------------------------
    // Medium-Poll (5s)
    // ---------------------------------------------------------
    if (now - last_medium_poll_ >= UDS_POLL_INTERVAL_MEDIUM_MS) {
        for (size_t i = 0; i < count; i++) {
            if (items[i].poll_class == UDS_POLL_MEDIUM) {
                scanner.requestDID(items[i]);
                delay(50);
            }
        }
        last_medium_poll_ = now;
    }

    // ---------------------------------------------------------
    // Slow-Poll (30s)
    // ---------------------------------------------------------
    if (now - last_slow_poll_ >= UDS_POLL_INTERVAL_SLOW_MS) {
        for (size_t i = 0; i < count; i++) {
            if (items[i].poll_class == UDS_POLL_SLOW) {
                scanner.requestDID(items[i]);
                delay(50);
            }
        }
        last_slow_poll_ = now;
    }
}

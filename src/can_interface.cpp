/**
 * @file can_interface.cpp
 * @brief Implementierung des TWAI-Interfaces
 */

#include "can_interface.h"

CANInterface::CANInterface() 
    : initialized_(false), error_count_(0), last_error_time_(0) {
}

bool CANInterface::begin() {
    // TWAI-Konfiguration
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(
        TWAI_TX_GPIO, 
        TWAI_RX_GPIO, 
        TWAI_MODE_LISTEN_ONLY  // Nur empfangen (sicher!)
    );
    g_config.rx_queue_len = CAN_RX_QUEUE_SIZE;

    // Timing-Konfiguration für 500 kbit/s
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();

    // Filter-Konfiguration (alle Frames akzeptieren)
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    // TWAI-Treiber installieren
    esp_err_t err = twai_driver_install(&g_config, &t_config, &f_config);
    if (err != ESP_OK) {
        DEBUG_PRINT("ERROR: twai_driver_install failed: ");
        DEBUG_PRINTLN(esp_err_to_name(err));
        return false;
    }

    // TWAI starten
    err = twai_start();
    if (err != ESP_OK) {
        DEBUG_PRINT("ERROR: twai_start failed: ");
        DEBUG_PRINTLN(esp_err_to_name(err));
        twai_driver_uninstall();
        return false;
    }

    initialized_ = true;
    DEBUG_PRINTLN("TWAI driver started in LISTEN_ONLY mode.");
    return true;
}

bool CANInterface::receive(twai_message_t& frame, uint32_t timeout_ms) {
    if (!initialized_) {
        return false;
    }

    TickType_t ticks = (timeout_ms == 0) ? 0 : pdMS_TO_TICKS(timeout_ms);
    esp_err_t err = twai_receive(&frame, ticks);

    if (err == ESP_OK) {
        return true;
    } else if (err == ESP_ERR_TIMEOUT) {
        // Timeout ist normal, kein Fehler
        return false;
    } else {
        // Echter Fehler
        DEBUG_VERBOSE_PRINT("WARNING: twai_receive error: ");
        DEBUG_VERBOSE_PRINTLN(esp_err_to_name(err));
        error_count_++;
        last_error_time_ = millis();
        return false;
    }
}

bool CANInterface::send(const twai_message_t& frame, uint32_t timeout_ms) {
    if (!initialized_) {
        return false;
    }

    TickType_t ticks = pdMS_TO_TICKS(timeout_ms);
    esp_err_t err = twai_transmit(&frame, ticks);

    if (err == ESP_OK) {
        return true;
    } else {
        DEBUG_VERBOSE_PRINT("WARNING: twai_transmit error: ");
        DEBUG_VERBOSE_PRINTLN(esp_err_to_name(err));
        error_count_++;
        last_error_time_ = millis();
        return false;
    }
}

bool CANInterface::isBusHealthy() {
    if (!initialized_) {
        return false;
    }

    twai_status_info_t status;
    esp_err_t err = twai_get_status_info(&status);
    
    if (err != ESP_OK) {
        return false;
    }

    // Bus-Off-Zustand prüfen
    if (status.state == TWAI_STATE_BUS_OFF) {
        DEBUG_PRINTLN("WARNING: TWAI bus is in BUS_OFF state!");
        return false;
    }

    return true;
}

bool CANInterface::recoverBus() {
    if (!initialized_) {
        return false;
    }

    DEBUG_PRINTLN("Attempting TWAI bus recovery...");

    // Bus stoppen
    esp_err_t err = twai_stop();
    if (err != ESP_OK) {
        DEBUG_PRINT("ERROR: twai_stop failed: ");
        DEBUG_PRINTLN(esp_err_to_name(err));
        return false;
    }

    delay(100);  // Kurze Pause

    // Bus neu starten
    err = twai_start();
    if (err != ESP_OK) {
        DEBUG_PRINT("ERROR: twai_start failed: ");
        DEBUG_PRINTLN(esp_err_to_name(err));
        return false;
    }

    DEBUG_PRINTLN("TWAI bus recovered successfully.");
    return true;
}

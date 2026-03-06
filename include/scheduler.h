/**
 * @file scheduler.h
 * @brief Task-Scheduler für UDS-Anfragen
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <Arduino.h>
#include "config.h"
#include "uds_scanner.h"

class Scheduler {
public:
    Scheduler();
    
    /**
     * @brief Initialisiert den Scheduler
     */
    void begin();
    
    /**
     * @brief Aktualisiert den Scheduler (ruft zyklisch in loop() auf)
     * @param scanner Referenz zum UDS-Scanner
     */
    void update(UDSScanner& scanner);

private:
    uint32_t last_fast_poll_;
    uint32_t last_medium_poll_;
    uint32_t last_slow_poll_;
};

#endif // SCHEDULER_H

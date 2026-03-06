# Software-Architektur: RejsaCAN Zoe Telemetry System

## Übersicht

Dieses Dokument beschreibt die technische Architektur des CAN-Telemetriesystems für den Renault Zoe ZE50 Phase 2 auf Basis eines ESP32-C6 mit RejsaCAN v6.x Hardware.

## Design-Entscheidungen

### 1. ESP32-C6 TWAI-Controller

**Entscheidung**: Direkter Zugriff auf ESP32-C6 TWAI (Two-Wire Automotive Interface) via Arduino-Framework.

**Begründung**:
- ESP32-C6 hat keinen klassischen CAN-Controller, sondern TWAI (kompatibel zu CAN 2.0B)
- Arduino-Framework bietet stabile `driver/twai.h` API
- Keine zusätzliche externe CAN-Library nötig
- Magnus Thomé empfiehlt TWAI-nahe Ansätze für neuere ESP32-Varianten (S3, C6)
- Bessere Performance und Kontrolle als über Abstraktions-Libraries

**Alternativen (verworfen)**:
- `ESP32-Arduino-CAN` Library: Nicht für C6 getestet
- `ACAN_ESP32` Library: Unterstützt C6 nicht explizit
- Externe MCP2515-CAN-Controller: Unnötig, da TWAI vorhanden

### 2. Modulare Architektur

**Komponenten**:
```
src/
├── main.cpp              # Hauptprogramm, Initialisierung, Loop
├── can_interface.cpp/.h  # TWAI-Treiber, Low-Level CAN-Zugriff
├── can_decoder.cpp/.h    # Passive Frame-Dekodierung
├── uds_scanner.cpp/.h    # UDS-Diagnose (ISO 14229, ISO 15765)
├── isotp.cpp/.h          # ISO-TP Transport-Layer
├── telemetry.cpp/.h      # Zentrales Datenmodell
├── uart_output.cpp/.h    # UART-Serialisierung (JSON Lines)
├── scheduler.cpp/.h      # Task-Scheduler (Fast/Medium/Slow)
└── config.h              # Zentrale Konfiguration
```

**Vorteile**:
- Klare Verantwortlichkeiten
- Einfaches Testen einzelner Module
- Leichte Erweiterbarkeit
- Wiederverwendbare Komponenten

### 3. Passive-First-Strategie

**Entscheidung**: Passive CAN-Frames haben Priorität, UDS ist optional.

**Begründung**:
- **Security Gateway**: Renault Zoe ZE50 PH2 hat SGW am OBD2-Port
- **Sicherheit**: Passive Frames können Bus nicht stören
- **Zuverlässigkeit**: Passive Daten immer verfügbar
- **Busload**: Keine zusätzliche Last durch Anfragen

**UDS nur wenn**:
- Explizit aktiviert via `-DENABLE_UDS_DIAGNOSTICS=1`
- Notwendig für kritische Daten (z.B. Real SOC)
- Mit Timeouts und Retry-Limits abgesichert

### 4. UART als primäre Datenschnittstelle

**Entscheidung**: JSON Lines Format über UART (115200 baud).

**Begründung**:
- **Maschinenlesbar**: Jede Zeile = valides JSON-Objekt
- **Robust**: Zeilenbasiert, Parsing-Fehler isoliert
- **Einfach**: Standard-JSON-Parser in allen Sprachen
- **Erweiterbar**: Neue Felder ohne Protokolländerung
- **Backhaul-ready**: Zweiter ESP32 kann direkt parsen

**Alternativen (verworfen)**:
- **Binärprotokoll**: Komplexer, schwerer zu debuggen
- **MessagePack**: Overhead für kleine Nachrichten
- **Protobuf**: Overhead für Embedded-System
- **Plain Text**: Nicht maschinenlesbar, fehleranfällig

**Beispiel-Ausgabe**:
```json
{"type":"telemetry","ts":12345,"source":"passive","frame_id":"0x427","data":{"available_energy_kwh":45.2}}
{"type":"telemetry","ts":12346,"source":"uds_fast","ecu":"EVC","did":"0x20BE","data":{"real_soc_pct":87.5}}
```

### 5. Dreistufiger Scheduler

**Entscheidung**: Drei Polling-Klassen mit unterschiedlichen Intervallen.

**Kategorien**:
- **Fast (1s)**: Dynamische Fahrdaten (SOC, Strom, Spannung, Drehzahl)
- **Medium (5s)**: Langsam ändernde Werte (Temperatur, Ladeleistung)
- **Slow (30-60s)**: Statische Daten (VIN, SOH, Kapazität)

**Begründung**:
- **Busschonend**: Nur nötige Frequenz pro Datenpunkt
- **SGW-freundlich**: Vermeidet Überlastung des Security Gateways
- **Ressourcen-effizient**: Weniger CPU-Last, weniger UART-Traffic
- **Priorisierung**: Kritische Daten häufiger

### 6. ISO-TP-Implementierung

**Entscheidung**: Minimale, aber funktionale ISO-TP-Implementierung für UDS.

**Umfang**:
- **Single-Frame**: Antworten ≤ 7 Bytes
- **Multi-Frame**: Antworten > 7 Bytes
  - First Frame (FF) Empfang
  - Flow Control (FC) Senden: "ContinueToSend"
  - Consecutive Frames (CF) Empfang und Reassembly
- **Timeouts**: Konfigurierbar pro Request

**Nicht implementiert (aktuell)**:
- **Flow Control mit Wait**: Blockierung wird nicht gehandhabt
- **Flow Control mit Overflow**: Als Fehler behandelt
- **Erweiterte Adressierung**: Nur Normal Addressing
- **CAN-FD**: Nur CAN 2.0B

**Begründung**:
- **Ausreichend**: Renault Zoe nutzt Standard-ISO-TP
- **Einfach**: Weniger Code, weniger Fehlerquellen
- **Erweiterbar**: Flow Control kann später ergänzt werden

### 7. Zentrales Telemetrie-Datenmodell

**Entscheidung**: Alle Daten laufen durch `TelemetryManager`.

**Struktur**:
```cpp
struct TelemetryDataPoint {
    const char* key;           // Eindeutiger Bezeichner
    float value;               // Numerischer Wert
    const char* unit;          // Einheit (optional)
    uint32_t timestamp;        // Millisekunden seit Boot
    TelemetrySource source;    // passive, uds_fast, etc.
    bool valid;                // Gültigkeit
};
```

**Vorteile**:
- **Entkopplung**: Decoder kennen UART-Format nicht
- **Konsistenz**: Einheitliche Datenrepräsentation
- **Testbarkeit**: Mock-Implementierung möglich
- **Erweiterbarkeit**: Neue Outputs (MQTT, SD, Display) trivial

### 8. Konfiguration zur Compile-Time

**Entscheidung**: Alle Einstellungen als Preprocessor-Defines in `platformio.ini`.

**Begründung**:
- **Performance**: Keine Runtime-Checks nötig
- **Code-Size**: Ungenutzter Code wird wegoptimiert
- **Sicherheit**: Keine versehentliche Änderung zur Laufzeit
- **Einfachheit**: Keine EEPROM/Flash-Konfiguration nötig

**Nachteil**:
- Änderungen erfordern Neucompilierung
- Akzeptabel für Prototyp und Embedded-System

### 9. Fehlerbehandlung

**Strategie**:
- **Defensive Programmierung**: Checks auf NULL, Bounds, Timeouts
- **Graceful Degradation**: UDS-Fehler stoppen nicht passive Frames
- **Retry-Logik**: Maximale Wiederholungen mit Backoff
- **Error-Output**: Fehler über UART als `{"type":"error",...}`
- **Watchdog**: ESP32 interner Watchdog (TODO)

**CAN-Bus-Fehler**:
- **Bus-Off**: Automatischer Neustart via TWAI-Recovery
- **Timeout**: Request abbrechen nach konfigurierter Zeit
- **Malformed Frames**: Ignorieren, loggen

**UDS-Fehler**:
- **Timeout**: Max. Retries, dann Datenpunkt als invalid markieren
- **Negative Response**: Loggen, nicht wiederholen
- **SGW-Block**: Exponentieller Backoff (TODO)

## Datenfluss

### Passive Frames

```
1. CAN-Frame empfangen (CANInterface::receive())
    ↓
2. Frame-ID prüfen (CANDecoder::isKnownFrame())
    ↓
3. Frame dekodieren (CANDecoder::decodeFrame())
    ↓
4. Datenpunkt extrahieren (z.B. SOC aus 0x427)
    ↓
5. An Telemetry übergeben (TelemetryManager::updateDataPoint())
    ↓
6. UART-Ausgabe (UARTOutput::sendTelemetry())
```

### UDS-Requests

```
1. Scheduler prüft Intervall (Scheduler::update())
    ↓
2. UDS-Request bauen (UDSScanner::requestDID())
    ↓
3. ISO-TP Single-Frame senden (ISOTP::sendSingleFrame())
    ↓
4. Warten auf Response mit Timeout
    ↓
5. ISO-TP-Response reassemblen (ISOTP::receiveFrame())
    ↓  
6. UDS-Response dekodieren (UDSScanner::decodeResponse())
    ↓
7. An Telemetry übergeben
    ↓
8. UART-Ausgabe
```

## Timing-Budgets

### CAN-Bus (500 kbit/s)

**Single-Frame (8 Bytes)**:
- Übertragungszeit: ~160 µs
- Mit Arbitration: ~200 µs

**Multi-Frame (64 Bytes, max. ISO-TP)**:
- First Frame: ~200 µs
- Flow Control: ~200 µs
- 8x Consecutive Frames: ~1.6 ms
- Gesamt: ~2 ms

### UDS-Timing

**Standard-Request**:
- Request senden: ~200 µs
- ECU-Verarbeitung: 10-50 ms (geschätzt)
- Response empfangen: ~200 µs - 2 ms (Single/Multi-Frame)
- **Worst-Case**: ~100 ms
- **Timeout konfiguriert**: 200 ms (sicher)

**Scheduler-Budget**:
- Fast (1s): Max. 10 DIDs → 10 × 100 ms = 1000 ms (100% Auslastung)
- Medium (5s): Max. 5 DIDs → 5 × 100 ms = 500 ms (10% Auslastung)
- Slow (30s): Max. 5 DIDs → 5 × 100 ms = 500 ms (1.7% Auslastung)

**→ Aktuelle Konfiguration ist busschonend!**

## Speicherverbrauch

### Flash (Programmspeicher)

**Geschätzt**:
- Arduino-Framework: ~200 KB
- Eigener Code: ~50-80 KB
- ArduinoJson: ~30 KB
- **Gesamt**: ~280-310 KB
- **ESP32-C6 verfügbar**: 4 MB → 8% Auslastung

### RAM (SRAM)

**Geschätzt**:
- Stack: ~16 KB
- CAN-RX-Queue (50 Frames): 50 × 16 B = 800 B
- ISO-TP-Buffer (2× 512 B): 1 KB
- Telemetry-Buffer (512 B): 512 B
- JSON-Serialisierung: ~2 KB
- **Gesamt**: ~20 KB
- **ESP32-C6 verfügbar**: 512 KB → 4% Auslastung

## Sicherheit und Robustheit

### Gegen SGW-Blockierung

1. **Rate Limiting**: Max. Anfragen pro Sekunde limitiert
2. **Timeout**: Keine unbegrenzten Wartezeiten
3. **Retry-Limit**: Max. 2 Wiederholungen
4. **Graceful Degradation**: Bei Fehler Datenpunkt als invalid, aber System läuft weiter
5. **Session-Reset**: Bei kritischen Fehlern UDS-Session zurücksetzen (TODO)

### Gegen CAN-Bus-Fehler

1. **TWAI-Recovery**: Automatischer Neustart bei Bus-Off
2. **Frame-Validierung**: CAN-ID und DLC prüfen
3. **Timeout**: Keine blockierenden Empfangsoperationen
4. **Queue-Overflow-Handling**: Älteste Frames verwerfen

### Gegen Software-Fehler

1. **NULL-Checks**: Alle Pointer vor Zugriff prüfen
2. **Bounds-Checks**: Array-Zugriffe validieren
3. **Watchdog**: ESP32 Watchdog (TODO: explizit konfigurieren)
4. **Error-Logging**: Alle Fehler über UART ausgeben

## Erweiterbarkeit

### Neue passive Frames hinzufügen

**Schritte**:
1. Decoder-Funktion in `can_decoder.cpp` schreiben:
   ```cpp
   void decodeFrame_0xXXX(const twai_message_t& frame, 
                          TelemetryManager& telemetry) {
       float value = (frame.data[0] << 8 | frame.data[1]) * 0.01;
       telemetry.updateDataPoint("my_value", value, "unit", 
                                  TELEMETRY_SOURCE_PASSIVE);
   }
   ```

2. Frame-Descriptor registrieren:
   ```cpp
   decoder.registerFrame(0xXXX, "my_frame", decodeFrame_0xXXX);
   ```

3. Kompilieren, flashen, testen.

### Neue UDS-DIDs hinzufügen

**Schritte**:
1. DID-Descriptor in `uds_scanner.cpp` ergänzen:
   ```cpp
   {0x7E2, 0xYYYY, "my_did", UDS_POLL_MEDIUM, decodeMyDID}
   ```

2. Decoder-Funktion schreiben:
   ```cpp
   void decodeMyDID(const uint8_t* data, size_t len, 
                    TelemetryManager& telemetry) {
       float value = (data[0] << 8 | data[1]) * 0.1;
       telemetry.updateDataPoint("my_value", value, "unit", 
                                  TELEMETRY_SOURCE_UDS_MEDIUM);
   }
   ```

3. Kompilieren, flashen, testen.

### UART-Backhaul-Modul anbinden

**Hardware**:
- ESP32 (beliebige Variante)
- SIM70xx LTE-Modem (z.B. SIM7020G für NB-IoT)
- GPS-Modul (z.B. NEO-6M, optional integriert in SIM70xx)
- UART-Verbindung:
  - TX von RejsaCAN → RX von Backhaul-ESP32
  - GND gemeinsam
  - (Optional: RX von RejsaCAN → TX von Backhaul für Kommandos)

**Software (Backhaul-ESP32)**:
```cpp
// Pseudo-Code
void loop() {
    String line = Serial.readStringUntil('\n');
    JsonDocument doc = parseJson(line);
    
    if (doc["type"] == "telemetry") {
        doc["gps_lat"] = gps.latitude();
        doc["gps_lon"] = gps.longitude();
        mqtt.publish("zoe/telemetry", doc.serializeToString());
    }
}
```

**→ Keine Änderungen am RejsaCAN-Code nötig!**

## Offene Punkte und Risiken

### Bekannte Einschränkungen

1. **SGW-Verhalten**: Nicht vollständig dokumentiert
   - **Risiko**: SGW könnte bei bestimmten DIDs oder Frequenzen blockieren
   - **Mitigation**: Konservative Intervalle, Retry-Limits, Logging

2. **ISO-TP Flow Control**: Nur "ContinueToSend" implementiert
   - **Risiko**: Bei langsamen ECUs könnte Overflow auftreten
   - **Mitigation**: Renault ECUs sind üblicherweise schnell, kann später ergänzt werden

3. **Keine CRC-Prüfung**: UDS-Responses haben keine zusätzliche Checksumme
   - **Risiko**: Korrupte Daten unerkannt
   - **Mitigation**: CAN-Bus hat eigene CRC, sehr unwahrscheinlich

4. **Kein Watchdog**: Expliziter Watchdog-Reset fehlt
   - **Risiko**: System könnte bei Fehler hängen
   - **Mitigation**: ESP32 hat Default-Watchdog, kann explizit gemacht werden

5. **Frame 0x5D7**: Multiplexing noch nicht dekodiert
   - **Risiko**: Zellspannungen nicht verfügbar
   - **Mitigation**: Frame wird empfangen, Decoder kann später ergänzt werden

### Ungeklärte Fragen

1. **SGW-Freigabe**: Welche DIDs sind garantiert freigeschaltet?
   - Nur durch Tests im Fahrzeug klärbar

2. **CAN-Bitrate**: 500 kbit/s ist üblich, aber nicht offiziell bestätigt
   - Sollte funktionieren, ansonsten auf 250 kbit/s ändern

3. **GPIO-Pins**: TWAI-Pins für RejsaCAN v6.x
   - GPIO 4 (RX) und GPIO 5 (TX) sind angenommen
   - Falls falsch, in `config.h` anpassen

## Performance-Metriken (Erwartungswerte)

### Datenraten

**Passive Frames**:
- Annahme: 50 Frames/s auf Bus
- Relevante Frames: ~5-10 Frames/s
- UART-Output: ~500 Bytes/s

**UDS-Requests**:
- Fast: 10 DIDs/s (Worst-Case)
- Medium: 1 DID/s (gemittelt)
- Slow: 0.17 DIDs/s (gemittelt)
- **Gesamt**: ~11 Requests/s (Worst-Case)

**UART-Gesamt**:
- Telemetrie: ~500 B/s (passiv) + ~2000 B/s (UDS) = 2.5 KB/s
- Heartbeat: ~50 B/5s = 10 B/s
- Fehler: negligible
- **Gesamt**: ~2.6 KB/s
- **Baudrate**: 115200 baud = 11.5 KB/s → **23% Auslastung**

### Latenz

**Passive Frame → UART**:
- CAN-Empfang: < 1 ms
- Dekodierung: < 1 ms
- JSON-Serialisierung: < 2 ms
- UART-Senden: < 5 ms
- **Gesamt**: < 10 ms

**UDS-Request → UART**:
- Request-Senden: ~0.2 ms
- ECU-Antwort: 10-50 ms
- ISO-TP-Reassembly: < 5 ms
- Dekodierung: < 1 ms
- JSON-Serialisierung: < 2 ms
- **Gesamt**: 15-60 ms (typisch ~30 ms)

## Zukünftige Erweiterungen

### Kurzfristig (v1.1)

- [ ] Vollständige 0x5D7-Dekodierung (Zellspannungen)
- [ ] Explizite Watchdog-Konfiguration
- [ ] SD-Karten-Logging (optional)
- [ ] WiFi-Konfiguration via Access Point (optional)

### Mittelfristig (v2.0)

- [ ] UART-Backhaul-Modul (separates Projekt)
- [ ] MQTT-Bridge im Backhaul-Modul
- [ ] GPS-Integration im Backhaul-Modul
- [ ] Home Assistant MQTT Discovery

### Langfristig (v3.0)

- [ ] CAN-Senden (nach ausgiebigen Tests!)
- [ ] Remote-Konfiguration via MQTT
- [ ] OTA-Updates
- [ ] Multi-Fahrzeug-Unterstützung (Konfigurations-Profiles)

## Schlussfolgerung

Die gewählte Architektur ist:
- ✅ **Modular**: Klare Trennung der Verantwortlichkeiten
- ✅ **Erweiterbar**: Neue Datenpunkte trivial hinzufügbar
- ✅ **Robust**: Fehlerbehandlung und Timeouts
- ✅ **SGW-freundlich**: Busschonend und passiv-first
- ✅ **Zukunftssicher**: UART-Backhaul-ready
- ✅ **Performant**: < 25% UART-Auslastung, < 10% RAM

Das System ist bereit für erste Fahrzeugtests im passiven Modus.

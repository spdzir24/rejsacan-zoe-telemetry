# RejsaCAN Zoe Telemetry System

## Professionelles CAN-Telemetriesystem für Renault Zoe ZE50 Phase 2

### Hardware
- **Board**: RejsaCAN v6.x mit ESP32-C6
- **MCU**: ESP32-C6 @ 160 MHz
- **CAN-Interface**: Integrierter TWAI-Controller (500 kbit/s)
- **Anschluss**: Direkt am OBD2-Port unter dem Lenkrad

### Funktionsumfang

#### Aktuelle Version (v1.0 - Lokale Telemetrie)
- ✅ **Passive CAN-Frame-Erfassung** (Standard-Modus)
- ✅ **Optionale UDS-Diagnose** (konfigurierbar deaktiviert)
- ✅ **UART-Datenausgabe** (JSON Lines Format)
- ✅ **Zentrales Telemetrie-Datenmodell**
- ✅ **Dreistufiger Scheduler** (schnell/mittel/langsam)
- ✅ **ISO-TP Multi-Frame-Unterstützung**
- ✅ **Robuste Fehlerbehandlung**

#### Zukünftige Erweiterungen
- 🔄 **UART-Backhaul-Modul** (zweiter ESP32 mit SIM70xx + GPS)
- 🔄 **MQTT-Übertragung** (via Backhaul-Modul)
- 🔄 **GPS-Standortdaten** (via Backhaul-Modul)
- 🔄 **Home Assistant Integration** (via MQTT)

### Architektur

```
┌─────────────────────────────────────────────────────────────┐
│                    ESP32-C6 (RejsaCAN)                      │
│                                                             │
│  ┌──────────────┐  ┌─────────────┐  ┌──────────────┐      │
│  │ Passive CAN  │  │ UDS Scanner │  │  Telemetry   │      │
│  │  Sniffer     │  │ (optional)  │  │  Data Model  │      │
│  └──────┬───────┘  └──────┬──────┘  └──────┬───────┘      │
│         │                 │                 │              │
│         └─────────────────┴─────────────────┘              │
│                          │                                 │
│                  ┌───────▼────────┐                        │
│                  │ UART Telemetry │                        │
│                  │  Output (JSON) │                        │
│                  └───────┬────────┘                        │
└──────────────────────────┼─────────────────────────────────┘
                           │ UART (115200 baud)
                           ▼
            ┌──────────────────────────────┐
            │  Zukünftiges Backhaul-Modul  │
            │  (ESP32 + SIM70xx + GPS)     │
            │  • MQTT Publishing           │
            │  • GPS Tracking              │
            │  • Cloud Connectivity        │
            └──────────────────────────────┘
```

### Sicherheit und Kompatibilität

#### Security Gateway (SGW) Awareness
Der Renault Zoe ZE50 Phase 2 verfügt über ein Security Gateway am OBD2-Port:
- **Standard-Modus**: Nur passive Frames (empfohlen)
- **UDS-Modus**: Freigegebene DIDs über Service 0x22 (optional)
- **Timing**: Busschonende Intervalle mit Timeouts
- **Schutz**: Maximale Retry-Limits und Session-Reset

### Datenpunkte

#### Passive Frames
- **0x427**: Verfügbare Batterieenergie
- **0x5D7**: Zellspannungen (multiplexed, ZE50-spezifisch)

#### UDS-Diagnose (optional)

**EVC (0x7E2 → 0x7EA)** - Fahrzeug-Controller:
- Real SOC (DID 0x20BE)
- Energie pro SOC% (DID 0x303E)
- HV-Batteriespannung (DID 0x20FE)
- HV-Batteriestrom (DID 0x21CC)
- Motordrehzahl (DID 0x3064)

**LBC (0x7E4 → 0x7EC)** - Batterie-Management:
- Batteriespannung (DID 0x3203)
- Batteriestrom (DID 0x3204)
- Verfügbare Entladeenergie (DID 0x502C)
- State of Health (DID 0x0101)
- Max. Ladeleistung (DID 0x3206)
- Max. Zelltemperatur (DID 0x320B)

**BCM (0x771 → 0x779)** - Body Control Module:
- 12V-Batteriespannung (DID 0x2002)

**Multimedia (0x7B5 → 0x7BD)** - Infotainment:
- VIN (DID 0xF190, nur beim Start)

### Konfiguration

#### Compile-Time-Konfiguration (platformio.ini)

```ini
; Funktionen aktivieren/deaktivieren
-DENABLE_PASSIVE_SNIFFING=1     ; Passive Frames lesen
-DENABLE_UDS_DIAGNOSTICS=0      ; UDS-Anfragen (Standard: AUS)
-DENABLE_DEBUG_OUTPUT=1         ; Debug-Meldungen
-DENABLE_HEARTBEAT=1            ; Heartbeat über UART

; CAN-Konfiguration
-DCAN_BITRATE=500000            ; 500 kbit/s
-DTWAI_TX_GPIO=GPIO_NUM_5       ; TX Pin
-DTWAI_RX_GPIO=GPIO_NUM_4       ; RX Pin

; UDS-Timing
-DUDS_POLL_INTERVAL_FAST_MS=1000    ; Schnelle Daten (1s)
-DUDS_POLL_INTERVAL_MEDIUM_MS=5000  ; Mittlere Daten (5s)
-DUDS_POLL_INTERVAL_SLOW_MS=30000   ; Langsame Daten (30s)
-DUDS_TIMEOUT_MS=200                ; Antwort-Timeout
-DUDS_MAX_RETRIES=2                 ; Max. Wiederholungen
```

#### Build-Environments

```bash
# Standard: Nur passive Frames, UDS deaktiviert
pio run -e esp32-c6-devkitc-1

# Debug: Erweiterte Logs
pio run -e esp32-c6-devkitc-1-debug

# Produktion mit UDS: UDS-Anfragen aktiviert
pio run -e esp32-c6-devkitc-1-uds
```

### UART-Telemetrie-Ausgabe

#### JSON Lines Format
Jede Zeile ist ein valides JSON-Objekt:

```json
{"type":"heartbeat","ts":12345,"uptime":12345}
{"type":"telemetry","ts":12346,"source":"passive","frame_id":"0x427","data":{"available_energy_kwh":45.2}}
{"type":"telemetry","ts":12347,"source":"uds_fast","ecu":"EVC","did":"0x20BE","data":{"real_soc_pct":87.5}}
{"type":"error","ts":12348,"module":"uds","message":"Timeout on DID 0x3203"}
```

#### Datenfelder
- `type`: Nachrichtentyp (`telemetry`, `heartbeat`, `error`, `debug`)
- `ts`: Timestamp in Millisekunden seit Boot
- `source`: Datenquelle (`passive`, `uds_fast`, `uds_medium`, `uds_slow`)
- `data`: Nutzdaten als JSON-Objekt

### Installation und Verwendung

#### Voraussetzungen
- **VS Code** mit PlatformIO Extension
- **USB-C-Kabel** für ESP32-C6
- **RejsaCAN v6.x Hardware**

#### Flash-Prozess

```bash
# Repository klonen
git clone https://github.com/spdzir24/rejsacan-zoe-telemetry.git
cd rejsacan-zoe-telemetry

# Kompilieren und flashen (Standard-Modus)
pio run -e esp32-c6-devkitc-1 -t upload

# Monitor starten
pio device monitor -b 115200
```

#### Test ohne Fahrzeug

```bash
# Nur kompilieren (Syntaxprüfung)
pio run -e esp32-c6-devkitc-1

# Upload ohne Auto (LED blinkt, UART gibt Heartbeat aus)
pio run -e esp32-c6-devkitc-1 -t upload
pio device monitor
```

#### Produktiv-Einsatz im Fahrzeug

1. **Erste Tests**: Nur passive Frames
   ```bash
   pio run -e esp32-c6-devkitc-1 -t upload
   ```

2. **Erweiterte Tests**: UDS aktivieren
   - In `platformio.ini` ändern: `-DENABLE_UDS_DIAGNOSTICS=1`
   - Oder fertiges Environment nutzen:
   ```bash
   pio run -e esp32-c6-devkitc-1-uds -t upload
   ```

3. **Datenerfassung über UART**:
   ```bash
   # Linux/Mac
   cat /dev/ttyUSB0 > zoe_telemetry.jsonl
   
   # Windows (PowerShell)
   Get-Content COM3 > zoe_telemetry.jsonl
   ```

### Bekannte Einschränkungen

#### Hardware
- **ESP32-C6**: Kein klassischer CAN-Controller, sondern TWAI (Two-Wire Automotive Interface)
- **RejsaCAN v6.x**: GPIO-Pins für TWAI sind boardspezifisch

#### Fahrzeug (Renault Zoe ZE50 Phase 2)
- **Security Gateway**: Blockiert viele Standard-OBD2-PIDs
- **UDS-Zugriff**: Nur freigegebene DIDs über Service 0x22
- **Timing-sensitiv**: SGW kann bei zu vielen Anfragen blockieren
- **Keine Standard-OBD2-Mode-01-PIDs**: Geschwindigkeit, Drehzahl etc. nur über Renault-spezifische DIDs

#### Software
- **ISO-TP**: Flow Control noch nicht vollständig implementiert (Multi-Frame funktional)
- **Keine CRC-Prüfung**: Datenintegrität nur durch UDS-Protokoll
- **Keine persistente Konfiguration**: Alle Einstellungen zur Compile-Zeit

### Erweiterbarkeit

#### Neue Datenpunkte hinzufügen

**Passive Frames** in `src/can_decoder.cpp`:
```cpp
void CANDecoder::addFrameDescriptor(uint32_t frame_id, 
                                     const char* name,
                                     FrameDecoder decoder) {
    // Tabelle erweitern
}
```

**UDS-DIDs** in `src/uds_scanner.cpp`:
```cpp
const UDSDataItem uds_data_items[] = {
    {0x7E2, 0x20BE, "real_soc_pct", UDS_POLL_FAST, decodeRealSOC},
    // Weitere DIDs hinzufügen
};
```

#### UART-Backhaul-Modul anbinden

**Hardware**: Zweiter ESP32 mit:
- SIM70xx LTE-Modem
- GPS-Modul
- UART-Verbindung zu RejsaCAN

**Software-Integration**:
1. Backhaul-Modul liest UART (JSON Lines)
2. Parst Telemetriedaten
3. Fügt GPS-Koordinaten hinzu
4. Überträgt via MQTT an Cloud/Home Assistant

**Keine Änderungen am RejsaCAN-Code nötig!**

### Lizenz

MIT License - Siehe LICENSE-Datei

### Autor

Generiert für professionelle Automotive-Embedded-Entwicklung auf Basis von:
- RejsaCAN v6.x Hardware-Plattform
- ESP32-C6 TWAI-Controller
- Renault Zoe ZE50 Phase 2 CAN-Bus-Spezifikationen

### Support und Beiträge

**Issues**: Bug-Reports und Feature-Requests über GitHub Issues
**Pull Requests**: Willkommen für neue Datenpunkte, Decoder, Bugfixes

### Haftungsausschluss

Diese Software dient ausschließlich zu Forschungs- und Entwicklungszwecken. Der Einsatz im Straßenverkehr erfolgt auf eigene Verantwortung. Der Autor übernimmt keine Haftung für Schäden am Fahrzeug oder Datenverlust.

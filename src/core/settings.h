// Zentrale, im NVS abgelegte Einstellungen.
//
// Die Werte aus include/config.h sind die Werksvorgabe; was zur Laufzeit
// geaendert wird, ueberschreibt sie beim naechsten Start.
#pragma once

#include <stddef.h>
#include <stdint.h>

#include "desk/presets.h"

namespace core {

struct Settings {
    // Netz
    char wifiSsid[33] = {};
    char wifiPassword[65] = {};

    // Ort und Zeit
    char locationName[32] = {};
    float latitude = 0.0f;
    float longitude = 0.0f;
    char timezone[48] = {};
    char ntpServer[48] = {};

    // Anzeige
    uint8_t brightness = 180;
    uint32_t sleepAfterMs = 0;

    // Schreibtisch
    float minHeightCm = 60.0f;
    float maxHeightCm = 125.0f;

    // Touch (Einbaulage)
    bool touchInvertX = false;
    bool touchInvertY = false;
};

Settings &settings();
desk::PresetTable &presets();

// Werksvorgaben laden und mit dem NVS-Inhalt ueberschreiben.
void settingsBegin();
// Einstellungen und Speicherplaetze sichern.
void settingsSave();
// Auf die Werte aus config.h zuruecksetzen (und sichern).
void settingsResetToDefaults();

} // namespace core

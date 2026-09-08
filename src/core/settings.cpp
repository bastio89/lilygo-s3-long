#include "core/settings.h"

#include <Arduino.h>
#include <Preferences.h>
#include <string.h>

#include "config.h"

namespace core {
namespace {

constexpr const char *kNamespace = "smartdesk";
constexpr const char *kKeySettings = "cfg";
constexpr const char *kKeyPresets = "presets";
// Erhoehen, sobald sich das Speicherlayout aendert -- dann werden die alten
// Daten verworfen statt falsch interpretiert.
constexpr uint32_t kLayoutVersion = 1;
constexpr const char *kKeyVersion = "ver";

Settings g_settings;
desk::PresetTable g_presets;
Preferences g_prefs;
bool g_open = false;

void applyDefaults() {
    g_settings = Settings{};
    strlcpy(g_settings.wifiSsid, WIFI_SSID, sizeof(g_settings.wifiSsid));
    strlcpy(g_settings.wifiPassword, WIFI_PASSWORD, sizeof(g_settings.wifiPassword));
    strlcpy(g_settings.locationName, LOCATION_NAME, sizeof(g_settings.locationName));
    g_settings.latitude = LOCATION_LATITUDE;
    g_settings.longitude = LOCATION_LONGITUDE;
    strlcpy(g_settings.timezone, TIMEZONE_POSIX, sizeof(g_settings.timezone));
    strlcpy(g_settings.ntpServer, NTP_SERVER, sizeof(g_settings.ntpServer));
    g_settings.brightness = DISPLAY_BRIGHTNESS;
    g_settings.sleepAfterMs = DISPLAY_SLEEP_AFTER_MS;
    g_settings.minHeightCm = DESK_MIN_HEIGHT_CM;
    g_settings.maxHeightCm = DESK_MAX_HEIGHT_CM;
    g_settings.touchInvertX = false;
    g_settings.touchInvertY = false;

    g_presets.resetToDefaults();
    g_presets.at(0).mode = desk::PresetMode::TargetHeight;
    g_presets.at(0).heightCm = DESK_SIT_HEIGHT_CM;
    strlcpy(g_presets.at(0).name, "Sitzen", desk::kPresetNameLen);
    g_presets.at(1).mode = desk::PresetMode::TargetHeight;
    g_presets.at(1).heightCm = DESK_STAND_HEIGHT_CM;
    strlcpy(g_presets.at(1).name, "Stehen", desk::kPresetNameLen);
}

} // namespace

Settings &settings() { return g_settings; }

desk::PresetTable &presets() { return g_presets; }

void settingsBegin() {
    applyDefaults();

    if (!g_prefs.begin(kNamespace, false)) {
        log_w("NVS nicht verfuegbar, benutze Werksvorgaben");
        return;
    }
    g_open = true;

    if (g_prefs.getUInt(kKeyVersion, 0) != kLayoutVersion) {
        log_i("Neues Einstellungs-Layout, uebernehme Werksvorgaben");
        settingsSave();
        return;
    }

    Settings stored;
    if (g_prefs.getBytes(kKeySettings, &stored, sizeof(stored)) == sizeof(stored)) {
        g_settings = stored;
        // Sicherheitsnetz gegen unbrauchbare Werte aus alten Staenden.
        if (g_settings.maxHeightCm <= g_settings.minHeightCm) {
            g_settings.minHeightCm = DESK_MIN_HEIGHT_CM;
            g_settings.maxHeightCm = DESK_MAX_HEIGHT_CM;
        }
        if (g_settings.brightness < 10) {
            g_settings.brightness = 10;
        }
    }

    desk::PresetTable storedPresets;
    if (g_prefs.getBytes(kKeyPresets, &storedPresets, sizeof(storedPresets)) ==
        sizeof(storedPresets)) {
        g_presets = storedPresets;
    }
}

void settingsSave() {
    if (!g_open) {
        return;
    }
    g_prefs.putUInt(kKeyVersion, kLayoutVersion);
    g_prefs.putBytes(kKeySettings, &g_settings, sizeof(g_settings));
    g_prefs.putBytes(kKeyPresets, &g_presets, sizeof(g_presets));
}

void settingsResetToDefaults() {
    applyDefaults();
    settingsSave();
}

} // namespace core

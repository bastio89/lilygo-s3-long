#include "net/weather.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "config.h"

namespace weather {
namespace {

Data g_data;
uint32_t g_nextFetchMs = 0;
bool g_pending = true;

constexpr uint32_t kRetryMs = 60000;

bool fetch() {
    if (WiFi.status() != WL_CONNECTED) {
        return false;
    }

    char url[320];
    snprintf(url, sizeof(url),
             "https://api.open-meteo.com/v1/forecast"
             "?latitude=%.4f&longitude=%.4f"
             "&current=temperature_2m,relative_humidity_2m,apparent_temperature,"
             "weather_code,wind_speed_10m"
             "&daily=temperature_2m_max,temperature_2m_min,precipitation_probability_max"
             "&timezone=auto&forecast_days=1",
             static_cast<double>(LOCATION_LATITUDE), static_cast<double>(LOCATION_LONGITUDE));

    WiFiClientSecure client;
    client.setInsecure(); // oeffentliche, unkritische Daten
    client.setTimeout(8000);

    HTTPClient http;
    http.setConnectTimeout(8000);
    if (!http.begin(client, url)) {
        return false;
    }
    const int status = http.GET();
    if (status != HTTP_CODE_OK) {
        log_w("Wetter-Abruf fehlgeschlagen: HTTP %d", status);
        http.end();
        return false;
    }

    JsonDocument doc;
    JsonDocument filter;
    filter["current"] = true;
    filter["daily"] = true;
    const DeserializationError err = deserializeJson(doc, http.getStream(),
                                                     DeserializationOption::Filter(filter));
    http.end();
    if (err) {
        log_w("Wetter-JSON fehlerhaft: %s", err.c_str());
        return false;
    }

    JsonObject current = doc["current"];
    JsonObject daily = doc["daily"];
    if (current.isNull()) {
        return false;
    }

    g_data.temperature = current["temperature_2m"] | 0.0f;
    g_data.apparent = current["apparent_temperature"] | g_data.temperature;
    g_data.humidity = current["relative_humidity_2m"] | 0;
    g_data.windKmh = current["wind_speed_10m"] | 0.0f;
    g_data.code = current["weather_code"] | 0;
    if (!daily.isNull()) {
        g_data.todayMax = daily["temperature_2m_max"][0] | g_data.temperature;
        g_data.todayMin = daily["temperature_2m_min"][0] | g_data.temperature;
        g_data.precipitationProb = daily["precipitation_probability_max"][0] | 0;
    }
    g_data.valid = true;
    g_data.updatedMs = millis();
    return true;
}

} // namespace

void begin() {
    g_nextFetchMs = 0;
    g_pending = true;
}

void requestRefresh() { g_pending = true; }

void loop(uint32_t nowMs) {
    if (!g_pending && static_cast<int32_t>(nowMs - g_nextFetchMs) < 0) {
        return;
    }
    if (WiFi.status() != WL_CONNECTED) {
        g_nextFetchMs = nowMs + kRetryMs;
        return;
    }
    g_pending = false;
    g_nextFetchMs = nowMs + (fetch() ? WEATHER_REFRESH_MS : kRetryMs);
}

const Data &data() { return g_data; }

const char *description(int code) {
    switch (code) {
    case 0: return "Klar";
    case 1: return "Ueberwiegend klar";
    case 2: return "Teils bewoelkt";
    case 3: return "Bedeckt";
    case 45:
    case 48: return "Nebel";
    case 51: return "Leichter Niesel";
    case 53: return "Niesel";
    case 55: return "Starker Niesel";
    case 56:
    case 57: return "Gefrierender Niesel";
    case 61: return "Leichter Regen";
    case 63: return "Regen";
    case 65: return "Starker Regen";
    case 66:
    case 67: return "Gefrierender Regen";
    case 71: return "Leichter Schnee";
    case 73: return "Schnee";
    case 75: return "Starker Schnee";
    case 77: return "Schneegriesel";
    case 80: return "Leichte Schauer";
    case 81: return "Schauer";
    case 82: return "Starke Schauer";
    case 85:
    case 86: return "Schneeschauer";
    case 95: return "Gewitter";
    case 96:
    case 99: return "Gewitter mit Hagel";
    default: return "-";
    }
}

uint32_t accentColor(int code) {
    if (code <= 1) {
        return 0xFFC64Bu; // Sonne
    }
    if (code == 2) {
        return 0xE8D7A0u; // heiter
    }
    if (code == 3 || code == 45 || code == 48) {
        return 0x9AA5B1u; // bedeckt / Nebel
    }
    if (code >= 71 && code <= 77) {
        return 0xCFE8FFu; // Schnee
    }
    if (code == 85 || code == 86) {
        return 0xCFE8FFu;
    }
    if (code >= 95) {
        return 0xC77DFFu; // Gewitter
    }
    if (code >= 51) {
        return 0x5AA9E6u; // Regen
    }
    return 0x9AA5B1u;
}

} // namespace weather

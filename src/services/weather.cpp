#include "services/weather.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include "config.h"
#include "core/network.h"
#include "core/settings.h"

namespace services {
namespace {

constexpr uint32_t kRetryMs = 60000;
constexpr uint16_t kHttpTimeoutMs = 8000;
WeatherService g_weather;

} // namespace

WeatherService &weather() { return g_weather; }

void WeatherService::begin() {
    nextFetchMs_ = 0;
    pending_ = true;
    failures_ = 0;
}

void WeatherService::refresh() { pending_ = true; }

const char *WeatherService::statusText() const {
    if (data_.valid) {
        return failures_ > 0 ? "veraltet" : "ok";
    }
    return core::online() ? "kein Abruf" : "offline";
}

void WeatherService::loop(uint32_t nowMs) {
    if (!pending_ && static_cast<int32_t>(nowMs - nextFetchMs_) < 0) {
        return;
    }
    if (!core::online()) {
        nextFetchMs_ = nowMs + kRetryMs;
        return;
    }
    pending_ = false;
    if (fetch()) {
        failures_ = 0;
        nextFetchMs_ = nowMs + WEATHER_REFRESH_MS;
    } else {
        if (failures_ < 255) {
            ++failures_;
        }
        nextFetchMs_ = nowMs + kRetryMs;
    }
}

bool WeatherService::fetch() {
    const core::Settings &cfg = core::settings();

    char url[384];
    snprintf(url, sizeof(url),
             "https://api.open-meteo.com/v1/forecast"
             "?latitude=%.4f&longitude=%.4f"
             "&current=temperature_2m,relative_humidity_2m,apparent_temperature,"
             "weather_code,wind_speed_10m"
             "&daily=weather_code,temperature_2m_max,temperature_2m_min,"
             "precipitation_probability_max"
             "&timezone=auto&forecast_days=%u",
             static_cast<double>(cfg.latitude), static_cast<double>(cfg.longitude),
             static_cast<unsigned>(kWeatherForecastDays + 1));

    log_i("Wetter-Abruf gestartet");

    WiFiClientSecure client;
    client.setInsecure(); // oeffentliche, unkritische Daten
    client.setHandshakeTimeout((kHttpTimeoutMs + 999) / 1000);
    client.setTimeout((kHttpTimeoutMs + 999) / 1000);

    HTTPClient http;
    http.setConnectTimeout(kHttpTimeoutMs);
    http.setTimeout(kHttpTimeoutMs);
    if (!http.begin(client, url)) {
        log_w("Wetter-Abruf konnte nicht initialisiert werden");
        return false;
    }
    const int status = http.GET();
    if (status != HTTP_CODE_OK) {
        log_w("Wetter-Abruf fehlgeschlagen: HTTP %d (%s)", status,
              HTTPClient::errorToString(status).c_str());
        http.end();
        return false;
    }

    const String payload = http.getString();
    http.end();
    if (payload.length() == 0) {
        log_w("Wetter-Antwort ist leer");
        return false;
    }

    JsonDocument doc;
    const DeserializationError err = deserializeJson(doc, payload);
    if (err) {
        log_w("Wetter-JSON fehlerhaft: %s", err.c_str());
        return false;
    }

    JsonObject current = doc["current"];
    if (current.isNull()) {
        const char *reason = doc["reason"] | "Feld current fehlt";
        log_w("Wetterdaten unvollstaendig: %s", reason);
        return false;
    }
    JsonObject daily = doc["daily"];

    data_.temperature = current["temperature_2m"] | 0.0f;
    data_.apparent = current["apparent_temperature"] | data_.temperature;
    data_.humidity = current["relative_humidity_2m"] | 0;
    data_.windKmh = current["wind_speed_10m"] | 0.0f;
    data_.code = current["weather_code"] | 0;
    for (WeatherForecastDay &forecast : data_.forecast) {
        forecast = WeatherForecastDay{};
    }
    uint8_t forecastDays = 0;
    if (!daily.isNull()) {
        data_.todayMax = daily["temperature_2m_max"][0] | data_.temperature;
        data_.todayMin = daily["temperature_2m_min"][0] | data_.temperature;
        data_.precipitationProb = daily["precipitation_probability_max"][0] | 0;
        for (uint8_t index = 0; index < kWeatherForecastDays; ++index) {
            const uint8_t sourceIndex = index + 1;
            WeatherForecastDay &forecast = data_.forecast[index];
            const char *date = daily["time"][sourceIndex] | "";
            if (date[0] == '\0') {
                continue;
            }
            snprintf(forecast.date, sizeof(forecast.date), "%s", date);
            forecast.low = daily["temperature_2m_min"][sourceIndex] | data_.temperature;
            forecast.high = daily["temperature_2m_max"][sourceIndex] | data_.temperature;
            forecast.code = daily["weather_code"][sourceIndex] | 0;
            forecast.precipitationProb =
                daily["precipitation_probability_max"][sourceIndex] | 0;
            forecast.valid = true;
            ++forecastDays;
        }
    }
    data_.valid = true;
    data_.updatedMs = millis();
    log_i("Wetter-Abruf erfolgreich: %u Prognosetage", static_cast<unsigned>(forecastDays));
    return true;
}

const char *weatherDescription(int code) {
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

uint32_t weatherAccentColor(int code) {
    if (code <= 1) {
        return 0xFFC64Bu; // Sonne
    }
    if (code == 2) {
        return 0xE8D7A0u; // heiter
    }
    if (code == 3 || code == 45 || code == 48) {
        return 0x9AA5B1u; // bedeckt / Nebel
    }
    if ((code >= 71 && code <= 77) || code == 85 || code == 86) {
        return 0xCFE8FFu; // Schnee
    }
    if (code >= 95) {
        return 0xC77DFFu; // Gewitter
    }
    if (code >= 51) {
        return 0x5AA9E6u; // Regen
    }
    return 0x9AA5B1u;
}

} // namespace services

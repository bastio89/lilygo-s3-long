#include "services/commute.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include "config.h"
#include "core/network.h"

namespace services {
namespace {

constexpr uint32_t kRetryMs = 60000;
constexpr uint16_t kHttpTimeoutMs = 8000;
CommuteService g_commute;

bool validCoordinate(float latitude, float longitude) {
    return latitude >= -90.0f && latitude <= 90.0f && longitude >= -180.0f &&
           longitude <= 180.0f && (latitude != 0.0f || longitude != 0.0f);
}

bool coordinatesConfigured() {
    return validCoordinate(COMMUTE_ORIGIN_LATITUDE, COMMUTE_ORIGIN_LONGITUDE) &&
           validCoordinate(COMMUTE_DESTINATION_LATITUDE, COMMUTE_DESTINATION_LONGITUDE);
}

} // namespace

CommuteService &commute() { return g_commute; }

void CommuteService::begin() {
    nextFetchMs_ = 0;
    pending_ = true;
    failures_ = 0;
}

void CommuteService::refresh() { pending_ = true; }

void CommuteService::setDirection(CommuteDirection direction) {
    if (direction_ == direction) {
        return;
    }
    direction_ = direction;
    data_.valid = false;
    pending_ = true;
}

bool CommuteService::configured() const {
    return MAPBOX_ACCESS_TOKEN[0] != '\0' && coordinatesConfigured();
}

const char *CommuteService::originName() const { return COMMUTE_ORIGIN_NAME; }

const char *CommuteService::destinationName() const { return COMMUTE_DESTINATION_NAME; }

const char *CommuteService::statusText() const {
    if (MAPBOX_ACCESS_TOKEN[0] == '\0') {
        return "Mapbox-Token fehlt";
    }
    if (!configured()) {
        return "Route nicht konfiguriert";
    }
    if (data_.valid) {
        return failures_ > 0 ? "veraltet" : "ok";
    }
    if (!core::online()) {
        return "offline";
    }
    return failures_ > 0 ? "Abruf fehlgeschlagen" : "wird abgerufen";
}

void CommuteService::loop(uint32_t nowMs) {
    if (!configured()) {
        pending_ = false;
        return;
    }
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
        nextFetchMs_ = nowMs + COMMUTE_REFRESH_MS;
    } else {
        if (failures_ < 255) {
            ++failures_;
        }
        nextFetchMs_ = nowMs + kRetryMs;
    }
}

bool CommuteService::fetch() {
    const bool toOffice = direction_ == CommuteDirection::ToOffice;
    const float startLatitude = toOffice ? COMMUTE_ORIGIN_LATITUDE
                                         : COMMUTE_DESTINATION_LATITUDE;
    const float startLongitude = toOffice ? COMMUTE_ORIGIN_LONGITUDE
                                          : COMMUTE_DESTINATION_LONGITUDE;
    const float endLatitude = toOffice ? COMMUTE_DESTINATION_LATITUDE
                                       : COMMUTE_ORIGIN_LATITUDE;
    const float endLongitude = toOffice ? COMMUTE_DESTINATION_LONGITUDE
                                        : COMMUTE_ORIGIN_LONGITUDE;

        char url[512];
    snprintf(url, sizeof(url),
             "https://api.mapbox.com/directions/v5/mapbox/driving-traffic/"
             "%.6f,%.6f;%.6f,%.6f"
             "?overview=false&alternatives=false&steps=false&depart_at=now"
             "&access_token=%s",
             static_cast<double>(startLongitude), static_cast<double>(startLatitude),
             static_cast<double>(endLongitude), static_cast<double>(endLatitude),
             MAPBOX_ACCESS_TOKEN);

        log_i("Pendelzeit-Abruf gestartet (%s, Live-Verkehr)",
            toOffice ? "Buero" : "nach Hause");

    WiFiClientSecure client;
    client.setInsecure(); // Der oeffentliche Routing-Endpunkt benoetigt keinen Login.
    client.setHandshakeTimeout((kHttpTimeoutMs + 999) / 1000);
    client.setTimeout((kHttpTimeoutMs + 999) / 1000);

    HTTPClient http;
    http.setConnectTimeout(kHttpTimeoutMs);
    http.setTimeout(kHttpTimeoutMs);
    if (!http.begin(client, url)) {
        log_w("Pendelzeit-Abruf konnte nicht initialisiert werden");
        return false;
    }
    const int status = http.GET();
    if (status != HTTP_CODE_OK) {
        log_w("Pendelzeit-Abruf fehlgeschlagen: HTTP %d (%s)", status,
              HTTPClient::errorToString(status).c_str());
        http.end();
        return false;
    }

    const String payload = http.getString();
    http.end();
    if (payload.length() == 0) {
        log_w("Pendelzeit-Antwort ist leer");
        return false;
    }

    JsonDocument doc;
    const DeserializationError err = deserializeJson(doc, payload);
    if (err) {
        log_w("Pendelzeit-JSON fehlerhaft: %s", err.c_str());
        return false;
    }

    const char *code = doc["code"] | "";
    JsonObject route = doc["routes"][0];
    const float duration = route["duration"] | 0.0f;
    const float typicalDuration = route["duration_typical"] | 0.0f;
    const float distance = route["distance"] | 0.0f;
    if (strcmp(code, "Ok") != 0 || route.isNull() || duration <= 0.0f || distance <= 0.0f) {
        log_w("Pendelroute unvollstaendig: %s", code[0] != '\0' ? code : "unbekannt");
        return false;
    }

    data_.durationSeconds = static_cast<uint32_t>(duration + 0.5f);
    data_.distanceMeters = static_cast<uint32_t>(distance + 0.5f);
    data_.trafficKnown = typicalDuration > 0.0f;
    data_.trafficDelaySeconds = data_.trafficKnown && duration > typicalDuration
                                    ? static_cast<uint32_t>(duration - typicalDuration + 0.5f)
                                    : 0;
    data_.updatedMs = millis();
    data_.valid = true;
        log_i("Pendelzeit erfolgreich: %lu min, %.1f km, Verkehr +%lu min",
          static_cast<unsigned long>((data_.durationSeconds + 59) / 60),
            static_cast<double>(data_.distanceMeters / 1000.0f),
            static_cast<unsigned long>((data_.trafficDelaySeconds + 59) / 60));
    return true;
}

} // namespace services
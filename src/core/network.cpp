#include "core/network.h"

#include <Arduino.h>
#include <WiFi.h>

#include "core/settings.h"

namespace core {
namespace {

constexpr uint32_t kRetryIntervalMs = 15000;

WifiState g_state = WifiState::Disabled;
uint32_t g_lastAttemptMs = 0;
char g_ip[16] = "-";

void startConnect(uint32_t nowMs) {
    g_lastAttemptMs = nowMs;
    g_state = WifiState::Connecting;
    WiFi.disconnect();
    WiFi.begin(settings().wifiSsid, settings().wifiPassword);
}

} // namespace

void networkBegin() {
    if (settings().wifiSsid[0] == '\0') {
        g_state = WifiState::Disabled;
        log_w("Kein WLAN konfiguriert (include/secrets.h)");
        return;
    }
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.setSleep(true);
    startConnect(millis());
}

void networkLoop(uint32_t nowMs) {
    if (g_state == WifiState::Disabled) {
        return;
    }

    if (WiFi.status() == WL_CONNECTED) {
        if (g_state != WifiState::Connected) {
            g_state = WifiState::Connected;
            strlcpy(g_ip, WiFi.localIP().toString().c_str(), sizeof(g_ip));
            log_i("WLAN verbunden: %s", g_ip);
        }
        return;
    }

    if (g_state == WifiState::Connected) {
        g_state = WifiState::Connecting;
        g_lastAttemptMs = nowMs;
        strlcpy(g_ip, "-", sizeof(g_ip));
    }
    if (static_cast<uint32_t>(nowMs - g_lastAttemptMs) > kRetryIntervalMs) {
        startConnect(nowMs);
    }
}

WifiState wifiState() { return g_state; }

bool online() { return g_state == WifiState::Connected; }

const char *wifiStateText() {
    switch (g_state) {
    case WifiState::Connected: return "verbunden";
    case WifiState::Connecting: return "verbinde...";
    default: return "aus";
    }
}

const char *ssid() { return settings().wifiSsid; }

const char *ipAddress() { return g_ip; }

int rssi() { return online() ? WiFi.RSSI() : 0; }

} // namespace core

#include "net/net.h"

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

#include "config.h"

namespace net {
namespace {

WifiState g_state = WifiState::Disabled;
uint32_t g_lastAttemptMs = 0;
bool g_timeConfigured = false;
char g_ip[16] = "-";
char g_clock[8] = "--:--";
char g_date[24] = "";

constexpr uint32_t kRetryIntervalMs = 15000;

const char *const kWeekdays[7] = {"So", "Mo", "Di", "Mi", "Do", "Fr", "Sa"};
const char *const kMonths[12] = {"Jan", "Feb", "Mrz", "Apr", "Mai", "Jun",
                                 "Jul", "Aug", "Sep", "Okt", "Nov", "Dez"};

void startConnect(uint32_t nowMs) {
    g_lastAttemptMs = nowMs;
    g_state = WifiState::Connecting;
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

} // namespace

void begin() {
    if (strlen(WIFI_SSID) == 0) {
        g_state = WifiState::Disabled;
        return;
    }
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.setSleep(true);
    startConnect(millis());
}

void loop(uint32_t nowMs) {
    if (g_state == WifiState::Disabled) {
        return;
    }

    if (WiFi.status() == WL_CONNECTED) {
        if (g_state != WifiState::Connected) {
            g_state = WifiState::Connected;
            strlcpy(g_ip, WiFi.localIP().toString().c_str(), sizeof(g_ip));
        }
        if (!g_timeConfigured) {
            configTzTime(TIMEZONE_POSIX, NTP_SERVER, "pool.ntp.org");
            g_timeConfigured = true;
        }
    } else {
        if (g_state == WifiState::Connected) {
            g_state = WifiState::Connecting;
            g_lastAttemptMs = nowMs;
            strlcpy(g_ip, "-", sizeof(g_ip));
        }
        if (static_cast<uint32_t>(nowMs - g_lastAttemptMs) > kRetryIntervalMs) {
            startConnect(nowMs);
        }
    }

    struct tm tm_now;
    if (getLocalTime(&tm_now, 0)) {
        snprintf(g_clock, sizeof(g_clock), "%02d:%02d", tm_now.tm_hour, tm_now.tm_min);
        snprintf(g_date, sizeof(g_date), "%s, %d. %s", kWeekdays[tm_now.tm_wday % 7],
                 tm_now.tm_mday, kMonths[tm_now.tm_mon % 12]);
    }
}

WifiState wifiState() { return g_state; }

const char *wifiStateText() {
    switch (g_state) {
    case WifiState::Connected: return "verbunden";
    case WifiState::Connecting: return "verbinde...";
    case WifiState::Failed: return "Fehler";
    default: return "aus";
    }
}

const char *ipAddress() { return g_ip; }

int rssi() { return WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0; }

bool timeSynced() { return g_date[0] != '\0'; }

const char *clockText() { return g_clock; }

const char *dateText() { return g_date; }

} // namespace net

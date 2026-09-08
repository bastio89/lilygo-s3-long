#include "core/clock.h"

#include <Arduino.h>
#include <time.h>

#include "core/network.h"
#include "core/settings.h"

namespace core {
namespace {

const char *const kWeekdays[7] = {"So", "Mo", "Di", "Mi", "Do", "Fr", "Sa"};
const char *const kMonths[12] = {"Jan", "Feb", "Mrz", "Apr", "Mai", "Jun",
                                 "Jul", "Aug", "Sep", "Okt", "Nov", "Dez"};

bool g_configured = false;
bool g_synced = false;
char g_clock[8] = "--:--";
char g_date[24] = "";

// Vor der ersten NTP-Antwort steht die Systemzeit auf 1970.
constexpr time_t kPlausibleEpoch = 1700000000; // 2023-11-14

} // namespace

void clockBegin() {
    g_configured = false;
    g_synced = false;
}

void clockLoop(uint32_t) {
    if (!g_configured) {
        if (!online()) {
            return;
        }
        configTzTime(settings().timezone, settings().ntpServer, "pool.ntp.org");
        g_configured = true;
    }

    struct tm now;
    if (!getLocalTime(&now, 0)) {
        return;
    }
    if (mktime(&now) < kPlausibleEpoch) {
        return;
    }
    g_synced = true;
    snprintf(g_clock, sizeof(g_clock), "%02d:%02d", now.tm_hour, now.tm_min);
    snprintf(g_date, sizeof(g_date), "%s, %d. %s", kWeekdays[now.tm_wday % 7], now.tm_mday,
             kMonths[now.tm_mon % 12]);
}

bool clockSynced() { return g_synced; }

const char *clockText() { return g_clock; }

const char *dateText() { return g_date; }

} // namespace core

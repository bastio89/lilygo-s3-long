// WLAN + Zeitsynchronisation.
#pragma once

#include <stdint.h>

namespace net {

enum class WifiState : uint8_t { Disabled, Connecting, Connected, Failed };

void begin();
void loop(uint32_t nowMs);

WifiState wifiState();
const char *wifiStateText();
const char *ipAddress();
int rssi();

bool timeSynced();
// "13:45"
const char *clockText();
// "Mo, 8. Sep"
const char *dateText();

} // namespace net

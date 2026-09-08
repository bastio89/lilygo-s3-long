// WLAN-Verbindung inklusive automatischer Wiederherstellung.
#pragma once

#include <stdint.h>

namespace core {

enum class WifiState : uint8_t { Disabled, Connecting, Connected };

void networkBegin();
void networkLoop(uint32_t nowMs);

WifiState wifiState();
bool online();
const char *wifiStateText();
const char *ssid();
const char *ipAddress();
int rssi();

} // namespace core

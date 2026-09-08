// AXS15231B (QSPI) + LVGL-Anbindung fuer das T-Display-S3 Long.
#pragma once

#include <stdint.h>

namespace board {

// Logische Aufloesung der UI (Querformat).
constexpr int kScreenWidth = 640;
constexpr int kScreenHeight = 180;

// Initialisiert Panel, Backlight-PWM und LVGL. Muss vor jeder LVGL-Nutzung
// aufgerufen werden. Gibt false zurueck, wenn die Zeichenpuffer nicht
// allokiert werden konnten.
bool displayBegin();

// LVGL-Timer bedienen; regelmaessig aus loop() aufrufen.
void displayLoop();

// Helligkeit 0..255 (0 = aus). Wird sanft angefahren.
void setBrightness(uint8_t value);
uint8_t brightness();

// Display schlafen legen / aufwecken (Backlight + Panel).
void displaySleep();
void displayWake();
bool displayAsleep();

} // namespace board

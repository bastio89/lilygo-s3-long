// LVGL-Oberflaeche: Statuszeile + drei wischbare Seiten
// (Schreibtisch / Wetter / System).
#pragma once

#include <stdint.h>

#include "desk/desk_controller.h"

namespace ui {

void begin(desk::Controller &controller);
// Aktualisiert die Anzeigewerte; regelmaessig aus loop() aufrufen.
void tick(uint32_t nowMs);

// true, wenn Zielhoehen geregelt angefahren werden statt die Speicherplaetze
// der Steuerbox zu benutzen.
bool useTargetHeights();

void showToast(const char *text);

} // namespace ui

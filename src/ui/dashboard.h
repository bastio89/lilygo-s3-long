// Rahmen der Oberflaeche: Statuszeile, wischbare Seiten, Kurzmeldungen.
#pragma once

#include <stdint.h>

#include "desk/flexispot.h"

namespace ui {
namespace dashboard {

void begin(desk::FlexiSpot &desk);
void tick(uint32_t nowMs);

// Kurzmeldung am unteren Rand.
void toast(const char *text);

// Auf eine bestimmte Seite springen (0 = Schreibtisch).
void showPage(uint8_t index);

} // namespace dashboard
} // namespace ui

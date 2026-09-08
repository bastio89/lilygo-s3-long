// Seite "Einstellungen": Anzeige, Fahrbereich, Speicherplaetze, Diagnose.
#pragma once

#include <stdint.h>

#include <lvgl.h>

#include "desk/flexispot.h"

namespace ui {
namespace settings_screen {

void create(lv_obj_t *parent, desk::FlexiSpot &desk);
void tick(uint32_t nowMs);

} // namespace settings_screen
} // namespace ui

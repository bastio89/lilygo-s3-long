// Seite "Schreibtisch": Hoehe, Fahrtasten, Speicherplaetze.
#pragma once

#include <stdint.h>

#include <lvgl.h>

#include "desk/flexispot.h"

namespace ui {
namespace desk_screen {

void create(lv_obj_t *parent, desk::FlexiSpot &desk);
void tick(uint32_t nowMs);

} // namespace desk_screen
} // namespace ui

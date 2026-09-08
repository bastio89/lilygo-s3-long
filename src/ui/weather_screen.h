// Seite "Wetter".
#pragma once

#include <stdint.h>

#include <lvgl.h>

namespace ui {
namespace weather_screen {

void create(lv_obj_t *parent);
void tick(uint32_t nowMs);

} // namespace weather_screen
} // namespace ui

// Seite "Pendelzeit".
#pragma once

#include <lvgl.h>

namespace ui {
namespace commute_screen {

void create(lv_obj_t *parent);
void tick(uint32_t nowMs);

} // namespace commute_screen
} // namespace ui
// Seite "Drucker".
#pragma once

#include <stdint.h>

#include <lvgl.h>

namespace ui {
namespace printer_screen {

void create(lv_obj_t *parent);
void tick(uint32_t nowMs);

} // namespace printer_screen
} // namespace ui
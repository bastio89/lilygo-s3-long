// Gemeinsame Farben, Masse und Bausteine der Oberflaeche.
#pragma once

#include <lvgl.h>

namespace ui {

// Logische Aufloesung (Querformat, siehe board/display.cpp).
constexpr int kScreenWidth = 640;
constexpr int kScreenHeight = 180;
constexpr int kStatusHeight = 26;
constexpr int kPageHeight = kScreenHeight - kStatusHeight;

namespace theme {

lv_color_t bg();
lv_color_t panel();
lv_color_t panelLight();
lv_color_t accent();
lv_color_t text();
lv_color_t muted();
lv_color_t warn();
lv_color_t good();

// Karten-Optik fuer einen Container.
void styleCard(lv_obj_t *obj);
// Container ohne Rahmen, Polsterung und Scrollbalken.
lv_obj_t *plainBox(lv_obj_t *parent);

lv_obj_t *label(lv_obj_t *parent, const lv_font_t *font, lv_color_t color, const char *text);
lv_obj_t *button(lv_obj_t *parent, const char *caption, const lv_font_t *font, lv_coord_t x,
                 lv_coord_t y, lv_coord_t w, lv_coord_t h);
// Das Textlabel eines mit button() erzeugten Knopfes.
lv_obj_t *buttonLabel(lv_obj_t *btn);

} // namespace theme
} // namespace ui

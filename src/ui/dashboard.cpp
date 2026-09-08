#include "ui/dashboard.h"

#include <Arduino.h>

#include "core/clock.h"
#include "core/network.h"
#include "ui/desk_screen.h"
#include "ui/settings_screen.h"
#include "ui/theme.h"
#include "ui/weather_screen.h"

namespace ui {
namespace dashboard {
namespace {

constexpr uint32_t kToastMs = 2500;

lv_obj_t *g_tiles = nullptr;
lv_obj_t *g_tileObjects[3] = {};
lv_obj_t *g_clockLabel = nullptr;
lv_obj_t *g_dateLabel = nullptr;
lv_obj_t *g_titleLabel = nullptr;
lv_obj_t *g_wifiLabel = nullptr;
lv_obj_t *g_toast = nullptr;
uint32_t g_toastUntilMs = 0;

const char *const kPageTitles[3] = {"Schreibtisch", "Wetter", "Einstellungen"};

void buildStatusBar(lv_obj_t *parent) {
    lv_obj_t *bar = theme::plainBox(parent);
    lv_obj_set_pos(bar, 0, 0);
    lv_obj_set_size(bar, kScreenWidth, kStatusHeight);

    g_clockLabel = theme::label(bar, &lv_font_montserrat_16, theme::text(), "--:--");
    lv_obj_align(g_clockLabel, LV_ALIGN_LEFT_MID, 10, 0);

    g_dateLabel = theme::label(bar, &lv_font_montserrat_14, theme::muted(), "");
    lv_obj_align(g_dateLabel, LV_ALIGN_LEFT_MID, 72, 0);

    g_titleLabel = theme::label(bar, &lv_font_montserrat_14, theme::muted(), kPageTitles[0]);
    lv_obj_align(g_titleLabel, LV_ALIGN_CENTER, 0, 0);

    g_wifiLabel = theme::label(bar, &lv_font_montserrat_14, theme::muted(), LV_SYMBOL_WIFI);
    lv_obj_align(g_wifiLabel, LV_ALIGN_RIGHT_MID, -10, 0);
}

uint8_t currentPage() {
    if (g_tiles == nullptr) {
        return 0;
    }
    lv_obj_t *active = lv_tileview_get_tile_act(g_tiles);
    for (uint8_t i = 0; i < 3; ++i) {
        if (g_tileObjects[i] == active) {
            return i;
        }
    }
    return 0;
}

} // namespace

void begin(desk::FlexiSpot &desk) {
    lv_obj_t *screen = lv_scr_act();
    lv_obj_set_style_bg_color(screen, theme::bg(), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    buildStatusBar(screen);

    g_tiles = lv_tileview_create(screen);
    lv_obj_set_pos(g_tiles, 0, kStatusHeight);
    lv_obj_set_size(g_tiles, kScreenWidth, kPageHeight);
    lv_obj_set_style_bg_color(g_tiles, theme::bg(), 0);
    lv_obj_set_style_border_width(g_tiles, 0, 0);

    g_tileObjects[0] = lv_tileview_add_tile(g_tiles, 0, 0, LV_DIR_RIGHT);
    g_tileObjects[1] = lv_tileview_add_tile(g_tiles, 1, 0, LV_DIR_HOR);
    g_tileObjects[2] = lv_tileview_add_tile(g_tiles, 2, 0, LV_DIR_LEFT);

    desk_screen::create(g_tileObjects[0], desk);
    weather_screen::create(g_tileObjects[1]);
    settings_screen::create(g_tileObjects[2], desk);

    g_toast = lv_label_create(lv_layer_top());
    lv_obj_set_style_bg_color(g_toast, theme::accent(), 0);
    lv_obj_set_style_bg_opa(g_toast, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(g_toast, lv_color_black(), 0);
    lv_obj_set_style_text_font(g_toast, &lv_font_montserrat_16, 0);
    lv_obj_set_style_pad_all(g_toast, 8, 0);
    lv_obj_set_style_radius(g_toast, 8, 0);
    lv_label_set_text(g_toast, "");
    lv_obj_align(g_toast, LV_ALIGN_BOTTOM_MID, 0, -8);
    lv_obj_add_flag(g_toast, LV_OBJ_FLAG_HIDDEN);
}

void toast(const char *text) {
    if (g_toast == nullptr) {
        return;
    }
    lv_label_set_text(g_toast, text);
    lv_obj_clear_flag(g_toast, LV_OBJ_FLAG_HIDDEN);
    lv_obj_align(g_toast, LV_ALIGN_BOTTOM_MID, 0, -8);
    g_toastUntilMs = millis() + kToastMs;
}

void showPage(uint8_t index) {
    if (g_tiles != nullptr && index < 3) {
        lv_obj_set_tile(g_tiles, g_tileObjects[index], LV_ANIM_ON);
    }
}

void tick(uint32_t nowMs) {
    lv_label_set_text(g_clockLabel, core::clockText());
    lv_label_set_text(g_dateLabel, core::dateText());

    const uint8_t page = currentPage();
    lv_label_set_text(g_titleLabel, kPageTitles[page]);
    lv_obj_align(g_titleLabel, LV_ALIGN_CENTER, 0, 0);

    lv_obj_set_style_text_color(g_wifiLabel,
                                core::online() ? theme::accent() : theme::muted(), 0);

    // Nur die sichtbare Seite aktualisieren.
    switch (page) {
    case 0: desk_screen::tick(nowMs); break;
    case 1: weather_screen::tick(nowMs); break;
    case 2: settings_screen::tick(nowMs); break;
    default: break;
    }

    if (!lv_obj_has_flag(g_toast, LV_OBJ_FLAG_HIDDEN) &&
        static_cast<int32_t>(nowMs - g_toastUntilMs) >= 0) {
        lv_obj_add_flag(g_toast, LV_OBJ_FLAG_HIDDEN);
    }
}

} // namespace dashboard
} // namespace ui

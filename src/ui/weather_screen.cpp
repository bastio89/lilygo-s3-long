#include "ui/weather_screen.h"

#include <stdio.h>

#include "core/clock.h"
#include "core/settings.h"
#include "services/weather.h"
#include "ui/theme.h"

namespace ui {
namespace weather_screen {
namespace {

lv_obj_t *g_temp = nullptr;
lv_obj_t *g_dot = nullptr;
lv_obj_t *g_desc = nullptr;
lv_obj_t *g_details = nullptr;
lv_obj_t *g_place = nullptr;

} // namespace

void create(lv_obj_t *parent) {
    lv_obj_set_style_pad_all(parent, 6, 0);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *card = lv_obj_create(parent);
    theme::styleCard(card);
    lv_obj_set_pos(card, 0, 0);
    lv_obj_set_size(card, kScreenWidth - 24, kPageHeight - 12);

    g_temp = theme::label(card, &lv_font_montserrat_48, theme::text(), "--");
    lv_obj_align(g_temp, LV_ALIGN_TOP_LEFT, 4, 2);

    g_dot = lv_obj_create(card);
    lv_obj_set_size(g_dot, 18, 18);
    lv_obj_set_style_radius(g_dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(g_dot, 0, 0);
    lv_obj_set_style_bg_color(g_dot, theme::muted(), 0);
    lv_obj_align(g_dot, LV_ALIGN_TOP_LEFT, 190, 14);

    g_desc = theme::label(card, &lv_font_montserrat_24, theme::text(), "Keine Daten");
    lv_obj_align(g_desc, LV_ALIGN_TOP_LEFT, 218, 12);

    g_details = theme::label(card, &lv_font_montserrat_16, theme::muted(), "");
    lv_obj_align(g_details, LV_ALIGN_TOP_LEFT, 218, 48);

    g_place = theme::label(card, &lv_font_montserrat_14, theme::muted(), "");
    lv_obj_align(g_place, LV_ALIGN_BOTTOM_LEFT, 4, 0);
}

void tick(uint32_t) {
    const services::WeatherData &wx = services::weather().data();

    char place[96];
    snprintf(place, sizeof(place), "%s   %s", core::settings().locationName,
             services::weather().statusText());
    lv_label_set_text(g_place, place);

    if (!wx.valid) {
        return;
    }
    char temperature[24];
    snprintf(temperature, sizeof(temperature), "%.0f C", static_cast<double>(wx.temperature));
    lv_label_set_text(g_temp, temperature);
    lv_label_set_text(g_desc, services::weatherDescription(wx.code));
    lv_obj_set_style_bg_color(g_dot, lv_color_hex(services::weatherAccentColor(wx.code)), 0);
    char details[128];
    snprintf(details, sizeof(details),
             "min %.0f / max %.0f   gefuehlt %.0f\n"
             "%d %% Luftfeuchte   %.0f km/h Wind   %d %% Regen",
             static_cast<double>(wx.todayMin), static_cast<double>(wx.todayMax),
             static_cast<double>(wx.apparent), wx.humidity, static_cast<double>(wx.windKmh),
             wx.precipitationProb);
    lv_label_set_text(g_details, details);
}

} // namespace weather_screen
} // namespace ui

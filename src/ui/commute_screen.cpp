#include "ui/commute_screen.h"

#include <Arduino.h>
#include <stdio.h>
#include <string.h>

#include "services/commute.h"
#include "ui/dashboard.h"
#include "ui/theme.h"

namespace ui {
namespace commute_screen {
namespace {

lv_obj_t *g_direction = nullptr;
lv_obj_t *g_route = nullptr;
lv_obj_t *g_duration = nullptr;
lv_obj_t *g_distance = nullptr;
lv_obj_t *g_status = nullptr;
lv_obj_t *g_updated = nullptr;

void setLabelTextIfChanged(lv_obj_t *label, const char *text) {
    if (strcmp(lv_label_get_text(label), text) != 0) {
        lv_label_set_text(label, text);
    }
}

const char *directionText(services::CommuteDirection direction) {
    return direction == services::CommuteDirection::ToOffice ? "Ins Büro" : "Nach Hause";
}

void directionCb(lv_event_t *e) {
    const services::CommuteDirection direction =
        static_cast<services::CommuteDirection>(reinterpret_cast<uintptr_t>(
            lv_event_get_user_data(e)));
    services::commute().setDirection(direction);
}

void refreshCb(lv_event_t *) {
    services::commute().refresh();
    dashboard::toast("Pendelzeit wird aktualisiert");
}

} // namespace

void create(lv_obj_t *parent) {
    lv_obj_set_style_pad_all(parent, 6, 0);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *summary = lv_obj_create(parent);
    theme::styleCard(summary);
    lv_obj_set_pos(summary, 0, 0);
    lv_obj_set_size(summary, 286, kPageHeight - 12);

    g_direction = theme::label(summary, &lv_font_montserrat_20, theme::text(), "Ins Büro");
    lv_obj_set_pos(g_direction, 12, 6);

    g_duration = theme::label(summary, &lv_font_montserrat_48, theme::text(), "--");
    lv_obj_set_pos(g_duration, 12, 28);
    lv_obj_set_size(g_duration, 100, 56);
    lv_label_set_long_mode(g_duration, LV_LABEL_LONG_CLIP);

    g_updated = theme::label(summary, &lv_font_montserrat_16, theme::muted(), "");
    lv_obj_set_pos(g_updated, 116, 50);
    lv_obj_set_size(g_updated, 145, 22);
    lv_label_set_long_mode(g_updated, LV_LABEL_LONG_CLIP);

    lv_obj_t *minutes = theme::label(summary, &lv_font_montserrat_16, theme::muted(), "Minuten");
    lv_obj_set_pos(minutes, 12, 84);

    g_status = theme::label(summary, &lv_font_montserrat_14, theme::muted(), "Wird abgerufen");
    lv_obj_set_pos(g_status, 12, 106);
    lv_obj_set_size(g_status, 260, 20);
    lv_label_set_long_mode(g_status, LV_LABEL_LONG_CLIP);

    lv_obj_t *details = lv_obj_create(parent);
    theme::styleCard(details);
    lv_obj_set_pos(details, 294, 0);
    lv_obj_set_size(details, kScreenWidth - 300, kPageHeight - 12);

    g_route = theme::label(details, &lv_font_montserrat_16, theme::text(), "Start -> Büro");
    lv_obj_set_pos(g_route, 12, 6);
    lv_obj_set_size(g_route, 260, 22);
    lv_label_set_long_mode(g_route, LV_LABEL_LONG_CLIP);

    g_distance = theme::label(details, &lv_font_montserrat_24, theme::accent(), "-- km");
    lv_obj_set_pos(g_distance, 12, 38);

    lv_obj_t *toOffice = theme::button(details, "Ins Büro", &lv_font_montserrat_14, 12, 82, 142, 34);
    lv_obj_add_event_cb(toOffice, directionCb, LV_EVENT_CLICKED,
                        reinterpret_cast<void *>(static_cast<uintptr_t>(
                            services::CommuteDirection::ToOffice)));

    lv_obj_t *home = theme::button(details, "Nach Hause", &lv_font_montserrat_14, 162, 82, 142, 34);
    lv_obj_add_event_cb(home, directionCb, LV_EVENT_CLICKED,
                        reinterpret_cast<void *>(static_cast<uintptr_t>(
                            services::CommuteDirection::Home)));

    lv_obj_t *refresh = theme::button(details, LV_SYMBOL_REFRESH, &lv_font_montserrat_20, 282, 6, 34, 34);
    lv_obj_add_event_cb(refresh, refreshCb, LV_EVENT_CLICKED, nullptr);
}

void tick(uint32_t nowMs) {
    services::CommuteService &service = services::commute();
    const services::CommuteData &data = service.data();
    const services::CommuteDirection direction = service.direction();

    char directionLabel[32];
    snprintf(directionLabel, sizeof(directionLabel), "%s", directionText(direction));
    setLabelTextIfChanged(g_direction, directionLabel);

    char route[96];
    if (direction == services::CommuteDirection::ToOffice) {
        snprintf(route, sizeof(route), "%s -> %s", service.originName(), service.destinationName());
    } else {
        snprintf(route, sizeof(route), "%s -> %s", service.destinationName(), service.originName());
    }
    setLabelTextIfChanged(g_route, route);
    setLabelTextIfChanged(g_status, service.statusText());

    if (!data.valid) {
        setLabelTextIfChanged(g_duration, "--");
        setLabelTextIfChanged(g_distance, "-- km");
        setLabelTextIfChanged(g_updated, "Noch keine Route");
        return;
    }

    char duration[24];
    snprintf(duration, sizeof(duration), "%lu",
             static_cast<unsigned long>((data.durationSeconds + 59) / 60));
    setLabelTextIfChanged(g_duration, duration);

    char distance[24];
    snprintf(distance, sizeof(distance), "%.1f km",
             static_cast<double>(data.distanceMeters / 1000.0f));
    setLabelTextIfChanged(g_distance, distance);

    char updated[32];
    if (!data.trafficKnown) {
        snprintf(updated, sizeof(updated), "Verkehr unbekannt");
    } else if (data.trafficDelaySeconds >= 30) {
        snprintf(updated, sizeof(updated), "+%lu min Verkehr",
                 static_cast<unsigned long>((data.trafficDelaySeconds + 59) / 60));
    } else {
        snprintf(updated, sizeof(updated), "Verkehr normal");
    }
    setLabelTextIfChanged(g_updated, updated);
}

} // namespace commute_screen
} // namespace ui
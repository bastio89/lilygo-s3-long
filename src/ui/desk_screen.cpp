#include "ui/desk_screen.h"

#include <Arduino.h>
#include <stdio.h>

#include "core/settings.h"
#include "ui/dashboard.h"
#include "ui/theme.h"

namespace ui {
namespace desk_screen {
namespace {

desk::FlexiSpot *g_desk = nullptr;

lv_obj_t *g_heightLabel = nullptr;
lv_obj_t *g_statusLabel = nullptr;
lv_obj_t *g_presetBtns[desk::kPresetCount] = {};

void holdCb(lv_event_t *e) {
    const uint16_t key =
        static_cast<uint16_t>(reinterpret_cast<uintptr_t>(lv_event_get_user_data(e)));
    if (g_desk == nullptr) {
        return;
    }
    switch (lv_event_get_code(e)) {
    case LV_EVENT_PRESSED:
        g_desk->hold(key, millis());
        break;
    case LV_EVENT_RELEASED:
    case LV_EVENT_PRESS_LOST:
        g_desk->release(millis());
        break;
    default:
        break;
    }
}

void presetCb(lv_event_t *e) {
    const uint8_t index =
        static_cast<uint8_t>(reinterpret_cast<uintptr_t>(lv_event_get_user_data(e)));
    if (g_desk == nullptr) {
        return;
    }
    const uint32_t now = millis();

    if (lv_event_get_code(e) == LV_EVENT_LONG_PRESSED) {
        if (!core::presets().capture(index, *g_desk)) {
            dashboard::toast("Keine Hoehe bekannt");
            return;
        }
        core::settingsSave();
        char msg[48];
        snprintf(msg, sizeof(msg), "Platz %u = %.1f cm", static_cast<unsigned>(index + 1),
                 static_cast<double>(core::presets().at(index).heightCm));
        dashboard::toast(msg);
        return;
    }

    if (lv_event_get_code(e) == LV_EVENT_SHORT_CLICKED) {
        core::presets().apply(index, *g_desk, now);
    }
}

void stopCb(lv_event_t *) {
    if (g_desk != nullptr) {
        g_desk->stop(millis());
        dashboard::toast("Gestoppt");
    }
}

const char *moveResultText(desk::MoveResult result) {
    switch (result) {
    case desk::MoveResult::Reached: return "Ziel erreicht";
    case desk::MoveResult::Stalled: return "Blockiert - gestoppt";
    case desk::MoveResult::TimedOut: return "Zeitueberschreitung";
    case desk::MoveResult::NoFeedback: return "Keine Rueckmeldung";
    default: return "bereit";
    }
}

void addHoldHandlers(lv_obj_t *btn, uint16_t key) {
    void *arg = reinterpret_cast<void *>(static_cast<uintptr_t>(key));
    lv_obj_add_event_cb(btn, holdCb, LV_EVENT_PRESSED, arg);
    lv_obj_add_event_cb(btn, holdCb, LV_EVENT_RELEASED, arg);
    lv_obj_add_event_cb(btn, holdCb, LV_EVENT_PRESS_LOST, arg);
}

} // namespace

void create(lv_obj_t *parent, desk::FlexiSpot &desk) {
    g_desk = &desk;

    lv_obj_set_style_pad_all(parent, 6, 0);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *card = lv_obj_create(parent);
    theme::styleCard(card);
    lv_obj_set_pos(card, 0, 0);
    lv_obj_set_size(card, 250, kPageHeight - 12);

    g_heightLabel = theme::label(card, &lv_font_montserrat_48, theme::text(), "--.-");
    lv_obj_align(g_heightLabel, LV_ALIGN_TOP_LEFT, 0, 6);

    lv_obj_t *unit = theme::label(card, &lv_font_montserrat_20, theme::muted(), "cm");
    lv_obj_align_to(unit, g_heightLabel, LV_ALIGN_OUT_RIGHT_BOTTOM, 6, -6);

    g_statusLabel = theme::label(card, &lv_font_montserrat_16, theme::muted(), "Start...");
    lv_obj_align(g_statusLabel, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    lv_obj_t *up =
        theme::button(parent, LV_SYMBOL_UP, &lv_font_montserrat_24, 260, 0, 96, 66);
    addHoldHandlers(up, loctek::KEY_UP);

    lv_obj_t *down =
        theme::button(parent, LV_SYMBOL_DOWN, &lv_font_montserrat_24, 260, 74, 96, 66);
    addHoldHandlers(down, loctek::KEY_DOWN);

    const lv_coord_t px[2] = {366, 480};
    const lv_coord_t py[2] = {0, 74};
    for (uint8_t i = 0; i < desk::kPresetCount; ++i) {
        char caption[desk::kPresetNameLen];
        core::presets().label(i, caption, sizeof(caption));
        lv_obj_t *btn = theme::button(parent, caption, &lv_font_montserrat_20, px[i % 2],
                                      py[i / 2], 108, 66);
        void *arg = reinterpret_cast<void *>(static_cast<uintptr_t>(i));
        lv_obj_add_event_cb(btn, presetCb, LV_EVENT_SHORT_CLICKED, arg);
        lv_obj_add_event_cb(btn, presetCb, LV_EVENT_LONG_PRESSED, arg);
        g_presetBtns[i] = btn;
    }

    lv_obj_t *stop =
        theme::button(parent, LV_SYMBOL_STOP, &lv_font_montserrat_20, 594, 0, 34, 140);
    lv_obj_set_style_bg_color(stop, lv_color_hex(0x3A2126), 0);
    lv_obj_set_style_bg_color(stop, theme::warn(), LV_STATE_PRESSED);
    lv_obj_add_event_cb(stop, stopCb, LV_EVENT_CLICKED, nullptr);
}

void tick(uint32_t) {
    if (g_desk == nullptr) {
        return;
    }

    if (g_desk->heightKnown()) {
        char height[16];
        snprintf(height, sizeof(height), "%.1f", static_cast<double>(g_desk->heightCm()));
        lv_label_set_text(g_heightLabel, height);
    } else if (g_desk->displayText()[0] != '\0') {
        lv_label_set_text(g_heightLabel, g_desk->displayText());
    } else {
        lv_label_set_text(g_heightLabel, "--.-");
    }

    char status[64];
    if (g_desk->targetActive()) {
        snprintf(status, sizeof(status), "Ziel %.1f cm",
                 static_cast<double>(g_desk->targetCm()));
    } else if (g_desk->motion() == desk::Motion::Up) {
        snprintf(status, sizeof(status), "faehrt hoch");
    } else if (g_desk->motion() == desk::Motion::Down) {
        snprintf(status, sizeof(status), "faehrt runter");
    } else if (!g_desk->heightKnown()) {
        snprintf(status, sizeof(status), "Steuerbox meldet nichts");
    } else {
        snprintf(status, sizeof(status), "%s", moveResultText(g_desk->lastMoveResult()));
    }
    lv_label_set_text(g_statusLabel, status);

    for (uint8_t i = 0; i < desk::kPresetCount; ++i) {
        if (g_presetBtns[i] == nullptr) {
            continue;
        }
        char caption[desk::kPresetNameLen];
        core::presets().label(i, caption, sizeof(caption));
        lv_label_set_text(theme::buttonLabel(g_presetBtns[i]), caption);
    }
}

} // namespace desk_screen
} // namespace ui

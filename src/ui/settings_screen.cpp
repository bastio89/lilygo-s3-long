#include "ui/settings_screen.h"

#include <Arduino.h>
#include <stdio.h>
#include <string.h>

#include "board/display.h"
#include "board/touch.h"
#include "core/clock.h"
#include "core/network.h"
#include "core/settings.h"
#include "services/service.h"
#include "ui/dashboard.h"
#include "ui/theme.h"

namespace ui {
namespace settings_screen {
namespace {

constexpr int kRowHeight = 46;
constexpr int kRowWidth = kScreenWidth - 30;

desk::FlexiSpot *g_desk = nullptr;
lv_obj_t *g_diagnostics = nullptr;
lv_obj_t *g_presetHeightLabels[desk::kPresetCount] = {};
lv_obj_t *g_presetModeDrop[desk::kPresetCount] = {};

// --- Zahlenfeld mit -/+ ----------------------------------------------------

struct Stepper {
    lv_obj_t *value = nullptr;
    float *target = nullptr;
    float step = 0.5f;
    float lower = 0.0f;
    float upper = 250.0f;
    const char *suffix = "";
};

Stepper g_steppers[2 + desk::kPresetCount];
uint8_t g_stepperCount = 0;

void renderStepper(const Stepper &s) {
    if (s.value == nullptr || s.target == nullptr) {
        return;
    }
    char text[32];
    snprintf(text, sizeof(text), "%.1f%s", static_cast<double>(*s.target), s.suffix);
    if (strcmp(lv_label_get_text(s.value), text) != 0) {
        lv_label_set_text(s.value, text);
    }
}

void stepperChange(Stepper *s, float direction) {
    if (s == nullptr || s->target == nullptr) {
        return;
    }
    float next = *s->target + direction * s->step;
    if (next < s->lower) {
        next = s->lower;
    }
    if (next > s->upper) {
        next = s->upper;
    }
    *s->target = next;
    renderStepper(*s);
    core::settingsSave();
}

void stepperMinusCb(lv_event_t *e) {
    stepperChange(static_cast<Stepper *>(lv_event_get_user_data(e)), -1.0f);
}

void stepperPlusCb(lv_event_t *e) {
    stepperChange(static_cast<Stepper *>(lv_event_get_user_data(e)), +1.0f);
}

// Legt ein Stepper-Bedienfeld an und liefert das Wertlabel zurueck.
lv_obj_t *makeStepper(lv_obj_t *row, lv_coord_t rightOffset, float *target, float step,
                      float lower, float upper, const char *suffix) {
    if (g_stepperCount >= (sizeof(g_steppers) / sizeof(g_steppers[0]))) {
        return nullptr;
    }
    Stepper &s = g_steppers[g_stepperCount++];
    s.target = target;
    s.step = step;
    s.lower = lower;
    s.upper = upper;
    s.suffix = suffix;

    lv_obj_t *minus = theme::button(row, "-", &lv_font_montserrat_20, 0, 0, 34, 30);
    lv_obj_align(minus, LV_ALIGN_RIGHT_MID, -rightOffset - 108, 0);
    lv_obj_set_style_bg_color(minus, theme::panelLight(), 0);
    lv_obj_add_event_cb(minus, stepperMinusCb, LV_EVENT_CLICKED, &s);

    s.value = theme::label(row, &lv_font_montserrat_16, theme::text(), "");
    lv_obj_set_style_text_align(s.value, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(s.value, 66);
    lv_obj_align(s.value, LV_ALIGN_RIGHT_MID, -rightOffset - 38, 0);

    lv_obj_t *plus = theme::button(row, "+", &lv_font_montserrat_20, 0, 0, 34, 30);
    lv_obj_align(plus, LV_ALIGN_RIGHT_MID, -rightOffset, 0);
    lv_obj_set_style_bg_color(plus, theme::panelLight(), 0);
    lv_obj_add_event_cb(plus, stepperPlusCb, LV_EVENT_CLICKED, &s);

    renderStepper(s);
    return s.value;
}

// --- Zeilengeruest ---------------------------------------------------------

lv_obj_t *makeRow(lv_obj_t *list, const char *title) {
    lv_obj_t *row = lv_obj_create(list);
    theme::styleCard(row);
    lv_obj_set_size(row, kRowWidth, kRowHeight);
    lv_obj_set_style_pad_all(row, 6, 0);

    lv_obj_t *label = theme::label(row, &lv_font_montserrat_14, theme::muted(), title);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 2, 0);
    return row;
}

// --- Rueckrufe -------------------------------------------------------------

void brightnessCb(lv_event_t *e) {
    const uint8_t value = static_cast<uint8_t>(lv_slider_get_value(lv_event_get_target(e)));
    core::settings().brightness = value;
    board::setBrightness(value);
}

void brightnessReleasedCb(lv_event_t *) { core::settingsSave(); }

void sleepCb(lv_event_t *e) {
    static const uint32_t kValues[] = {0, 60000UL, 120000UL, 300000UL, 600000UL};
    const uint16_t sel = lv_dropdown_get_selected(lv_event_get_target(e));
    core::settings().sleepAfterMs = kValues[sel < 5 ? sel : 0];
    core::settingsSave();
}

void touchInvertCb(lv_event_t *e) {
    const bool isY = lv_event_get_user_data(e) != nullptr;
    const bool on = lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED);
    if (isY) {
        core::settings().touchInvertY = on;
    } else {
        core::settings().touchInvertX = on;
    }
    board::touchSetInvert(core::settings().touchInvertX, core::settings().touchInvertY);
    core::settingsSave();
}

void presetModeCb(lv_event_t *e) {
    const uint8_t index =
        static_cast<uint8_t>(reinterpret_cast<uintptr_t>(lv_event_get_user_data(e)));
    const uint16_t sel = lv_dropdown_get_selected(lv_event_get_target(e));
    desk::Preset &preset = core::presets().at(index);
    if (sel < 4) {
        preset.mode = desk::PresetMode::BoxMemory;
        preset.boxSlot = static_cast<uint8_t>(sel + 1);
    } else {
        preset.mode = desk::PresetMode::TargetHeight;
    }
    core::settingsSave();
}

void presetCaptureCb(lv_event_t *e) {
    const uint8_t index =
        static_cast<uint8_t>(reinterpret_cast<uintptr_t>(lv_event_get_user_data(e)));
    if (g_desk == nullptr) {
        return;
    }
    if (!core::presets().capture(index, *g_desk)) {
        dashboard::toast("Keine Höhe bekannt");
        return;
    }
    core::settingsSave();
    lv_dropdown_set_selected(g_presetModeDrop[index], 4);
    dashboard::toast("Übernommen");
}

void factoryResetCb(lv_event_t *) {
    core::settingsResetToDefaults();
    board::setBrightness(core::settings().brightness);
    board::touchSetInvert(core::settings().touchInvertX, core::settings().touchInvertY);
    dashboard::toast("Werkseinstellungen - bitte neu starten");
}

lv_obj_t *makeDropdown(lv_obj_t *row, const char *options, lv_coord_t width,
                       lv_coord_t rightOffset) {
    lv_obj_t *drop = lv_dropdown_create(row);
    lv_dropdown_set_options_static(drop, options);
    lv_obj_set_size(drop, width, 30);
    lv_obj_align(drop, LV_ALIGN_RIGHT_MID, -rightOffset, 0);
    lv_obj_set_style_bg_color(drop, theme::panelLight(), 0);
    lv_obj_set_style_border_width(drop, 0, 0);
    lv_obj_set_style_text_font(drop, theme::localizedFont(&lv_font_montserrat_14), 0);
    return drop;
}

} // namespace

void create(lv_obj_t *parent, desk::FlexiSpot &desk) {
    g_desk = &desk;
    g_stepperCount = 0;

    lv_obj_set_style_pad_all(parent, 6, 0);

    lv_obj_t *list = lv_obj_create(parent);
    lv_obj_set_size(list, kScreenWidth - 12, kPageHeight - 12);
    lv_obj_set_pos(list, 0, 0);
    lv_obj_set_style_bg_opa(list, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(list, 0, 0);
    lv_obj_set_style_pad_all(list, 0, 0);
    lv_obj_set_style_pad_row(list, 6, 0);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scroll_dir(list, LV_DIR_VER);
    lv_obj_clear_flag(list, LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_ELASTIC);

    // Anzeige
    {
        lv_obj_t *row = makeRow(list, "Helligkeit");
        lv_obj_t *slider = lv_slider_create(row);
        lv_obj_set_width(slider, 260);
        lv_slider_set_range(slider, 10, 255);
        lv_slider_set_value(slider, core::settings().brightness, LV_ANIM_OFF);
        lv_obj_align(slider, LV_ALIGN_RIGHT_MID, -8, 0);
        lv_obj_add_event_cb(slider, brightnessCb, LV_EVENT_VALUE_CHANGED, nullptr);
        lv_obj_add_event_cb(slider, brightnessReleasedCb, LV_EVENT_RELEASED, nullptr);
    }
    {
        lv_obj_t *row = makeRow(list, "Display-Ruhe nach");
        lv_obj_t *drop =
            makeDropdown(row, "nie\n1 Minute\n2 Minuten\n5 Minuten\n10 Minuten", 150, 8);
        const uint32_t ms = core::settings().sleepAfterMs;
        uint16_t sel = 0;
        if (ms >= 600000UL) {
            sel = 4;
        } else if (ms >= 300000UL) {
            sel = 3;
        } else if (ms >= 120000UL) {
            sel = 2;
        } else if (ms >= 60000UL) {
            sel = 1;
        }
        lv_dropdown_set_selected(drop, sel);
        lv_obj_add_event_cb(drop, sleepCb, LV_EVENT_VALUE_CHANGED, nullptr);
    }

    // Fahrbereich
    {
        lv_obj_t *row = makeRow(list, "Fahrbereich min / max (cm)");
        makeStepper(row, 8, &core::settings().maxHeightCm, 0.5f, 60.0f, 200.0f, "");
        makeStepper(row, 150, &core::settings().minHeightCm, 0.5f, 40.0f, 180.0f, "");
    }

    // Speicherplaetze
    for (uint8_t i = 0; i < desk::kPresetCount; ++i) {
        char title[24];
        snprintf(title, sizeof(title), "Platz %u", static_cast<unsigned>(i + 1));
        lv_obj_t *row = makeRow(list, title);

        lv_obj_t *capture = theme::button(row, "Jetzt", &lv_font_montserrat_14, 0, 0, 60, 30);
        lv_obj_align(capture, LV_ALIGN_RIGHT_MID, -8, 0);
        lv_obj_set_style_bg_color(capture, theme::panelLight(), 0);
        lv_obj_add_event_cb(capture, presetCaptureCb, LV_EVENT_CLICKED,
                            reinterpret_cast<void *>(static_cast<uintptr_t>(i)));

        g_presetHeightLabels[i] =
            makeStepper(row, 76, &core::presets().at(i).heightCm, 0.5f, 40.0f, 200.0f, "");

        lv_obj_t *drop = makeDropdown(
            row, "Box-Platz 1\nBox-Platz 2\nBox-Platz 3\nBox-Platz 4\nZielhöhe", 140, 220);
        const desk::Preset &preset = core::presets().at(i);
        lv_dropdown_set_selected(drop, preset.mode == desk::PresetMode::TargetHeight
                                           ? 4
                                           : static_cast<uint16_t>(preset.boxSlot - 1));
        lv_obj_add_event_cb(drop, presetModeCb, LV_EVENT_VALUE_CHANGED,
                            reinterpret_cast<void *>(static_cast<uintptr_t>(i)));
        g_presetModeDrop[i] = drop;
    }

    // Touch
    {
        lv_obj_t *row = makeRow(list, "Touch spiegeln  X / Y");
        lv_obj_t *swY = lv_switch_create(row);
        lv_obj_align(swY, LV_ALIGN_RIGHT_MID, -8, 0);
        if (core::settings().touchInvertY) {
            lv_obj_add_state(swY, LV_STATE_CHECKED);
        }
        lv_obj_add_event_cb(swY, touchInvertCb, LV_EVENT_VALUE_CHANGED,
                            reinterpret_cast<void *>(static_cast<uintptr_t>(1)));

        lv_obj_t *swX = lv_switch_create(row);
        lv_obj_align(swX, LV_ALIGN_RIGHT_MID, -70, 0);
        if (core::settings().touchInvertX) {
            lv_obj_add_state(swX, LV_STATE_CHECKED);
        }
        lv_obj_add_event_cb(swX, touchInvertCb, LV_EVENT_VALUE_CHANGED, nullptr);
    }

    // Diagnose
    {
        lv_obj_t *row = lv_obj_create(list);
        theme::styleCard(row);
        lv_obj_set_size(row, kRowWidth, 108);
        lv_obj_set_style_pad_all(row, 8, 0);

        g_diagnostics = theme::label(row, &lv_font_montserrat_14, theme::muted(), "");
        lv_obj_align(g_diagnostics, LV_ALIGN_TOP_LEFT, 0, 0);

        lv_obj_t *reset =
            theme::button(row, "Werkseinstellungen", &lv_font_montserrat_14, 0, 0, 180, 30);
        lv_obj_align(reset, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
        lv_obj_set_style_bg_color(reset, lv_color_hex(0x3A2126), 0);
        lv_obj_set_style_bg_color(reset, theme::warn(), LV_STATE_PRESSED);
        lv_obj_add_event_cb(reset, factoryResetCb, LV_EVENT_CLICKED, nullptr);
    }
}

void tick(uint32_t nowMs) {
    for (uint8_t i = 0; i < g_stepperCount; ++i) {
        renderStepper(g_steppers[i]);
    }

    if (g_diagnostics == nullptr || g_desk == nullptr) {
        return;
    }

    char servicesLine[96] = {};
    size_t used = 0;
    for (size_t i = 0; i < services::count(); ++i) {
        if (used + 1 >= sizeof(servicesLine)) {
            break;
        }
        services::Service *service = services::at(i);
        const int written =
            snprintf(servicesLine + used, sizeof(servicesLine) - used, "%s%s: %s",
                     used == 0 ? "" : "   ", service->name(), service->statusText());
        if (written <= 0) {
            break;
        }
        used += static_cast<size_t>(written);
        if (used >= sizeof(servicesLine)) {
            used = sizeof(servicesLine) - 1;
            break;
        }
    }

    char diagnostics[384];
    snprintf(diagnostics, sizeof(diagnostics),
             "WLAN: %s  %s  (%d dBm)\n"
             "Touch: %s     Zeit: %s\n"
             "Steuerbox: %s   Anzeige \"%s\"\n"
             "Frames %lu   CRC-Fehler %lu\n"
             "%s\n"
             "Heap %lu kB   Laufzeit %lu min",
             core::wifiStateText(), core::ipAddress(), core::rssi(), board::touchChipName(),
             core::clockSynced() ? "synchron" : "-",
             g_desk->heightKnown() ? "wach" : "keine Daten", g_desk->displayText(),
             static_cast<unsigned long>(g_desk->framesOk()),
             static_cast<unsigned long>(g_desk->crcErrors()), servicesLine,
             static_cast<unsigned long>(ESP.getFreeHeap() / 1024),
             static_cast<unsigned long>(nowMs / 60000UL));
    if (strcmp(lv_label_get_text(g_diagnostics), diagnostics) != 0) {
        lv_label_set_text(g_diagnostics, diagnostics);
    }
}

} // namespace settings_screen
} // namespace ui

#include "ui/ui.h"

#include <Arduino.h>
#include <Preferences.h>
#include <lvgl.h>
#include <stdio.h>

#include "board/display.h"
#include "board/touch.h"
#include "config.h"
#include "net/net.h"
#include "net/weather.h"

namespace ui {
namespace {

constexpr int kWidth = board::kScreenWidth;   // 640
constexpr int kHeight = board::kScreenHeight; // 180
constexpr int kStatusHeight = 26;
constexpr int kPageHeight = kHeight - kStatusHeight;

constexpr uint32_t kPresetCount = 4;

const lv_color_t kBg = LV_COLOR_MAKE(0x10, 0x14, 0x1A);
const lv_color_t kPanel = LV_COLOR_MAKE(0x1C, 0x23, 0x2D);
const lv_color_t kAccent = LV_COLOR_MAKE(0x4F, 0xA3, 0xFF);
const lv_color_t kText = LV_COLOR_MAKE(0xEC, 0xF1, 0xF7);
const lv_color_t kMuted = LV_COLOR_MAKE(0x8B, 0x97, 0xA6);
const lv_color_t kWarn = LV_COLOR_MAKE(0xFF, 0x8A, 0x3D);

desk::Controller *g_desk = nullptr;
Preferences g_prefs;

bool g_useTargets = false;
float g_presetHeights[kPresetCount] = {DESK_SIT_HEIGHT_CM, 90.0f, DESK_STAND_HEIGHT_CM, 120.0f};

lv_obj_t *g_clockLabel = nullptr;
lv_obj_t *g_dateLabel = nullptr;
lv_obj_t *g_wifiLabel = nullptr;

lv_obj_t *g_heightLabel = nullptr;
lv_obj_t *g_unitLabel = nullptr;
lv_obj_t *g_deskStatus = nullptr;
lv_obj_t *g_presetBtns[kPresetCount] = {};
lv_obj_t *g_presetLabels[kPresetCount] = {};

lv_obj_t *g_wxTemp = nullptr;
lv_obj_t *g_wxDesc = nullptr;
lv_obj_t *g_wxDot = nullptr;
lv_obj_t *g_wxDetails = nullptr;
lv_obj_t *g_wxPlace = nullptr;

lv_obj_t *g_sysInfo = nullptr;
lv_obj_t *g_toast = nullptr;
uint32_t g_toastUntilMs = 0;

// --------------------------------------------------------------- Helfer ---

void styleCard(lv_obj_t *obj) {
    lv_obj_set_style_bg_color(obj, kPanel, 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_radius(obj, 10, 0);
    lv_obj_set_style_pad_all(obj, 8, 0);
}

lv_obj_t *makeLabel(lv_obj_t *parent, const lv_font_t *font, lv_color_t color,
                    const char *text) {
    lv_obj_t *label = lv_label_create(parent);
    lv_obj_set_style_text_font(label, font, 0);
    lv_obj_set_style_text_color(label, color, 0);
    lv_label_set_text(label, text);
    return label;
}

void savePrefs() {
    g_prefs.putBool("targets", g_useTargets);
    for (uint32_t i = 0; i < kPresetCount; ++i) {
        char key[8];
        snprintf(key, sizeof(key), "h%u", i);
        g_prefs.putFloat(key, g_presetHeights[i]);
    }
}

void loadPrefs() {
    g_prefs.begin("desk", false);
    g_useTargets = g_prefs.getBool("targets", false);
    for (uint32_t i = 0; i < kPresetCount; ++i) {
        char key[8];
        snprintf(key, sizeof(key), "h%u", i);
        g_presetHeights[i] = g_prefs.getFloat(key, g_presetHeights[i]);
    }
}

// ------------------------------------------------------------- Callbacks ---

void holdEventCb(lv_event_t *e) {
    const uint16_t key = static_cast<uint16_t>(reinterpret_cast<uintptr_t>(lv_event_get_user_data(e)));
    const lv_event_code_t code = lv_event_get_code(e);
    if (g_desk == nullptr) {
        return;
    }
    if (code == LV_EVENT_PRESSED) {
        g_desk->hold(key, millis());
    } else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        g_desk->release(millis());
    }
}

void presetEventCb(lv_event_t *e) {
    const uint32_t index = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(lv_event_get_user_data(e)));
    const lv_event_code_t code = lv_event_get_code(e);
    if (g_desk == nullptr || index >= kPresetCount) {
        return;
    }
    const uint32_t now = millis();

    if (code == LV_EVENT_LONG_PRESSED) {
        if (g_useTargets) {
            if (!g_desk->heightKnown()) {
                showToast("Keine Hoehe bekannt");
                return;
            }
            g_presetHeights[index] = g_desk->heightCm();
            savePrefs();
            char msg[48];
            snprintf(msg, sizeof(msg), "Platz %u = %.1f cm", index + 1,
                     static_cast<double>(g_presetHeights[index]));
            showToast(msg);
        } else {
            showToast("Speichern nur im Zielhoehen-Modus");
        }
        return;
    }

    if (code == LV_EVENT_SHORT_CLICKED) {
        if (g_useTargets) {
            g_desk->moveTo(g_presetHeights[index], now);
        } else {
            g_desk->preset(static_cast<uint8_t>(index + 1), now);
        }
    }
}

void stopEventCb(lv_event_t *) {
    if (g_desk != nullptr) {
        g_desk->stop(millis());
        showToast("Gestoppt");
    }
}

void brightnessEventCb(lv_event_t *e) {
    lv_obj_t *slider = lv_event_get_target(e);
    board::setBrightness(static_cast<uint8_t>(lv_slider_get_value(slider)));
}

void targetSwitchCb(lv_event_t *e) {
    lv_obj_t *sw = lv_event_get_target(e);
    g_useTargets = lv_obj_has_state(sw, LV_STATE_CHECKED);
    savePrefs();
}

// ----------------------------------------------------------- Seitenaufbau ---

lv_obj_t *makeBigButton(lv_obj_t *parent, const char *text, const lv_font_t *font,
                        lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h) {
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_pos(btn, x, y);
    lv_obj_set_size(btn, w, h);
    lv_obj_set_style_radius(btn, 10, 0);
    lv_obj_set_style_bg_color(btn, kPanel, 0);
    lv_obj_set_style_bg_color(btn, kAccent, LV_STATE_PRESSED);
    lv_obj_set_style_shadow_width(btn, 0, 0);

    lv_obj_t *label = lv_label_create(btn);
    lv_obj_set_style_text_font(label, font, 0);
    lv_obj_set_style_text_color(label, kText, 0);
    lv_label_set_text(label, text);
    lv_obj_center(label);
    return btn;
}

void buildDeskPage(lv_obj_t *page) {
    lv_obj_set_style_pad_all(page, 6, 0);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *card = lv_obj_create(page);
    styleCard(card);
    lv_obj_set_pos(card, 0, 0);
    lv_obj_set_size(card, 250, kPageHeight - 12);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    g_heightLabel = makeLabel(card, &lv_font_montserrat_48, kText, "--.-");
    lv_obj_align(g_heightLabel, LV_ALIGN_TOP_LEFT, 0, 6);

    g_unitLabel = makeLabel(card, &lv_font_montserrat_20, kMuted, "cm");
    lv_obj_align_to(g_unitLabel, g_heightLabel, LV_ALIGN_OUT_RIGHT_BOTTOM, 6, -6);

    g_deskStatus = makeLabel(card, &lv_font_montserrat_16, kMuted, "Start...");
    lv_obj_align(g_deskStatus, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    lv_obj_t *up = makeBigButton(page, LV_SYMBOL_UP, &lv_font_montserrat_24, 260, 0, 96, 66);
    lv_obj_add_event_cb(up, holdEventCb, LV_EVENT_PRESSED,
                        reinterpret_cast<void *>(static_cast<uintptr_t>(loctek::KEY_UP)));
    lv_obj_add_event_cb(up, holdEventCb, LV_EVENT_RELEASED,
                        reinterpret_cast<void *>(static_cast<uintptr_t>(loctek::KEY_UP)));
    lv_obj_add_event_cb(up, holdEventCb, LV_EVENT_PRESS_LOST,
                        reinterpret_cast<void *>(static_cast<uintptr_t>(loctek::KEY_UP)));

    lv_obj_t *down = makeBigButton(page, LV_SYMBOL_DOWN, &lv_font_montserrat_24, 260, 74, 96, 66);
    lv_obj_add_event_cb(down, holdEventCb, LV_EVENT_PRESSED,
                        reinterpret_cast<void *>(static_cast<uintptr_t>(loctek::KEY_DOWN)));
    lv_obj_add_event_cb(down, holdEventCb, LV_EVENT_RELEASED,
                        reinterpret_cast<void *>(static_cast<uintptr_t>(loctek::KEY_DOWN)));
    lv_obj_add_event_cb(down, holdEventCb, LV_EVENT_PRESS_LOST,
                        reinterpret_cast<void *>(static_cast<uintptr_t>(loctek::KEY_DOWN)));

    const lv_coord_t px[2] = {366, 480};
    const lv_coord_t py[2] = {0, 74};
    for (uint32_t i = 0; i < kPresetCount; ++i) {
        char text[8];
        snprintf(text, sizeof(text), "%u", i + 1);
        lv_obj_t *btn = makeBigButton(page, text, &lv_font_montserrat_20, px[i % 2], py[i / 2],
                                      108, 66);
        lv_obj_add_event_cb(btn, presetEventCb, LV_EVENT_SHORT_CLICKED,
                            reinterpret_cast<void *>(static_cast<uintptr_t>(i)));
        lv_obj_add_event_cb(btn, presetEventCb, LV_EVENT_LONG_PRESSED,
                            reinterpret_cast<void *>(static_cast<uintptr_t>(i)));
        g_presetBtns[i] = btn;
        g_presetLabels[i] = lv_obj_get_child(btn, 0);
    }

    lv_obj_t *stop = makeBigButton(page, LV_SYMBOL_STOP, &lv_font_montserrat_20, 594, 0, 34, 140);
    lv_obj_set_style_bg_color(stop, lv_color_hex(0x3A2126), 0);
    lv_obj_set_style_bg_color(stop, kWarn, LV_STATE_PRESSED);
    lv_obj_add_event_cb(stop, stopEventCb, LV_EVENT_CLICKED, nullptr);
}

void buildWeatherPage(lv_obj_t *page) {
    lv_obj_set_style_pad_all(page, 6, 0);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *card = lv_obj_create(page);
    styleCard(card);
    lv_obj_set_pos(card, 0, 0);
    lv_obj_set_size(card, kWidth - 24, kPageHeight - 12);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    g_wxTemp = makeLabel(card, &lv_font_montserrat_48, kText, "--");
    lv_obj_align(g_wxTemp, LV_ALIGN_TOP_LEFT, 4, 2);

    g_wxDot = lv_obj_create(card);
    lv_obj_set_size(g_wxDot, 18, 18);
    lv_obj_set_style_radius(g_wxDot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(g_wxDot, 0, 0);
    lv_obj_set_style_bg_color(g_wxDot, kMuted, 0);
    lv_obj_align(g_wxDot, LV_ALIGN_TOP_LEFT, 180, 14);

    g_wxDesc = makeLabel(card, &lv_font_montserrat_24, kText, "Keine Daten");
    lv_obj_align(g_wxDesc, LV_ALIGN_TOP_LEFT, 208, 12);

    g_wxDetails = makeLabel(card, &lv_font_montserrat_16, kMuted, "");
    lv_obj_align(g_wxDetails, LV_ALIGN_TOP_LEFT, 208, 48);

    g_wxPlace = makeLabel(card, &lv_font_montserrat_14, kMuted, LOCATION_NAME);
    lv_obj_align(g_wxPlace, LV_ALIGN_BOTTOM_LEFT, 4, 0);
}

void buildSystemPage(lv_obj_t *page) {
    lv_obj_set_style_pad_all(page, 6, 0);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *card = lv_obj_create(page);
    styleCard(card);
    lv_obj_set_pos(card, 0, 0);
    lv_obj_set_size(card, 380, kPageHeight - 12);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    g_sysInfo = makeLabel(card, &lv_font_montserrat_14, kMuted, "");
    lv_obj_align(g_sysInfo, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_width(g_sysInfo, 360);

    lv_obj_t *right = lv_obj_create(page);
    styleCard(right);
    lv_obj_set_pos(right, 392, 0);
    lv_obj_set_size(right, kWidth - 392 - 24, kPageHeight - 12);
    lv_obj_clear_flag(right, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *brightnessLabel = makeLabel(right, &lv_font_montserrat_14, kMuted, "Helligkeit");
    lv_obj_align(brightnessLabel, LV_ALIGN_TOP_LEFT, 0, 0);

    lv_obj_t *slider = lv_slider_create(right);
    lv_obj_set_width(slider, 190);
    lv_slider_set_range(slider, 10, 255);
    lv_slider_set_value(slider, board::brightness(), LV_ANIM_OFF);
    lv_obj_align(slider, LV_ALIGN_TOP_LEFT, 0, 24);
    lv_obj_add_event_cb(slider, brightnessEventCb, LV_EVENT_VALUE_CHANGED, nullptr);

    lv_obj_t *modeLabel = makeLabel(right, &lv_font_montserrat_14, kMuted, "Geregelt fahren");
    lv_obj_align(modeLabel, LV_ALIGN_TOP_LEFT, 0, 62);

    lv_obj_t *sw = lv_switch_create(right);
    lv_obj_align(sw, LV_ALIGN_TOP_LEFT, 0, 84);
    if (g_useTargets) {
        lv_obj_add_state(sw, LV_STATE_CHECKED);
    }
    lv_obj_add_event_cb(sw, targetSwitchCb, LV_EVENT_VALUE_CHANGED, nullptr);
}

void buildStatusBar(lv_obj_t *parent) {
    lv_obj_t *bar = lv_obj_create(parent);
    lv_obj_set_pos(bar, 0, 0);
    lv_obj_set_size(bar, kWidth, kStatusHeight);
    lv_obj_set_style_bg_color(bar, kBg, 0);
    lv_obj_set_style_border_width(bar, 0, 0);
    lv_obj_set_style_radius(bar, 0, 0);
    lv_obj_set_style_pad_all(bar, 0, 0);
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);

    g_clockLabel = makeLabel(bar, &lv_font_montserrat_16, kText, "--:--");
    lv_obj_align(g_clockLabel, LV_ALIGN_LEFT_MID, 10, 0);

    g_dateLabel = makeLabel(bar, &lv_font_montserrat_14, kMuted, "");
    lv_obj_align(g_dateLabel, LV_ALIGN_LEFT_MID, 70, 0);

    g_wifiLabel = makeLabel(bar, &lv_font_montserrat_14, kMuted, LV_SYMBOL_WIFI);
    lv_obj_align(g_wifiLabel, LV_ALIGN_RIGHT_MID, -10, 0);
}

const char *moveResultText(desk::MoveResult r) {
    switch (r) {
    case desk::MoveResult::Reached: return "Ziel erreicht";
    case desk::MoveResult::Stalled: return "Blockiert - gestoppt";
    case desk::MoveResult::TimedOut: return "Zeitueberschreitung";
    case desk::MoveResult::NoFeedback: return "Keine Rueckmeldung";
    default: return "";
    }
}

} // namespace

// -------------------------------------------------------------- Oeffentlich ---

void begin(desk::Controller &controller) {
    g_desk = &controller;
    loadPrefs();

    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, kBg, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    buildStatusBar(scr);

    lv_obj_t *tiles = lv_tileview_create(scr);
    lv_obj_set_pos(tiles, 0, kStatusHeight);
    lv_obj_set_size(tiles, kWidth, kPageHeight);
    lv_obj_set_style_bg_color(tiles, kBg, 0);
    lv_obj_set_style_border_width(tiles, 0, 0);

    lv_obj_t *deskTile = lv_tileview_add_tile(tiles, 0, 0, LV_DIR_RIGHT);
    lv_obj_t *weatherTile = lv_tileview_add_tile(tiles, 1, 0, LV_DIR_HOR);
    lv_obj_t *systemTile = lv_tileview_add_tile(tiles, 2, 0, LV_DIR_LEFT);

    buildDeskPage(deskTile);
    buildWeatherPage(weatherTile);
    buildSystemPage(systemTile);

    g_toast = lv_label_create(lv_layer_top());
    lv_obj_set_style_bg_color(g_toast, kAccent, 0);
    lv_obj_set_style_bg_opa(g_toast, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(g_toast, lv_color_black(), 0);
    lv_obj_set_style_text_font(g_toast, &lv_font_montserrat_16, 0);
    lv_obj_set_style_pad_all(g_toast, 8, 0);
    lv_obj_set_style_radius(g_toast, 8, 0);
    lv_label_set_text(g_toast, "");
    lv_obj_align(g_toast, LV_ALIGN_BOTTOM_MID, 0, -8);
    lv_obj_add_flag(g_toast, LV_OBJ_FLAG_HIDDEN);
}

void showToast(const char *text) {
    if (g_toast == nullptr) {
        return;
    }
    lv_label_set_text(g_toast, text);
    lv_obj_clear_flag(g_toast, LV_OBJ_FLAG_HIDDEN);
    lv_obj_align(g_toast, LV_ALIGN_BOTTOM_MID, 0, -8);
    g_toastUntilMs = millis() + 2500;
}

bool useTargetHeights() { return g_useTargets; }

void tick(uint32_t nowMs) {
    // Statuszeile
    lv_label_set_text(g_clockLabel, net::clockText());
    lv_label_set_text(g_dateLabel, net::dateText());
    lv_obj_set_style_text_color(
        g_wifiLabel, net::wifiState() == net::WifiState::Connected ? kAccent : kMuted, 0);

    // Schreibtisch
    if (g_desk != nullptr) {
        if (g_desk->heightKnown()) {
            lv_label_set_text_fmt(g_heightLabel, "%.1f", static_cast<double>(g_desk->heightCm()));
        } else if (g_desk->displayText()[0] != '\0') {
            lv_label_set_text(g_heightLabel, g_desk->displayText());
        } else {
            lv_label_set_text(g_heightLabel, "--.-");
        }

        char status[64];
        if (g_desk->targetActive()) {
            snprintf(status, sizeof(status), "Ziel %.1f cm", static_cast<double>(g_desk->targetCm()));
        } else if (g_desk->motion() == desk::Motion::Up) {
            snprintf(status, sizeof(status), "faehrt hoch");
        } else if (g_desk->motion() == desk::Motion::Down) {
            snprintf(status, sizeof(status), "faehrt runter");
        } else if (g_desk->lastMoveResult() != desk::MoveResult::None) {
            snprintf(status, sizeof(status), "%s", moveResultText(g_desk->lastMoveResult()));
        } else if (!g_desk->heightKnown()) {
            snprintf(status, sizeof(status), "Steuerbox meldet nichts");
        } else {
            snprintf(status, sizeof(status), "%s", g_useTargets ? "bereit - Zielhoehen" : "bereit");
        }
        lv_label_set_text(g_deskStatus, status);

        for (uint32_t i = 0; i < kPresetCount; ++i) {
            if (g_presetLabels[i] == nullptr) {
                continue;
            }
            if (g_useTargets) {
                lv_label_set_text_fmt(g_presetLabels[i], "%.0f",
                                      static_cast<double>(g_presetHeights[i]));
            } else {
                lv_label_set_text_fmt(g_presetLabels[i], "%u", i + 1);
            }
        }
    }

    // Wetter
    const weather::Data &wx = weather::data();
    if (wx.valid) {
        lv_label_set_text_fmt(g_wxTemp, "%.0f C", static_cast<double>(wx.temperature));
        lv_label_set_text(g_wxDesc, weather::description(wx.code));
        lv_obj_set_style_bg_color(g_wxDot, lv_color_hex(weather::accentColor(wx.code)), 0);
        lv_label_set_text_fmt(g_wxDetails,
                              "min %.0f / max %.0f   gefuehlt %.0f\n"
                              "%d %% Luftfeuchte   %.0f km/h Wind   %d %% Regen",
                              static_cast<double>(wx.todayMin), static_cast<double>(wx.todayMax),
                              static_cast<double>(wx.apparent), wx.humidity,
                              static_cast<double>(wx.windKmh), wx.precipitationProb);
    }

    // System
    if (g_desk != nullptr) {
        lv_label_set_text_fmt(g_sysInfo,
                              "WLAN: %s  (%s, %d dBm)\n"
                              "Touch: %s\n"
                              "Steuerbox: %s   Anzeige \"%s\"\n"
                              "Frames %lu   CRC-Fehler %lu\n"
                              "Heap %lu kB   Laufzeit %lu min",
                              net::wifiStateText(), net::ipAddress(), net::rssi(),
                              board::touchChipName(),
                              g_desk->heightKnown() ? "wach" : "keine Daten",
                              g_desk->displayText(),
                              static_cast<unsigned long>(g_desk->framesOk()),
                              static_cast<unsigned long>(g_desk->crcErrors()),
                              static_cast<unsigned long>(ESP.getFreeHeap() / 1024),
                              static_cast<unsigned long>(nowMs / 60000UL));
    }

    if (g_toast != nullptr && !lv_obj_has_flag(g_toast, LV_OBJ_FLAG_HIDDEN) &&
        static_cast<int32_t>(nowMs - g_toastUntilMs) >= 0) {
        lv_obj_add_flag(g_toast, LV_OBJ_FLAG_HIDDEN);
    }
}

} // namespace ui

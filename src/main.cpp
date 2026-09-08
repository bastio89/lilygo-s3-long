// Flexispot-/LoctekMotion-Schreibtischsteuerung auf dem LilyGo T-Display-S3 Long.
//
// Aufbau:
//   board/  Panel, Touch, Backlight
//   desk/   serielles Protokoll der Steuerbox + Ablauflogik
//   net/    WLAN, Zeit, Wetter
//   ui/     LVGL-Oberflaeche
#include <Arduino.h>
#include <lvgl.h>

#include "board/display.h"
#include "board/pins.h"
#include "board/touch.h"
#include "config.h"
#include "desk/desk_serial.h"
#include "net/net.h"
#include "net/weather.h"
#include "ui/ui.h"

namespace {

HardwareSerial g_deskPort(1);
desk::SerialIo g_deskIo(g_deskPort);
desk::Controller *g_desk = nullptr;

uint32_t g_lastUiTickMs = 0;
constexpr uint32_t kUiTickMs = 200;

// Mit -DDESK_SNIFFER=1 werden alle gueltigen Frames der Steuerbox auf der
// USB-Konsole ausgegeben -- hilfreich beim ersten Anschliessen.
#ifndef DESK_SNIFFER
#define DESK_SNIFFER 0
#endif

#if DESK_SNIFFER
void logFrame(const loctek::Frame &frame, void *) {
    Serial.printf("[desk] type=0x%02X len=%u payload=", frame.type, frame.payloadLen);
    for (uint8_t i = 0; i < frame.payloadLen; ++i) {
        Serial.printf("%02X ", frame.payload[i]);
    }
    Serial.println();
}
#endif

desk::Config deskConfig() {
    desk::Config cfg;
    cfg.minHeightCm = DESK_MIN_HEIGHT_CM;
    cfg.maxHeightCm = DESK_MAX_HEIGHT_CM;
    return cfg;
}

void handleDisplaySleep() {
#if DISPLAY_SLEEP_AFTER_MS > 0
    const uint32_t idle = lv_disp_get_inactive_time(nullptr);
    const bool busy = g_desk != nullptr && (g_desk->moving() || g_desk->targetActive());
    if (!board::displayAsleep() && idle > DISPLAY_SLEEP_AFTER_MS && !busy) {
        board::displaySleep();
    } else if (board::displayAsleep() && (idle < DISPLAY_SLEEP_AFTER_MS || busy)) {
        board::displayWake();
    }
#endif
}

} // namespace

void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println("\nT-Display-S3 Long :: Flexispot-Steuerung");

    if (!board::touchBegin()) {
        Serial.println("WARNUNG: kein Touchcontroller gefunden");
    } else {
        Serial.printf("Touch: %s\n", board::touchChipName());
    }

    if (!board::displayBegin()) {
        Serial.println("FEHLER: Display-Init fehlgeschlagen");
    }
    board::setBrightness(DISPLAY_BRIGHTNESS);

    g_deskIo.begin();
    static desk::Controller controller(g_deskIo, deskConfig());
    g_desk = &controller;
#if DESK_SNIFFER
    controller.onFrame(logFrame, nullptr);
#endif
    controller.begin(millis());

    ui::begin(controller);

    net::begin();
    weather::begin();

    Serial.printf("Schreibtisch-UART: RX=%d TX=%d WAKE=%d @ %d Baud\n", DESK_UART_RX_PIN,
                  DESK_UART_TX_PIN, DESK_WAKE_PIN, DESK_UART_BAUD);
}

void loop() {
    const uint32_t now = millis();

    if (g_desk != nullptr) {
        g_desk->poll(now);
    }
    net::loop(now);
    weather::loop(now);

    if (static_cast<uint32_t>(now - g_lastUiTickMs) >= kUiTickMs) {
        g_lastUiTickMs = now;
        ui::tick(now);
    }

    handleDisplaySleep();
    board::displayLoop();
    delay(2);
}

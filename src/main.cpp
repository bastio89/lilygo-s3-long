// SmartDesk -- Flexispot-/LoctekMotion-Schreibtischsteuerung auf dem
// LilyGo T-Display-S3 Long.
//
//   board/     Panel, Touch, Backlight (Hardwareschicht)
//   core/      Netz, Einstellungen, Zeit
//   desk/      Protokoll der Steuerbox, Fahrlogik, Speicherplaetze
//   services/  Hintergrunddienste (Wetter, spaeter Kalender ...)
//   ui/        Dashboard und Seiten
#include <Arduino.h>
#include <lvgl.h>

#include "board/display.h"
#include "board/pins.h"
#include "board/touch.h"
#include "core/clock.h"
#include "core/network.h"
#include "core/settings.h"
#include "desk/flexispot.h"
#include "desk/serial_io.h"
#include "services/service.h"
#include "services/weather.h"
#include "ui/dashboard.h"

#ifndef DESK_SNIFFER
#define DESK_SNIFFER 0
#endif

namespace {

constexpr uint32_t kUiTickMs = 200;

HardwareSerial g_deskPort(1);
desk::SerialIo g_deskIo(g_deskPort);
desk::FlexiSpot *g_desk = nullptr;
uint32_t g_lastUiTickMs = 0;

#if DESK_SNIFFER
// Mithoer-Modus fuer die erste Verkabelung. Zeigt zweierlei: was der Parser
// als gueltiges Frame erkannt hat, und -- unabhaengig davon -- ob ueberhaupt
// Bytes ankommen. Kommen Bytes, aber keine Frames, stimmt die Verkabelung
// grundsaetzlich und es hakt an Pegel, Polaritaet oder Baudrate.
constexpr size_t kRawTailSize = 24;
uint8_t g_rawTail[kRawTailSize];
size_t g_rawTailLen = 0;
size_t g_rawTailPos = 0;
uint32_t g_lastSummaryMs = 0;

void logRawByte(uint8_t byte, void *) {
    g_rawTail[g_rawTailPos] = byte;
    g_rawTailPos = (g_rawTailPos + 1) % kRawTailSize;
    if (g_rawTailLen < kRawTailSize) {
        ++g_rawTailLen;
    }
}

void logFrame(const loctek::Frame &frame, void *) {
    Serial.printf("[frame] typ=0x%02X ", frame.type);
    for (uint8_t i = 0; i < frame.payloadLen; ++i) {
        Serial.printf("%02X ", frame.payload[i]);
    }
    if (frame.type == loctek::kTypeDisplay && frame.payloadLen >= 3) {
        loctek::DisplayValue value;
        if (loctek::decodeDisplay(frame.payload, value)) {
            if (value.numeric) {
                Serial.printf("-> Anzeige \"%s\" = %.1f", value.text,
                              static_cast<double>(value.value));
            } else {
                Serial.printf("-> Anzeige \"%s\" (keine Zahl)", value.text);
            }
        } else {
            Serial.print("-> Segmentmuster unbekannt");
        }
    }
    Serial.println();
}

void logSummary(uint32_t nowMs) {
    if (static_cast<uint32_t>(nowMs - g_lastSummaryMs) < 2000) {
        return;
    }
    g_lastSummaryMs = nowMs;

    Serial.printf("[roh] %lu Bytes | %lu Frames ok | %lu CRC-Fehler | zuletzt: ",
                  static_cast<unsigned long>(g_desk->bytesReceived()),
                  static_cast<unsigned long>(g_desk->framesOk()),
                  static_cast<unsigned long>(g_desk->crcErrors()));
    if (g_rawTailLen == 0) {
        Serial.print("nichts empfangen");
    } else {
        const size_t start = (g_rawTailPos + kRawTailSize - g_rawTailLen) % kRawTailSize;
        for (size_t i = 0; i < g_rawTailLen; ++i) {
            Serial.printf("%02X ", g_rawTail[(start + i) % kRawTailSize]);
        }
    }
    Serial.println();
}
#endif

desk::Config deskConfigFromSettings() {
    desk::Config cfg;
    cfg.minHeightCm = core::settings().minHeightCm;
    cfg.maxHeightCm = core::settings().maxHeightCm;
    return cfg;
}

// Fahrbereich nachziehen, falls er auf der Einstellungsseite geaendert wurde.
void syncDeskLimits() {
    const desk::Config current = g_desk->config();
    if (current.minHeightCm != core::settings().minHeightCm ||
        current.maxHeightCm != core::settings().maxHeightCm) {
        g_desk->setConfig(deskConfigFromSettings());
    }
}

void handleDisplaySleep() {
    const uint32_t after = core::settings().sleepAfterMs;
    if (after == 0) {
        return;
    }
    const uint32_t idle = lv_disp_get_inactive_time(nullptr);
    const bool busy = g_desk->moving() || g_desk->targetActive();
    if (!board::displayAsleep() && idle > after && !busy) {
        board::displaySleep();
    } else if (board::displayAsleep() && (idle < after || busy)) {
        board::displayWake();
    }
}

} // namespace

void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println("\nSmartDesk auf T-Display-S3 Long");

    core::settingsBegin();

    if (!board::touchBegin()) {
        Serial.println("WARNUNG: kein Touchcontroller gefunden");
    } else {
        Serial.printf("Touch: %s\n", board::touchChipName());
    }
    board::touchSetInvert(core::settings().touchInvertX, core::settings().touchInvertY);

    if (!board::displayBegin()) {
        Serial.println("FEHLER: Display-Init fehlgeschlagen");
    }
    board::setBrightness(core::settings().brightness);

    g_deskIo.begin();
    static desk::FlexiSpot flexispot(g_deskIo, deskConfigFromSettings());
    g_desk = &flexispot;
#if DESK_SNIFFER
    flexispot.onFrame(logFrame, nullptr);
    flexispot.onRawByte(logRawByte, nullptr);
    Serial.println("Mithoer-Modus aktiv (DESK_SNIFFER=1)");
#endif
    flexispot.begin(millis());

    ui::dashboard::begin(flexispot);

    core::networkBegin();
    core::clockBegin();

    services::registerService(services::weather());
    services::beginAll();

    Serial.printf("Schreibtisch-UART: RX=%d TX=%d WAKE=%d @ %d Baud\n", DESK_UART_RX_PIN,
                  DESK_UART_TX_PIN, DESK_WAKE_PIN, DESK_UART_BAUD);
}

void loop() {
    const uint32_t now = millis();

    g_desk->poll(now);
    core::networkLoop(now);
    core::clockLoop(now);
    services::loopAll(now);

    if (static_cast<uint32_t>(now - g_lastUiTickMs) >= kUiTickMs) {
        g_lastUiTickMs = now;
        syncDeskLimits();
        ui::dashboard::tick(now);
    }

#if DESK_SNIFFER
    logSummary(now);
#endif

    handleDisplaySleep();
    board::displayLoop();
    delay(2);
}

// Hardwareunabhaengige Ansteuerung einer LoctekMotion-/Flexispot-Steuerbox.
//
// Das echte Handpanel sendet permanent seinen Tastenzustand an die Steuerbox
// (auch "keine Taste"). Genau das bildet diese Klasse nach: solange sie
// "wach" ist, geht alle `keyRepeatMs` ein Frame raus. Ein Tastendruck ist also
// kein einzelnes Kommando, sondern ein gehaltener Zustand -- deshalb kann der
// Tisch mit `hold()` / `release()` exakt wie mit den Originaltasten gefahren
// werden, und `moveTo()` regelt darueber auf eine Zielhoehe.
//
// Frei von Arduino-Abhaengigkeiten (Zeit kommt von aussen in poll()), damit
// die Ablauflogik nativ getestet werden kann: pio test -e native
#pragma once

#include <stddef.h>
#include <stdint.h>

#include "desk/protocol.h"

namespace desk {

// Transport zur Steuerbox (UART + optionaler PIN20-Weckleitung).
class Io {
  public:
    virtual ~Io() = default;
    virtual int available() = 0;
    virtual int read() = 0;
    virtual void write(const uint8_t *data, size_t len) = 0;
    // PIN20 der RJ45-Buchse: HIGH weckt die Steuerbox.
    virtual void setWakeLine(bool high) { (void)high; }
};

enum class Motion : uint8_t { Idle, Up, Down };

enum class MoveResult : uint8_t {
    None,      // kein Fahrauftrag aktiv / laeuft noch
    Reached,   // Zielhoehe erreicht
    Stalled,   // Hoehe aendert sich nicht mehr (Hindernis/Endlage)
    TimedOut,  // Fahrauftrag hat zu lange gedauert
    NoFeedback // Steuerbox meldet keine Hoehe -> Regelung nicht moeglich
};

struct Config {
    float minHeightCm = 60.0f;
    float maxHeightCm = 130.0f;
    float targetToleranceCm = 0.5f;
    // Vorhaltung: der Tisch laeuft nach dem Loslassen noch etwas nach.
    float stopMarginCm = 0.8f;

    uint32_t keyRepeatMs = 100;      // Sendetakt des simulierten Panels
    uint32_t keepAwakeMs = 20000;    // danach Panel-Heartbeat einstellen
    uint32_t heightStaleMs = 3000;   // danach gilt die Hoehe als unbekannt
    uint32_t moveTimeoutMs = 45000;  // Not-Aus fuer moveTo()
    uint32_t stallTimeoutMs = 2500;  // keine Hoehenaenderung -> abbrechen
    float stallEpsilonCm = 0.2f;
};

class FlexiSpot {
  public:
    FlexiSpot(Io &io, const Config &config = Config{});

    void begin(uint32_t nowMs);
    // Muss regelmaessig (>= alle 10 ms) aufgerufen werden.
    void poll(uint32_t nowMs);

    // --- Bedienung -------------------------------------------------------
    void wake(uint32_t nowMs);              // Steuerbox aufwecken
    void hold(uint16_t keys, uint32_t now); // Taste(n) gedrueckt halten
    void release(uint32_t nowMs);           // alle Tasten loslassen
    void tap(uint16_t keys, uint32_t nowMs, uint32_t durationMs = 350);
    void preset(uint8_t index, uint32_t nowMs); // 1..4
    // Faehrt geregelt auf `cm`. Bricht einen laufenden Auftrag ab.
    void moveTo(float cm, uint32_t nowMs);
    void stop(uint32_t nowMs);

    // --- Zustand ---------------------------------------------------------
    bool heightKnown() const { return heightValid_; }
    float heightCm() const { return heightCm_; }
    const char *displayText() const { return display_.text; }
    Motion motion() const { return motion_; }
    bool moving() const { return motion_ != Motion::Idle; }
    bool targetActive() const { return targetActive_; }
    float targetCm() const { return targetCm_; }
    MoveResult lastMoveResult() const { return lastMoveResult_; }
    bool awake() const { return awake_; }

    uint32_t framesOk() const { return parser_.framesOk(); }
    uint32_t crcErrors() const { return parser_.crcErrors(); }

    // Callback fuer jedes gueltige Frame (Debug/Sniffer). Optional.
    void onFrame(void (*cb)(const loctek::Frame &, void *), void *ctx) {
        frameCb_ = cb;
        frameCtx_ = ctx;
    }

    // Callback fuer jedes empfangene Byte, noch vor dem Parser. Zeigt beim
    // Anschliessen, ob ueberhaupt etwas ankommt -- auch wenn die Framegrenzen
    // nicht stimmen und deshalb kein Frame zustande kommt. Optional.
    void onRawByte(void (*cb)(uint8_t, void *), void *ctx) {
        rawCb_ = cb;
        rawCtx_ = ctx;
    }

    // Anzahl empfangener Bytes, unabhaengig davon, ob sie Frames ergaben.
    uint32_t bytesReceived() const { return bytesReceived_; }

    const Config &config() const { return config_; }
    void setConfig(const Config &c) { config_ = c; }

  private:
    void pumpRx(uint32_t nowMs);
    void pumpTx(uint32_t nowMs);
    void updateTarget(uint32_t nowMs);
    void finishMove(MoveResult result, uint32_t nowMs);
    void sendKeys(uint16_t keys);

    Io &io_;
    Config config_;
    loctek::Parser parser_;

    uint16_t heldKeys_ = loctek::KEY_NONE;
    uint16_t lastSentKeys_ = loctek::KEY_NONE;
    uint32_t holdUntilMs_ = 0; // fuer tap(): 0 = unbegrenzt halten
    bool holdLimited_ = false;

    bool awake_ = false;
    uint32_t awakeUntilMs_ = 0;
    uint32_t lastTxMs_ = 0;

    bool heightValid_ = false;
    float heightCm_ = 0.0f;
    uint32_t lastHeightMs_ = 0;
    loctek::DisplayValue display_{};

    Motion motion_ = Motion::Idle;

    bool targetActive_ = false;
    float targetCm_ = 0.0f;
    uint32_t moveStartedMs_ = 0;
    float stallReferenceCm_ = 0.0f;
    uint32_t stallSinceMs_ = 0;
    MoveResult lastMoveResult_ = MoveResult::None;

    uint32_t bytesReceived_ = 0;

    void (*frameCb_)(const loctek::Frame &, void *) = nullptr;
    void *frameCtx_ = nullptr;
    void (*rawCb_)(uint8_t, void *) = nullptr;
    void *rawCtx_ = nullptr;
};

} // namespace desk

// LoctekMotion / Flexispot serielles Protokoll (RJ45-Handschalter-Bus).
//
// Frameaufbau:
//
//   0x9B | LEN | TYPE | PAYLOAD... | CRC_HI | CRC_LO | 0x9D
//
//   LEN   Anzahl Bytes ab LEN bis einschliesslich CRC_LO (also 6 bei einem
//         Tastenkommando mit 2 Byte Payload).
//   CRC   CRC-16/MODBUS (Poly 0xA001, Init 0xFFFF) ueber LEN..PAYLOAD,
//         uebertragen mit High-Byte zuerst.
//
// Die Konstanten wurden gegen die bekannten Frames aus
// https://github.com/iMicknl/LoctekMotion_IoT verifiziert (siehe
// test/test_desk/test_protocol.cpp).
//
// Bewusst frei von Arduino-Abhaengigkeiten, damit die Logik nativ
// getestet werden kann:  pio test -e native
#pragma once

#include <stddef.h>
#include <stdint.h>

namespace loctek {

constexpr uint8_t kFrameStart = 0x9B;
constexpr uint8_t kFrameEnd = 0x9D;

// Nachrichtentypen
constexpr uint8_t kTypeKey = 0x02;     // Panel -> Steuerbox: Tastenzustand
constexpr uint8_t kTypeDisplay = 0x12; // Steuerbox -> Panel: 7-Segment-Inhalt

// Tastenmaske im Payload (Little-Endian 16 Bit).
enum Key : uint16_t {
    KEY_NONE = 0x0000,
    KEY_UP = 0x0001,
    KEY_DOWN = 0x0002,
    KEY_PRESET_1 = 0x0004,
    KEY_PRESET_2 = 0x0008,
    KEY_PRESET_3 = 0x0010, // auf vielen Panels "stehen"
    KEY_M = 0x0020,        // Memory/Set
    KEY_PRESET_4 = 0x0100, // auf vielen Panels "sitzen"
};

constexpr size_t kKeyFrameSize = 8;

uint16_t crc16Modbus(const uint8_t *data, size_t len);

// Baut ein Tastenkommando. `out` muss mindestens kKeyFrameSize gross sein.
// Ein leerer Tastensatz (KEY_NONE) ist das "Wake Up"-Frame.
size_t buildKeyFrame(uint16_t keys, uint8_t *out, size_t capacity);

// Ein vollstaendig empfangenes, CRC-geprueftes Frame.
struct Frame {
    uint8_t type = 0;
    uint8_t payload[16] = {};
    uint8_t payloadLen = 0;
};

// Byteweiser Frame-Parser fuer den UART-Strom der Steuerbox. Resynchronisiert
// sich selbst, wenn Bytes verloren gehen.
class Parser {
  public:
    void reset();
    // Liefert true, sobald `frame` ein gueltiges Frame enthaelt.
    bool feed(uint8_t byte, Frame &frame);

    uint32_t crcErrors() const { return crcErrors_; }
    uint32_t framesOk() const { return framesOk_; }

  private:
    enum class State : uint8_t { WaitStart, Length, Body };

    State state_ = State::WaitStart;
    uint8_t buf_[24] = {};
    uint8_t idx_ = 0;
    uint8_t expected_ = 0; // Bytes ab LEN bis CRC_LO
    uint32_t crcErrors_ = 0;
    uint32_t framesOk_ = 0;
};

// Dekodiertes 7-Segment-Display der Steuerbox.
struct DisplayValue {
    char text[4] = {0}; // z.B. "735", "ASr", "E01"
    bool numeric = false;
    float value = 0.0f; // nur gueltig wenn numeric == true
};

// Wandelt ein einzelnes 7-Segment-Byte in ein Zeichen (0 = unbekannt).
char decodeSegment(uint8_t segments);

// Dekodiert die drei Segment-Bytes eines kTypeDisplay-Frames.
bool decodeDisplay(const uint8_t *segments, DisplayValue &out);

} // namespace loctek

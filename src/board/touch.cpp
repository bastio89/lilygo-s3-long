#include "board/touch.h"

#include <Arduino.h>
#include <Wire.h>

#include "board/pins.h"

namespace board {
namespace {

constexpr uint8_t kAddrCst3xx = 0x1A;
constexpr uint8_t kAddrCst3530 = 0x58;
constexpr uint8_t kAddrAxs = 0x3B;

TouchChip g_chip = TouchChip::None;
bool g_invertX = false;
bool g_invertY = false;

bool probe(uint8_t address) {
    Wire.beginTransmission(address);
    return Wire.endTransmission() == 0;
}

void resetTouch(uint8_t resetPin) {
    pinMode(resetPin, OUTPUT);
    digitalWrite(resetPin, LOW);
    delay(20);
    digitalWrite(resetPin, HIGH);
    delay(60);
}

// --- CST3xx ---------------------------------------------------------------
// Ablauf laut Hynitron-Referenztreiber: Register 0xD000 lesen, 7 Bytes holen,
// Gueltigkeit ueber das Schlussbyte 0xAB pruefen, danach 0xD000 = 0xAB
// zurueckschreiben ("tail end"), sonst liefert der Chip keine neuen Daten.
bool cstRead(TouchPoint &point) {
    uint8_t buf[7];

    Wire.beginTransmission(kAddrCst3xx);
    Wire.write(0xD0);
    Wire.write(0x00);
    if (Wire.endTransmission(false) != 0) {
        return false;
    }
    if (Wire.requestFrom(kAddrCst3xx, static_cast<uint8_t>(sizeof(buf))) != sizeof(buf)) {
        return false;
    }
    for (uint8_t &b : buf) {
        b = Wire.read();
    }

    // Lesevorgang quittieren.
    Wire.beginTransmission(kAddrCst3xx);
    Wire.write(0xD0);
    Wire.write(0x00);
    Wire.write(0xAB);
    Wire.endTransmission();

    if (buf[6] != 0xAB || buf[0] == 0xAB) {
        return false;
    }
    if ((buf[5] & 0x7F) == 0 || (buf[5] & 0x80)) {
        return false; // keine Finger bzw. Tastenreport
    }
    point.x = static_cast<int16_t>((static_cast<uint16_t>(buf[1]) << 4) | (buf[3] >> 4));
    point.y = static_cast<int16_t>((static_cast<uint16_t>(buf[2]) << 4) | (buf[3] & 0x0F));
    return true;
}

bool cst3530Read(TouchPoint &point) {
    static const uint8_t kReadCommand[4] = {0xD0, 0x07, 0x00, 0x00};
    static const uint8_t kAcknowledge[4] = {0xD0, 0x00, 0x02, 0xAB};
    uint8_t buf[9];

    Wire.beginTransmission(kAddrCst3530);
    Wire.write(kReadCommand, sizeof(kReadCommand));
    if (Wire.endTransmission() != 0) {
        return false;
    }
    if (Wire.requestFrom(kAddrCst3530, static_cast<uint8_t>(sizeof(buf))) != sizeof(buf)) {
        return false;
    }
    for (uint8_t &b : buf) {
        b = Wire.read();
    }

    Wire.beginTransmission(kAddrCst3530);
    Wire.write(kAcknowledge, sizeof(kAcknowledge));
    Wire.endTransmission();

    const uint8_t fingerCount = buf[3] & 0x0F;
    const uint8_t event = buf[8] >> 4;
    if (buf[2] != 0xFF || fingerCount == 0 || event == 0) {
        return false;
    }

    point.x = static_cast<int16_t>(buf[4] | ((buf[7] & 0x0F) << 8));
    point.y = static_cast<int16_t>(buf[5] | ((buf[7] & 0xF0) << 4));
    return true;
}

// --- AXS15231B ------------------------------------------------------------
bool axsRead(TouchPoint &point) {
    static const uint8_t kReadCmd[11] = {0xB5, 0xAB, 0xA5, 0x5A, 0x00, 0x00,
                                         0x00, 0x08, 0x00, 0x00, 0x00};
    uint8_t buf[8];

    Wire.beginTransmission(kAddrAxs);
    Wire.write(kReadCmd, sizeof(kReadCmd));
    if (Wire.endTransmission() != 0) {
        return false;
    }
    if (Wire.requestFrom(kAddrAxs, static_cast<uint8_t>(sizeof(buf))) != sizeof(buf)) {
        return false;
    }
    for (uint8_t &b : buf) {
        b = Wire.read();
    }

    const uint8_t fingers = buf[1];
    const uint8_t event = buf[2] >> 4;
    if (fingers != 1 || event != 0x08) {
        return false;
    }
    const uint16_t rawX = ((static_cast<uint16_t>(buf[4] & 0x0F)) << 8) | buf[5];
    const uint16_t rawY = ((static_cast<uint16_t>(buf[2] & 0x0F)) << 8) | buf[3];
    point.x = static_cast<int16_t>(rawX);
    point.y = static_cast<int16_t>(TFT_PANEL_HEIGHT - 1 - rawY);
    return true;
}

} // namespace

bool touchBegin() {
    resetTouch(TOUCH_RST);

    pinMode(TOUCH_IRQ, INPUT);

    Wire.begin(TOUCH_I2C_SDA, TOUCH_I2C_SCL, 400000);

    if (probe(kAddrCst3530)) {
        g_chip = TouchChip::CST3530;
    } else if (probe(kAddrCst3xx)) {
        g_chip = TouchChip::CST3xx;
    } else {
        resetTouch(TFT_QSPI_RST);
        g_chip = probe(kAddrAxs) ? TouchChip::AXS15231B : TouchChip::None;
    }
    return g_chip != TouchChip::None;
}

TouchChip touchChip() { return g_chip; }

const char *touchChipName() {
    switch (g_chip) {
    case TouchChip::CST3xx: return "CST3xx";
    case TouchChip::CST3530: return "CST3530";
    case TouchChip::AXS15231B: return "AXS15231B";
    default: return "keiner";
    }
}

void touchSetInvert(bool invertX, bool invertY) {
    g_invertX = invertX;
    g_invertY = invertY;
}

bool touchRead(TouchPoint &point) {
    bool pressed = false;
    switch (g_chip) {
    case TouchChip::CST3xx: pressed = cstRead(point); break;
    case TouchChip::CST3530: pressed = cst3530Read(point); break;
    case TouchChip::AXS15231B: pressed = axsRead(point); break;
    default: return false;
    }
    if (!pressed) {
        return false;
    }

    if (g_invertX) {
        point.x = TFT_PANEL_WIDTH - 1 - point.x;
    }
    if (g_invertY) {
        point.y = TFT_PANEL_HEIGHT - 1 - point.y;
    }
    point.x = constrain(point.x, 0, TFT_PANEL_WIDTH - 1);
    point.y = constrain(point.y, 0, TFT_PANEL_HEIGHT - 1);
    return true;
}

} // namespace board

// Kapazitiver Touch des T-Display-S3 Long.
//
// Vom Board gibt es mehrere Revisionen: CST3xx (I2C 0x1A), CST3530
// (I2C 0x58) oder der Touchteil im AXS15231B (I2C 0x3B). Welcher verbaut
// ist, wird beim Start automatisch erkannt.
#pragma once

#include <stdint.h>

namespace board {

enum class TouchChip : uint8_t { None, CST3xx, CST3530, AXS15231B };

struct TouchPoint {
    int16_t x = 0; // 0..179  (Panelkoordinaten, Hochformat)
    int16_t y = 0; // 0..639
};

bool touchBegin();
TouchChip touchChip();
const char *touchChipName();

// true, solange ein Finger auf dem Panel liegt.
bool touchRead(TouchPoint &point);

// Achsen spiegeln, falls die Einbaulage es verlangt.
void touchSetInvert(bool invertX, bool invertY);

} // namespace board

#include "desk/protocol.h"

#include <string.h>

namespace loctek {

uint16_t crc16Modbus(const uint8_t *data, size_t len) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; ++i) {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; ++bit) {
            crc = (crc & 1) ? static_cast<uint16_t>((crc >> 1) ^ 0xA001)
                            : static_cast<uint16_t>(crc >> 1);
        }
    }
    return crc;
}

size_t buildKeyFrame(uint16_t keys, uint8_t *out, size_t capacity) {
    if (capacity < kKeyFrameSize) {
        return 0;
    }
    out[0] = kFrameStart;
    out[1] = 0x06; // LEN..CRC_LO
    out[2] = kTypeKey;
    out[3] = static_cast<uint8_t>(keys & 0xFF);
    out[4] = static_cast<uint8_t>(keys >> 8);
    const uint16_t crc = crc16Modbus(&out[1], 4);
    out[5] = static_cast<uint8_t>(crc >> 8);
    out[6] = static_cast<uint8_t>(crc & 0xFF);
    out[7] = kFrameEnd;
    return kKeyFrameSize;
}

void Parser::reset() {
    state_ = State::WaitStart;
    idx_ = 0;
    expected_ = 0;
}

bool Parser::feed(uint8_t byte, Frame &frame) {
    switch (state_) {
    case State::WaitStart:
        if (byte == kFrameStart) {
            idx_ = 0;
            state_ = State::Length;
        }
        return false;

    case State::Length:
        // LEN zaehlt sich selbst + TYPE + PAYLOAD + 2 CRC-Bytes.
        if (byte < 4 || byte > sizeof(buf_)) {
            reset();
            return false;
        }
        buf_[0] = byte;
        idx_ = 1;
        expected_ = byte;
        state_ = State::Body;
        return false;

    case State::Body:
        if (idx_ < expected_) {
            buf_[idx_++] = byte;
            return false;
        }
        // Dieses Byte muss das Endebyte sein.
        state_ = State::WaitStart;
        if (byte != kFrameEnd) {
            return false;
        }
        {
            const uint8_t bodyLen = static_cast<uint8_t>(expected_ - 2);
            const uint16_t want =
                static_cast<uint16_t>(buf_[expected_ - 2] << 8) | buf_[expected_ - 1];
            if (crc16Modbus(buf_, bodyLen) != want) {
                ++crcErrors_;
                return false;
            }
            frame.type = buf_[1];
            frame.payloadLen = static_cast<uint8_t>(bodyLen - 2);
            if (frame.payloadLen > sizeof(frame.payload)) {
                frame.payloadLen = sizeof(frame.payload);
            }
            memcpy(frame.payload, &buf_[2], frame.payloadLen);
            ++framesOk_;
            return true;
        }
    }
    return false;
}

namespace {
struct SegmentMap {
    uint8_t segments;
    char ch;
};

// Uebliche 7-Segment-Kodierung (Bit0 = a ... Bit6 = g, Bit7 = Dezimalpunkt).
constexpr SegmentMap kSegmentMap[] = {
    {0x00, ' '}, {0x3F, '0'}, {0x06, '1'}, {0x5B, '2'}, {0x4F, '3'}, {0x66, '4'},
    {0x6D, '5'}, {0x7D, '6'}, {0x07, '7'}, {0x7F, '8'}, {0x6F, '9'}, {0x77, 'A'},
    {0x7C, 'b'}, {0x39, 'C'}, {0x5E, 'd'}, {0x79, 'E'}, {0x71, 'F'}, {0x3D, 'G'},
    {0x76, 'H'}, {0x30, 'I'}, {0x1E, 'J'}, {0x38, 'L'}, {0x54, 'n'}, {0x5C, 'o'},
    {0x73, 'P'}, {0x50, 'r'}, {0x78, 't'}, {0x3E, 'U'}, {0x1C, 'u'},
    {0x6E, 'y'}, {0x40, '-'}, {0x08, '_'},
};
} // namespace

char decodeSegment(uint8_t segments) {
    const uint8_t s = segments & 0x7F;
    for (const SegmentMap &m : kSegmentMap) {
        if (m.segments == s) {
            return m.ch;
        }
    }
    return 0;
}

bool decodeDisplay(const uint8_t *segments, DisplayValue &out) {
    out = DisplayValue{};

    int decimalAfter = -1;
    for (int i = 0; i < 3; ++i) {
        const char ch = decodeSegment(segments[i]);
        if (ch == 0) {
            return false;
        }
        out.text[i] = ch;
        if (segments[i] & 0x80) {
            decimalAfter = i;
        }
    }
    out.text[3] = '\0';

    // Rein numerisch? Dann Zahlenwert samt Dezimalpunkt bilden.
    int digits = 0;
    long raw = 0;
    for (int i = 0; i < 3; ++i) {
        const char ch = out.text[i];
        if (ch == ' ') {
            continue; // fuehrende Leerstelle, z.B. " 75"
        }
        if (ch < '0' || ch > '9') {
            return true; // gueltiger Text, aber keine Zahl (z.B. "ASr")
        }
        raw = raw * 10 + (ch - '0');
        ++digits;
    }
    if (digits == 0) {
        return true;
    }

    float value = static_cast<float>(raw);
    if (decimalAfter >= 0 && decimalAfter < 2) {
        for (int i = decimalAfter; i < 2; ++i) {
            value /= 10.0f;
        }
    }
    out.numeric = true;
    out.value = value;
    return true;
}

} // namespace loctek

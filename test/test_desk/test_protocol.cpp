// Host-Tests fuer Protokoll und Ablauflogik:  pio test -e native
#include <unity.h>

#include <stdio.h>
#include <string.h>
#include <vector>

#include "desk/desk_controller.h"
#include "desk/loctek_protocol.h"

using namespace loctek;

// ---------------------------------------------------------------- Helfer ---

static std::vector<uint8_t> hex(const char *s) {
    std::vector<uint8_t> out;
    int hi = -1;
    for (const char *p = s; *p; ++p) {
        int v;
        if (*p >= '0' && *p <= '9') {
            v = *p - '0';
        } else if (*p >= 'a' && *p <= 'f') {
            v = *p - 'a' + 10;
        } else if (*p >= 'A' && *p <= 'F') {
            v = *p - 'A' + 10;
        } else {
            continue;
        }
        if (hi < 0) {
            hi = v;
        } else {
            out.push_back(static_cast<uint8_t>((hi << 4) | v));
            hi = -1;
        }
    }
    return out;
}

// 7-Segment-Kodierung fuer eine Ziffer.
static uint8_t seg(char digit, bool dot = false) {
    static const uint8_t table[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66,
                                      0x6D, 0x7D, 0x07, 0x7F, 0x6F};
    uint8_t v = table[digit - '0'];
    if (dot) {
        v |= 0x80;
    }
    return v;
}

// Baut ein Hoehen-Frame der Steuerbox (Typ 0x12).
static std::vector<uint8_t> heightFrame(uint8_t a, uint8_t b, uint8_t c) {
    std::vector<uint8_t> f = {kFrameStart, 0x07, kTypeDisplay, a, b, c};
    const uint16_t crc = crc16Modbus(&f[1], 5);
    f.push_back(static_cast<uint8_t>(crc >> 8));
    f.push_back(static_cast<uint8_t>(crc & 0xFF));
    f.push_back(kFrameEnd);
    return f;
}

// ------------------------------------------------------- Protokoll-Tests ---

static void expectKeyFrame(uint16_t keys, const char *expected) {
    uint8_t out[kKeyFrameSize];
    const size_t len = buildKeyFrame(keys, out, sizeof(out));
    TEST_ASSERT_EQUAL_UINT32(kKeyFrameSize, len);
    const std::vector<uint8_t> want = hex(expected);
    TEST_ASSERT_EQUAL_UINT32(want.size(), len);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(want.data(), out, len);
}

// Referenzframes aus https://github.com/iMicknl/LoctekMotion_IoT
void test_key_frames_match_reference(void) {
    expectKeyFrame(KEY_NONE, "9b 06 02 00 00 6c a1 9d");
    expectKeyFrame(KEY_UP, "9b 06 02 01 00 fc a0 9d");
    expectKeyFrame(KEY_DOWN, "9b 06 02 02 00 0c a0 9d");
    expectKeyFrame(KEY_PRESET_1, "9b 06 02 04 00 ac a3 9d");
    expectKeyFrame(KEY_PRESET_2, "9b 06 02 08 00 ac a6 9d");
    expectKeyFrame(KEY_PRESET_3, "9b 06 02 10 00 ac ac 9d");
    expectKeyFrame(KEY_PRESET_4, "9b 06 02 00 01 ac 60 9d");
    expectKeyFrame(KEY_M, "9b 06 02 20 00 ac b8 9d");
}

void test_parser_accepts_valid_frame(void) {
    Parser p;
    Frame f;
    const std::vector<uint8_t> in = heightFrame(seg('7'), seg('3', true), seg('5'));
    bool got = false;
    for (uint8_t b : in) {
        got = p.feed(b, f);
    }
    TEST_ASSERT_TRUE(got);
    TEST_ASSERT_EQUAL_HEX8(kTypeDisplay, f.type);
    TEST_ASSERT_EQUAL_UINT8(3, f.payloadLen);
    TEST_ASSERT_EQUAL_UINT32(1, p.framesOk());
    TEST_ASSERT_EQUAL_UINT32(0, p.crcErrors());
}

void test_parser_resyncs_after_garbage(void) {
    Parser p;
    Frame f;
    const std::vector<uint8_t> in = heightFrame(seg('1'), seg('0'), seg('0'));
    std::vector<uint8_t> stream = {0x00, 0xFF, 0x9B, 0x02, 0x13};
    stream.insert(stream.end(), in.begin(), in.end());

    int frames = 0;
    for (uint8_t b : stream) {
        if (p.feed(b, f)) {
            ++frames;
        }
    }
    TEST_ASSERT_EQUAL_INT(1, frames);
}

void test_parser_rejects_bad_crc(void) {
    Parser p;
    Frame f;
    std::vector<uint8_t> in = heightFrame(seg('7'), seg('5'), seg('0'));
    in[6] ^= 0xFF;
    bool got = false;
    for (uint8_t b : in) {
        got = p.feed(b, f) || got;
    }
    TEST_ASSERT_FALSE(got);
    TEST_ASSERT_EQUAL_UINT32(1, p.crcErrors());
}

void test_decode_height_with_decimal_point(void) {
    const uint8_t s[3] = {seg('7'), seg('3', true), seg('5')};
    DisplayValue v;
    TEST_ASSERT_TRUE(decodeDisplay(s, v));
    TEST_ASSERT_TRUE(v.numeric);
    TEST_ASSERT_EQUAL_STRING("735", v.text);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 73.5f, v.value);
}

void test_decode_height_without_decimal_point(void) {
    const uint8_t s[3] = {seg('1'), seg('1'), seg('5')};
    DisplayValue v;
    TEST_ASSERT_TRUE(decodeDisplay(s, v));
    TEST_ASSERT_TRUE(v.numeric);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 115.0f, v.value);
}

void test_decode_text_code(void) {
    // "ASr" -- Anzeige nach dem Reset, keine Hoehe.
    const uint8_t s[3] = {0x77, 0x6D, 0x50};
    DisplayValue v;
    TEST_ASSERT_TRUE(decodeDisplay(s, v));
    TEST_ASSERT_FALSE(v.numeric);
    TEST_ASSERT_EQUAL_STRING("A5r", v.text);
}

// ------------------------------------------------------ Controller-Tests ---

namespace {

// Simulierte Steuerbox: nimmt Tastenframes an, faehrt eine Hoehe und
// antwortet mit Hoehenframes.
class FakeDesk : public desk::Io {
  public:
    float height = 70.0f;
    bool wakeLine = false;
    std::vector<uint16_t> keysSeen;

    int available() override { return static_cast<int>(rx.size()) - rxPos; }
    int read() override {
        if (rxPos >= static_cast<int>(rx.size())) {
            return -1;
        }
        return rx[rxPos++];
    }
    void write(const uint8_t *data, size_t len) override {
        for (size_t i = 0; i < len; ++i) {
            Frame f;
            if (parser.feed(data[i], f) && f.type == kTypeKey && f.payloadLen >= 2) {
                lastKeys = static_cast<uint16_t>(f.payload[0] | (f.payload[1] << 8));
                keysSeen.push_back(lastKeys);
            }
        }
    }
    void setWakeLine(bool high) override { wakeLine = high; }

    // Ein Simulationsschritt: Motor bewegen und Hoehe melden.
    void tick(float stepCm) {
        if (lastKeys & KEY_UP) {
            height += stepCm;
        } else if (lastKeys & KEY_DOWN) {
            height -= stepCm;
        }
        emitHeight();
    }

    void emitHeight() {
        const int raw = static_cast<int>(height * 10.0f + 0.5f);
        char buf[16];
        snprintf(buf, sizeof(buf), "%03d", raw > 999 ? raw / 10 : raw);
        const bool dot = raw <= 999;
        std::vector<uint8_t> f = heightFrame(seg(buf[0]), seg(buf[1], dot), seg(buf[2]));
        rx.insert(rx.end(), f.begin(), f.end());
    }

    void drain() {
        rx.clear();
        rxPos = 0;
    }

    uint16_t lastKeys = 0;

  private:
    Parser parser;
    std::vector<uint8_t> rx;
    int rxPos = 0;
};

} // namespace

void test_controller_wakes_and_sends_heartbeat(void) {
    FakeDesk fake;
    desk::Controller c(fake);
    uint32_t t = 1000;
    c.begin(t);

    // Ohne wake() geht nichts raus.
    for (int i = 0; i < 10; ++i) {
        c.poll(t += 10);
    }
    TEST_ASSERT_EQUAL_UINT32(0, fake.keysSeen.size());

    c.wake(t);
    for (int i = 0; i < 30; ++i) {
        c.poll(t += 10);
    }
    TEST_ASSERT_TRUE(fake.wakeLine);
    TEST_ASSERT_TRUE(fake.keysSeen.size() >= 3);
    TEST_ASSERT_EQUAL_HEX16(KEY_NONE, fake.keysSeen.front());
}

void test_controller_tracks_height(void) {
    FakeDesk fake;
    desk::Controller c(fake);
    uint32_t t = 1000;
    c.begin(t);
    c.wake(t);

    fake.height = 73.5f;
    fake.emitHeight();
    c.poll(t += 10);

    TEST_ASSERT_TRUE(c.heightKnown());
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 73.5f, c.heightCm());
}

void test_controller_move_to_reaches_target(void) {
    FakeDesk fake;
    desk::Controller c(fake);
    uint32_t t = 1000;
    c.begin(t);
    c.wake(t);

    fake.height = 70.0f;
    fake.emitHeight();
    c.poll(t += 10);

    c.moveTo(110.0f, t);
    for (int i = 0; i < 4000 && c.targetActive(); ++i) {
        t += 10;
        c.poll(t);
        if (i % 5 == 0) {
            fake.tick(0.2f); // ~4 cm/s
        }
    }

    TEST_ASSERT_FALSE(c.targetActive());
    TEST_ASSERT_EQUAL_INT(static_cast<int>(desk::MoveResult::Reached),
                          static_cast<int>(c.lastMoveResult()));
    TEST_ASSERT_FLOAT_WITHIN(1.5f, 110.0f, fake.height);
    TEST_ASSERT_EQUAL_HEX16(KEY_NONE, fake.lastKeys);
}

void test_controller_move_down_and_stall_detection(void) {
    FakeDesk fake;
    desk::Controller c(fake);
    uint32_t t = 1000;
    c.begin(t);
    c.wake(t);
    fake.height = 100.0f;
    fake.emitHeight();
    c.poll(t += 10);

    c.moveTo(70.0f, t);
    // Motor blockiert: Hoehe aendert sich nicht, es kommen nur Frames.
    for (int i = 0; i < 2000 && c.targetActive(); ++i) {
        t += 10;
        c.poll(t);
        if (i % 5 == 0) {
            fake.emitHeight();
        }
    }
    TEST_ASSERT_FALSE(c.targetActive());
    TEST_ASSERT_EQUAL_INT(static_cast<int>(desk::MoveResult::Stalled),
                          static_cast<int>(c.lastMoveResult()));
}

void test_controller_move_without_feedback_aborts(void) {
    FakeDesk fake;
    desk::Controller c(fake);
    uint32_t t = 1000;
    c.begin(t);
    c.wake(t);

    c.moveTo(110.0f, t); // Steuerbox meldet nie eine Hoehe
    for (int i = 0; i < 3000 && c.targetActive(); ++i) {
        c.poll(t += 10);
    }
    TEST_ASSERT_FALSE(c.targetActive());
    TEST_ASSERT_EQUAL_INT(static_cast<int>(desk::MoveResult::NoFeedback),
                          static_cast<int>(c.lastMoveResult()));
}

void test_controller_preset_is_a_tap(void) {
    FakeDesk fake;
    desk::Controller c(fake);
    uint32_t t = 1000;
    c.begin(t);
    c.preset(3, t);
    for (int i = 0; i < 20; ++i) {
        c.poll(t += 10);
    }
    TEST_ASSERT_EQUAL_HEX16(KEY_PRESET_3, fake.keysSeen.front());

    for (int i = 0; i < 60; ++i) {
        c.poll(t += 10);
    }
    TEST_ASSERT_EQUAL_HEX16(KEY_NONE, fake.lastKeys);
}

void test_controller_clamps_target_to_limits(void) {
    FakeDesk fake;
    desk::Controller c(fake);
    uint32_t t = 1000;
    c.begin(t);
    c.moveTo(300.0f, t);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, c.config().maxHeightCm, c.targetCm());
    c.moveTo(10.0f, t);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, c.config().minHeightCm, c.targetCm());
}

// ------------------------------------------------------------------ main ---

int main(int, char **) {
    UNITY_BEGIN();
    RUN_TEST(test_key_frames_match_reference);
    RUN_TEST(test_parser_accepts_valid_frame);
    RUN_TEST(test_parser_resyncs_after_garbage);
    RUN_TEST(test_parser_rejects_bad_crc);
    RUN_TEST(test_decode_height_with_decimal_point);
    RUN_TEST(test_decode_height_without_decimal_point);
    RUN_TEST(test_decode_text_code);
    RUN_TEST(test_controller_wakes_and_sends_heartbeat);
    RUN_TEST(test_controller_tracks_height);
    RUN_TEST(test_controller_move_to_reaches_target);
    RUN_TEST(test_controller_move_down_and_stall_detection);
    RUN_TEST(test_controller_move_without_feedback_aborts);
    RUN_TEST(test_controller_preset_is_a_tap);
    RUN_TEST(test_controller_clamps_target_to_limits);
    return UNITY_END();
}

void setUp(void) {}
void tearDown(void) {}

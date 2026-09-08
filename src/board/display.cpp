// Panel-Anbindung fuer den AXS15231B im QSPI-Modus.
//
// Die Initialisierungssequenz und der QSPI-Transferaufbau stammen aus den
// Beispielen von LilyGo (Xinyuan-LilyGO/T-Display-S3-Long, examples/lvgl_demo).
// Hier wird bewusst die einfache, synchrone Variante ohne eigene DMA-Queue
// benutzt: sie erlaubt partielles Neuzeichnen und damit LVGL-Softwarerotation.
#include "board/display.h"

#include <Arduino.h>
#include <esp_heap_caps.h>
#include <lvgl.h>

#include "board/pins.h"
#include "board/touch.h"
#include "driver/spi_master.h"

namespace board {
namespace {

struct LcdCmd {
    uint8_t cmd;
    uint8_t data[4];
    uint8_t len; // Bit7 = 200 ms warten, Bit6 = 20 ms warten
};

// Das Panel ist ab Werk konfiguriert; es genuegt Sleep-Out + Display-On.
const LcdCmd kInitSequence[] = {
    {0x28, {0x00}, 0x40}, // display off
    {0x10, {0x00}, 0x20}, // sleep in
    {0x11, {0x00}, 0x80}, // sleep out
    {0x29, {0x00}, 0x00}, // display on
};

constexpr uint8_t kCmdColumnAddress = 0x2A;
constexpr uint8_t kCmdRowAddress = 0x2B;
constexpr int kBacklightChannel = 0;
constexpr lv_coord_t kDrawBufferLines = 16;
constexpr uint8_t kTouchScrollLimit = 10;

spi_device_handle_t g_spi = nullptr;
lv_disp_draw_buf_t g_drawBuf;
lv_color_t *g_buf0 = nullptr;
lv_color_t *g_buf1 = nullptr;
lv_color_t *g_flushBuf = nullptr;
uint8_t g_brightness = 0;
bool g_asleep = false;

void sendCmd(uint8_t cmd, const uint8_t *data, uint32_t len) {
    spi_transaction_t t = {};
    t.flags = SPI_TRANS_MULTILINE_CMD | SPI_TRANS_MULTILINE_ADDR;
    t.cmd = 0x02;
    t.addr = static_cast<uint64_t>(cmd) << 8;
    if (len > 0) {
        t.tx_buffer = data;
        t.length = 8 * len;
    }
    digitalWrite(TFT_QSPI_CS, LOW);
    spi_device_polling_transmit(g_spi, &t);
    digitalWrite(TFT_QSPI_CS, HIGH);
}

void setAddressWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    const uint8_t col[4] = {static_cast<uint8_t>(x1 >> 8), static_cast<uint8_t>(x1),
                            static_cast<uint8_t>(x2 >> 8), static_cast<uint8_t>(x2)};
    const uint8_t row[4] = {static_cast<uint8_t>(y1 >> 8), static_cast<uint8_t>(y1),
                            static_cast<uint8_t>(y2 >> 8), static_cast<uint8_t>(y2)};
    sendCmd(kCmdColumnAddress, col, sizeof(col));
    sendCmd(kCmdRowAddress, row, sizeof(row));
}

void pushPixels(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data) {
    setAddressWindow(x, y, x + w - 1, y + h - 1);

    size_t remaining = static_cast<size_t>(w) * h;
    const uint16_t *p = data;
    bool first = true;

    while (remaining > 0) {
        size_t chunk = remaining > TFT_SEND_BUF_PIXELS ? TFT_SEND_BUF_PIXELS : remaining;

        spi_transaction_ext_t t = {};
        t.base.flags = SPI_TRANS_MODE_QIO;
        t.base.cmd = 0x32;
        t.base.addr = first ? 0x002C00 : 0x003C00; // memory write / continue
        t.base.tx_buffer = p;
        t.base.length = chunk * 16;

        if (!first) {
            digitalWrite(TFT_QSPI_CS, HIGH);
        }
        // Der AXS15231B braucht zwischen zwei QSPI-Fortsetzungen eine kurze
        // CS-Pause; die LilyGo-Referenz erzeugt sie ebenfalls.
        volatile int csGap = 0;
        for (int i = 0; i < 10; ++i) {
            csGap >>= 1;
        }
        (void)csGap;
        digitalWrite(TFT_QSPI_CS, LOW);
        spi_device_polling_transmit(g_spi, reinterpret_cast<spi_transaction_t *>(&t));

        first = false;
        remaining -= chunk;
        p += chunk;
    }
    digitalWrite(TFT_QSPI_CS, HIGH);
}

void panelInit() {
    pinMode(TFT_QSPI_CS, OUTPUT);
    pinMode(TFT_QSPI_RST, OUTPUT);
    digitalWrite(TFT_QSPI_CS, HIGH);

    digitalWrite(TFT_QSPI_RST, HIGH);
    delay(130);
    digitalWrite(TFT_QSPI_RST, LOW);
    delay(130);
    digitalWrite(TFT_QSPI_RST, HIGH);
    delay(300);

    spi_bus_config_t bus = {};
    bus.data0_io_num = TFT_QSPI_D0;
    bus.data1_io_num = TFT_QSPI_D1;
    bus.sclk_io_num = TFT_QSPI_SCK;
    bus.data2_io_num = TFT_QSPI_D2;
    bus.data3_io_num = TFT_QSPI_D3;
    bus.max_transfer_sz = (TFT_SEND_BUF_PIXELS * 16) + 8;
    bus.flags = SPICOMMON_BUSFLAG_MASTER | SPICOMMON_BUSFLAG_GPIO_PINS;

    spi_device_interface_config_t dev = {};
    dev.command_bits = 8;
    dev.address_bits = 24;
    dev.mode = 0;
    dev.clock_speed_hz = TFT_SPI_FREQUENCY;
    dev.spics_io_num = -1; // CS wird per GPIO gesteuert
    dev.flags = SPI_DEVICE_HALFDUPLEX;
    dev.queue_size = 4;

    ESP_ERROR_CHECK(spi_bus_initialize(TFT_SPI_HOST, &bus, SPI_DMA_CH_AUTO));
    ESP_ERROR_CHECK(spi_bus_add_device(TFT_SPI_HOST, &dev, &g_spi));

    for (const LcdCmd &c : kInitSequence) {
        sendCmd(c.cmd, c.data, c.len & 0x3F);
        if (c.len & 0x80) {
            delay(200);
        }
        if (c.len & 0x40) {
            delay(20);
        }
    }

}

void rounderCb(lv_disp_drv_t *, lv_area_t *area) {
    // Gleiche Geometrie fuer jede Rotationsausgabe: volle logische Breite,
    // an den Zeichenpuffer ausgerichtete Zeilenbaender.
    area->x1 = 0;
    area->x2 = kScreenWidth - 1;
    area->y1 = (area->y1 / kDrawBufferLines) * kDrawBufferLines;
    area->y2 = ((area->y2 / kDrawBufferLines) + 1) * kDrawBufferLines - 1;
    if (area->y2 >= kScreenHeight) {
        area->y2 = kScreenHeight - 1;
    }
}

void flushCb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color) {
    const uint16_t w = area->x2 - area->x1 + 1;
    const uint16_t h = area->y2 - area->y1 + 1;
    const size_t pixels = static_cast<size_t>(w) * h;
    memcpy(g_flushBuf, color, pixels * sizeof(lv_color_t));
    pushPixels(area->x1, area->y1, w, h, reinterpret_cast<uint16_t *>(g_flushBuf));
    lv_disp_flush_ready(drv);
}

void touchCb(lv_indev_drv_t *, lv_indev_data_t *data) {
    TouchPoint p;
    if (touchRead(p)) {
        data->state = LV_INDEV_STATE_PRESSED;
        data->point.x = p.x;
        data->point.y = p.y;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

} // namespace

bool displayBegin() {
    panelInit();

    ledcSetup(kBacklightChannel, 20000, 8);
    ledcAttachPin(TFT_BL, kBacklightChannel);
    ledcWrite(kBacklightChannel, 0);

    lv_init();

    // Partielles Neuzeichnen: zwei Streifenpuffer im internen DMA-faehigen RAM.
    // Die Pufferbreite entspricht der logischen LVGL-Breite vor der Rotation.
    const size_t pixels = kScreenWidth * kDrawBufferLines;
    const uint32_t caps = MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL;
    g_buf0 = static_cast<lv_color_t *>(heap_caps_malloc(pixels * sizeof(lv_color_t), caps));
    g_buf1 = static_cast<lv_color_t *>(heap_caps_malloc(pixels * sizeof(lv_color_t), caps));
    g_flushBuf = static_cast<lv_color_t *>(heap_caps_malloc(pixels * sizeof(lv_color_t), caps));
    if (g_buf0 == nullptr || g_buf1 == nullptr || g_flushBuf == nullptr) {
        log_e("LVGL-Zeichenpuffer konnten nicht allokiert werden");
        return false;
    }
    lv_disp_draw_buf_init(&g_drawBuf, g_buf0, g_buf1, pixels);

    static lv_disp_drv_t drv;
    lv_disp_drv_init(&drv);
    drv.hor_res = TFT_PANEL_WIDTH; // physisch, LVGL dreht auf 640x180
    drv.ver_res = TFT_PANEL_HEIGHT;
    drv.flush_cb = flushCb;
    drv.rounder_cb = rounderCb;
    drv.draw_buf = &g_drawBuf;
    drv.sw_rotate = 1;
    drv.rotated = LV_DISP_ROT_90;
    drv.full_refresh = 0;
    lv_disp_drv_register(&drv);

    static lv_indev_drv_t indev;
    lv_indev_drv_init(&indev);
    indev.type = LV_INDEV_TYPE_POINTER;
    indev.scroll_limit = kTouchScrollLimit;
    indev.read_cb = touchCb;
    lv_indev_drv_register(&indev);

    return true;
}

void displayLoop() { lv_timer_handler(); }

void setBrightness(uint8_t value) {
    g_brightness = value;
    ledcWrite(kBacklightChannel, value);
}

uint8_t brightness() { return g_brightness; }

void displaySleep() {
    if (g_asleep) {
        return;
    }
    g_asleep = true;
    ledcWrite(kBacklightChannel, 0);
    sendCmd(0x28, nullptr, 0); // display off
    sendCmd(0x10, nullptr, 0); // sleep in
}

void displayWake() {
    if (!g_asleep) {
        return;
    }
    g_asleep = false;
    sendCmd(0x11, nullptr, 0); // sleep out
    delay(120);
    sendCmd(0x29, nullptr, 0); // display on
    ledcWrite(kBacklightChannel, g_brightness);
    lv_obj_invalidate(lv_scr_act());
}

bool displayAsleep() { return g_asleep; }

} // namespace board

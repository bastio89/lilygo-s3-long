// Pinbelegung LilyGo T-Display-S3 Long (ESP32-S3R8, 180x640, AXS15231B QSPI).
// Quelle: examples/*/pins_config.h und schematic/T-Display-S3-Long-3.4-V1.0.pdf
// aus https://github.com/Xinyuan-LilyGO/T-Display-S3-Long
#pragma once

// --- Display (QSPI) ---------------------------------------------------------
#define TFT_QSPI_CS 12
#define TFT_QSPI_SCK 17
#define TFT_QSPI_D0 13
#define TFT_QSPI_D1 18
#define TFT_QSPI_D2 21
#define TFT_QSPI_D3 14
#define TFT_QSPI_RST 16
#define TFT_BL 1

#define TFT_SPI_HOST SPI2_HOST
#define TFT_SPI_MODE SPI_MODE0
#define TFT_SPI_FREQUENCY 32000000

// Physische Panelaufloesung (Hochformat). Die UI laeuft per LVGL-Rotation
// im Querformat 640x180, siehe board/display.cpp.
#define TFT_PANEL_WIDTH 180
#define TFT_PANEL_HEIGHT 640

// Maximale Pixelzahl pro QSPI-Transfer.
#define TFT_SEND_BUF_PIXELS (28800 / 2)

// --- Touch + I2C ------------------------------------------------------------
// Beide Boardrevisionen teilen sich diese Pins; der Controller ist entweder
// ein CST3xx (0x1A) oder der Touchteil des AXS15231B (0x3B).
#define TOUCH_I2C_SDA 15
#define TOUCH_I2C_SCL 10
#define TOUCH_IRQ 11
#define TOUCH_RST 2

// --- Taster -----------------------------------------------------------------
#define PIN_BUTTON_BOOT 0

// --- Schreibtisch (RJ45 zur Loctek-/Flexispot-Steuerbox) --------------------
// Freie GPIOs vom 2x15-Stiftleistenfeld P5. Bewusst nicht 38-41 (dort sitzt
// auf der SD-Variante des Boards der Kartenleser) und nicht 45/46 (Strapping).
#ifndef DESK_UART_RX_PIN
#define DESK_UART_RX_PIN 47 // <- RJ45 Pin 5 (Daten von der Steuerbox)
#endif
#ifndef DESK_UART_TX_PIN
#define DESK_UART_TX_PIN 48 // -> RJ45 Pin 6 (Daten zur Steuerbox)
#endif
#ifndef DESK_WAKE_PIN
#define DESK_WAKE_PIN 42 // -> RJ45 Pin 4 ("PIN 20", HIGH weckt die Box)
#endif
#define DESK_UART_BAUD 9600

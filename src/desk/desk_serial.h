// Arduino-Transport fuer desk::Controller: UART2 auf den freien GPIOs der
// Stiftleiste plus die PIN-20-Weckleitung der RJ45-Buchse.
#pragma once

#include <Arduino.h>
#include <HardwareSerial.h>

#include "board/pins.h"
#include "desk/desk_controller.h"

namespace desk {

class SerialIo : public Io {
  public:
    explicit SerialIo(HardwareSerial &port) : port_(port) {}

    void begin() {
        pinMode(DESK_WAKE_PIN, OUTPUT);
        digitalWrite(DESK_WAKE_PIN, LOW);
        port_.begin(DESK_UART_BAUD, SERIAL_8N1, DESK_UART_RX_PIN, DESK_UART_TX_PIN);
    }

    int available() override { return port_.available(); }
    int read() override { return port_.read(); }
    void write(const uint8_t *data, size_t len) override { port_.write(data, len); }
    void setWakeLine(bool high) override { digitalWrite(DESK_WAKE_PIN, high ? HIGH : LOW); }

  private:
    HardwareSerial &port_;
};

} // namespace desk

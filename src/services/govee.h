#pragma once

#include <Arduino.h>
#include <stdint.h>

#include <freertos/FreeRTOS.h>
#include <freertos/portmacro.h>
#include <freertos/task.h>

#include "services/service.h"

class NimBLEAdvertisedDevice;
class NimBLEScan;

namespace services {

struct GoveeData {
    bool valid = false;
    float temperatureC = 0.0f;
    float humidityPct = 0.0f;
    uint8_t batteryPct = 0;
    int16_t rssi = 0;
    char address[18] = "";
    uint32_t updatedMs = 0;
};

class GoveeService : public Service {
  public:
    const char *name() const override { return "AMS"; }
    void begin() override;
    void loop(uint32_t nowMs) override;
    void refresh() override;
    bool ready() const override;
    const char *statusText() const override;

    GoveeData snapshot() const;

  private:
    class ScanCallbacks;

    static void scanTaskEntry(void *context);
    void scanTask();
    void handleAdvertisement(NimBLEAdvertisedDevice *device);
    bool matchesAddress(const char *address) const;

    GoveeData data_;
    mutable portMUX_TYPE dataMux_ = portMUX_INITIALIZER_UNLOCKED;
    NimBLEScan *scan_ = nullptr;
    ScanCallbacks *callbacks_ = nullptr;
    TaskHandle_t scanTaskHandle_ = nullptr;
    volatile bool refreshRequested_ = false;
    bool initialized_ = false;
};

GoveeService &govee();

} // namespace services
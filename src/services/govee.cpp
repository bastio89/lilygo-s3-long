#include "services/govee.h"

#include <NimBLEDevice.h>

#include <string>

#include "config.h"
#include "services/govee_protocol.h"

namespace services {
namespace {

GoveeService g_govee;

bool looksLikeH5074Data(const std::string &data) {
    return (data.size() >= 2 && static_cast<uint8_t>(data[0]) == 0x88 &&
            static_cast<uint8_t>(data[1]) == 0xEC) ||
           (data.size() >= 7 && static_cast<uint8_t>(data[0]) == 0x00);
}

void logUndecodedAdvertisement(const char *address, NimBLEAdvertisedDevice *device,
                               uint8_t manufacturerCount) {
    for (uint8_t index = 0; index < manufacturerCount; ++index) {
        const std::string data = device->getManufacturerData(index);
        if (data.size() < 2) {
            continue;
        }

        // Suche nach Govee Company ID: 0xEC88 (little-endian: 0x88 0xEC) oder 0x0001 etc.
        uint8_t b0 = static_cast<uint8_t>(data[0]);
        uint8_t b1 = static_cast<uint8_t>(data[1]);
        if ((b0 == 0x88 && b1 == 0xEC) || (b0 == 0xEC && b1 == 0x88) || (b0 == 0x01 && b1 == 0x00)) {
            String hex;
            for (size_t i = 0; i < data.size() && i < 16; ++i) {
                char buf[4];
                snprintf(buf, sizeof(buf), "%02X ", static_cast<uint8_t>(data[i]));
                hex += buf;
            }
            log_i("Govee MFR candidate %s len=%u: [%s]", address, (unsigned)data.size(), hex.c_str());
        }
    }
}

} // namespace

class GoveeService::ScanCallbacks : public NimBLEAdvertisedDeviceCallbacks {
  public:
    explicit ScanCallbacks(GoveeService &owner) : owner_(owner) {}

    void onResult(NimBLEAdvertisedDevice *device) override {
        owner_.handleAdvertisement(device);
    }

  private:
    GoveeService &owner_;
};

GoveeService &govee() { return g_govee; }

void GoveeService::begin() {
    if (initialized_) {
        return;
    }

    NimBLEDevice::init("");
    scan_ = NimBLEDevice::getScan();
    if (scan_ == nullptr) {
        log_e("Govee-BLE-Scanner konnte nicht initialisiert werden");
        return;
    }

    callbacks_ = new ScanCallbacks(*this);
    scan_->setAdvertisedDeviceCallbacks(callbacks_, false);
    scan_->setActiveScan(true);
    scan_->setInterval(100);
    scan_->setWindow(99);
    initialized_ = true;

    const BaseType_t result = xTaskCreate(&GoveeService::scanTaskEntry, "goveeScan", 3072,
                                          this, 1, &scanTaskHandle_);
    if (result != pdPASS) {
        initialized_ = false;
        scanTaskHandle_ = nullptr;
        log_e("Govee-BLE-Scan-Task konnte nicht gestartet werden");
        return;
    }

    log_i("Govee H5074 BLE aktiv (aktiver Scan alle %lu s)",
          static_cast<unsigned long>(GOVEE_SCAN_PERIOD_MS / 1000UL));
}

void GoveeService::loop(uint32_t) {}

void GoveeService::refresh() { refreshRequested_ = true; }

bool GoveeService::ready() const {
    const GoveeData data = snapshot();
    return data.valid && static_cast<uint32_t>(millis() - data.updatedMs) <=
                             GOVEE_SENSOR_STALE_AFTER_MS;
}

const char *GoveeService::statusText() const {
    if (!initialized_) {
        return "BLE nicht bereit";
    }
    const GoveeData data = snapshot();
    if (!data.valid) {
        return "suche AMS-Sensor";
    }
    return ready() ? "verbunden" : "veraltet";
}

GoveeData GoveeService::snapshot() const {
    GoveeData copy;
    portENTER_CRITICAL(&dataMux_);
    copy = data_;
    portEXIT_CRITICAL(&dataMux_);
    return copy;
}

void GoveeService::scanTaskEntry(void *context) {
    static_cast<GoveeService *>(context)->scanTask();
    vTaskDelete(nullptr);
}

void GoveeService::scanTask() {
    const uint32_t scanDurationMs = static_cast<uint32_t>(GOVEE_SCAN_DURATION_SECONDS) * 1000UL;
    const uint32_t pauseMs = GOVEE_SCAN_PERIOD_MS > scanDurationMs
                                 ? GOVEE_SCAN_PERIOD_MS - scanDurationMs
                                 : 0;

    while (true) {
        if (scan_ != nullptr) {
            scan_->start(GOVEE_SCAN_DURATION_SECONDS, false);
            scan_->clearResults();
        }

        refreshRequested_ = false;
        uint32_t waitedMs = 0;
        while (waitedMs < pauseMs && !refreshRequested_) {
            const uint32_t stepMs = (pauseMs - waitedMs) > 250 ? 250 : (pauseMs - waitedMs);
            vTaskDelay(pdMS_TO_TICKS(stepMs));
            waitedMs += stepMs;
        }
    }
}

bool GoveeService::matchesAddress(const char *address) const {
    if (GOVEE_H5074_ADDRESS[0] == '\0') {
        return true;
    }
    return strcasecmp(address, GOVEE_H5074_ADDRESS) == 0;
}

void GoveeService::handleAdvertisement(NimBLEAdvertisedDevice *device) {
    if (device == nullptr) {
        return;
    }

    const std::string addressString = device->getAddress().toString();
    char address[sizeof(data_.address)] = {};
    snprintf(address, sizeof(address), "%s", addressString.c_str());

    if (!device->haveManufacturerData()) {
        return;
    }
    if (!matchesAddress(address)) {
        return;
    }

    const uint8_t manufacturerCount = device->getManufacturerDataCount();
    for (uint8_t index = 0; index < manufacturerCount; ++index) {
        const std::string manufacturerData = device->getManufacturerData(index);
        GoveeH5074Reading reading;
        if (!decodeGoveeH5074(reinterpret_cast<const uint8_t *>(manufacturerData.data()),
                               manufacturerData.size(), reading)) {
            continue;
        }

        const int16_t rssi = static_cast<int16_t>(device->getRSSI());
        portENTER_CRITICAL(&dataMux_);
        data_.valid = true;
        data_.temperatureC = reading.temperatureC;
        data_.humidityPct = reading.humidityPct;
        data_.batteryPct = reading.batteryPct;
        data_.rssi = rssi;
        snprintf(data_.address, sizeof(data_.address), "%s", address);
        data_.updatedMs = millis();
        portEXIT_CRITICAL(&dataMux_);

        log_i("Govee H5074: %.2f C, %.2f %% rF, Akku %u %% (RSSI %d, %s)",
              static_cast<double>(reading.temperatureC), static_cast<double>(reading.humidityPct),
              static_cast<unsigned>(reading.batteryPct), static_cast<int>(rssi), address);
        return;
    }

    logUndecodedAdvertisement(address, device, manufacturerCount);
}

} // namespace services
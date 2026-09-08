#pragma once

#include <Arduino.h>
#include <stdint.h>

#include <freertos/FreeRTOS.h>
#include <freertos/portmacro.h>
#include <mqtt_client.h>

#include "services/service.h"

namespace services {

struct BambuData {
    bool valid = false;
    char state[16] = "";
    char fileName[96] = "";
    uint8_t percent = 0;
    uint32_t remainingMinutes = 0;
    uint32_t currentLayer = 0;
    uint32_t totalLayers = 0;
    float nozzleTemperature = 0.0f;
    float bedTemperature = 0.0f;
    uint32_t errorCode = 0;
    uint32_t updatedMs = 0;
};

class BambuService : public Service {
  public:
    const char *name() const override { return "Drucker"; }
    void begin() override;
    void loop(uint32_t nowMs) override;
    void refresh() override;
    bool ready() const override;
    const char *statusText() const override;

    BambuData snapshot() const;
    bool configured() const;

  private:
    static esp_err_t mqttEventHandler(esp_mqtt_event_handle_t event);
    esp_err_t handleMqttEvent(esp_mqtt_event_handle_t event);
    void startClient();
    void handlePayload(const char *payload, size_t length);
    void setConnected(bool connected);

    BambuData data_;
    mutable portMUX_TYPE dataMux_ = portMUX_INITIALIZER_UNLOCKED;
    esp_mqtt_client_handle_t client_ = nullptr;
    String payloadBuffer_;
    char brokerUri_[128] = {};
    char reportTopic_[96] = {};
    char requestTopic_[96] = {};
    bool mqttConnected_ = false;
    uint32_t nextStartMs_ = 0;
};

BambuService &bambu();

} // namespace services
#include "services/bambu.h"

#include <ArduinoJson.h>

#include "config.h"
#include "core/network.h"

namespace services {
namespace {

constexpr uint32_t kRetryMs = 60000;
constexpr uint16_t kMqttBufferSize = 4096;
constexpr uint16_t kMqttOutputBufferSize = 1024;
constexpr char kMqttUsername[] = "bblp";
constexpr char kPushAll[] = "{\"pushing\":{\"sequence_id\":\"0\",\"command\":\"pushall\"}}";
constexpr char kBambuCaCertificate[] = R"bambu(-----BEGIN CERTIFICATE-----
MIIDZTCCAk2gAwIBAgIUV1FckwXElyek1onFnQ9kL7Bk4N8wDQYJKoZIhvcNAQEL
BQAwQjELMAkGA1UEBhMCQ04xIjAgBgNVBAoMGUJCTCBUZWNobm9sb2dpZXMgQ28u
LCBMdGQxDzANBgNVBAMMBkJCTCBDQTAeFw0yMjA0MDQwMzQyMTFaFw0zMjA0MDEw
MzQyMTFaMEIxCzAJBgNVBAYTAkNOMSIwIAYDVQQKDBlCQkwgVGVjaG5vbG9naWVz
IENvLiwgTHRkMQ8wDQYDVQQDDAZCQkwgQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IB
DwAwggEKAoIBAQDL3pnDdxGOk5Z6vugiT4dpM0ju+3Xatxz09UY7mbj4tkIdby4H
oeEdiYSZjc5LJngJuCHwtEbBJt1BriRdSVrF6M9D2UaBDyamEo0dxwSaVxZiDVWC
eeCPdELpFZdEhSNTaT4O7zgvcnFsfHMa/0vMAkvE7i0qp3mjEzYLfz60axcDoJLk
p7n6xKXI+cJbA4IlToFjpSldPmC+ynOo7YAOsXt7AYKY6Glz0BwUVzSJxU+/+VFy
/QrmYGNwlrQtdREHeRi0SNK32x1+bOndfJP0sojuIrDjKsdCLye5CSZIvqnbowwW
1jRwZgTBR29Zp2nzCoxJYcU9TSQp/4KZuWNVAgMBAAGjUzBRMB0GA1UdDgQWBBSP
NEJo3GdOj8QinsV8SeWr3US+HjAfBgNVHSMEGDAWgBSPNEJo3GdOj8QinsV8SeWr
3US+HjAPBgNVHRMBAf8EBTADAQH/MA0GCSqGSIb3DQEBCwUAA4IBAQABlBIT5ZeG
fgcK1LOh1CN9sTzxMCLbtTPFF1NGGA13mApu6j1h5YELbSKcUqfXzMnVeAb06Htu
3CoCoe+wj7LONTFO++vBm2/if6Jt/DUw1CAEcNyqeh6ES0NX8LJRVSe0qdTxPJuA
BdOoo96iX89rRPoxeed1cpq5hZwbeka3+CJGV76itWp35Up5rmmUqrlyQOr/Wax6
itosIzG0MfhgUzU51A2P/hSnD3NDMXv+wUY/AvqgIL7u7fbDKnku1GzEKIkfH8hm
Rs6d8SCU89xyrwzQ0PR853irHas3WrHVqab3P+qNwR0YirL0Qk7Xt/q3O1griNg2
Blbjg3obpHo9
-----END CERTIFICATE-----)bambu";
BambuService g_bambu;

void copyText(char *destination, size_t destinationSize, const char *source) {
    snprintf(destination, destinationSize, "%s", source == nullptr ? "" : source);
}

} // namespace

BambuService &bambu() { return g_bambu; }

void BambuService::begin() {
    portENTER_CRITICAL(&dataMux_);
    data_ = BambuData{};
    mqttConnected_ = false;
    portEXIT_CRITICAL(&dataMux_);
    client_ = nullptr;
    nextStartMs_ = 0;
}

void BambuService::loop(uint32_t nowMs) {
    if (!configured()) {
        return;
    }
    if (!core::online()) {
        setConnected(false);
        return;
    }
    if (client_ != nullptr || static_cast<int32_t>(nowMs - nextStartMs_) < 0) {
        return;
    }
    startClient();
}

void BambuService::refresh() {
    if (client_ != nullptr && mqttConnected_) {
        esp_mqtt_client_publish(client_, requestTopic_, kPushAll, 0, 0, 0);
    }
}

bool BambuService::ready() const {
    return snapshot().valid;
}

bool BambuService::configured() const {
    return BAMBU_PRINTER_IP[0] != '\0' && BAMBU_PRINTER_SERIAL[0] != '\0' &&
           BAMBU_ACCESS_CODE[0] != '\0';
}

const char *BambuService::statusText() const {
    if (!configured()) {
        return "nicht konfiguriert";
    }
    if (!core::online()) {
        return "WLAN offline";
    }

    bool connected = false;
    bool valid = false;
    portENTER_CRITICAL(&dataMux_);
    connected = mqttConnected_;
    valid = data_.valid;
    portEXIT_CRITICAL(&dataMux_);
    if (!connected) {
        return "Drucker offline";
    }
    return valid ? "verbunden" : "warte auf Daten";
}

BambuData BambuService::snapshot() const {
    BambuData copy;
    portENTER_CRITICAL(&dataMux_);
    copy = data_;
    portEXIT_CRITICAL(&dataMux_);
    return copy;
}

void BambuService::setConnected(bool connected) {
    portENTER_CRITICAL(&dataMux_);
    mqttConnected_ = connected;
    portEXIT_CRITICAL(&dataMux_);
}

void BambuService::startClient() {
    snprintf(brokerUri_, sizeof(brokerUri_), "mqtts://%s:8883", BAMBU_PRINTER_IP);
    snprintf(reportTopic_, sizeof(reportTopic_), "device/%s/report", BAMBU_PRINTER_SERIAL);
    snprintf(requestTopic_, sizeof(requestTopic_), "device/%s/request", BAMBU_PRINTER_SERIAL);

    esp_mqtt_client_config_t config = {};
    config.uri = brokerUri_;
    config.username = kMqttUsername;
    config.password = BAMBU_ACCESS_CODE;
    config.user_context = this;
    config.event_handle = mqttEventHandler;
    config.keepalive = 30;
    config.buffer_size = kMqttBufferSize;
    config.out_buffer_size = kMqttOutputBufferSize;
    config.task_stack = 4096;
    config.reconnect_timeout_ms = 10000;
    config.cert_pem = kBambuCaCertificate;
    config.skip_cert_common_name_check = true;

    client_ = esp_mqtt_client_init(&config);
    if (client_ == nullptr) {
        log_w("Bambu-MQTT konnte nicht initialisiert werden");
        nextStartMs_ = millis() + kRetryMs;
        return;
    }

    const esp_err_t startResult = esp_mqtt_client_start(client_);
    if (startResult != ESP_OK) {
        log_w("Bambu-MQTT konnte nicht gestartet werden: %s (%d)",
              esp_err_to_name(startResult), static_cast<int>(startResult));
        esp_mqtt_client_destroy(client_);
        client_ = nullptr;
        nextStartMs_ = millis() + kRetryMs;
    }
}

esp_err_t BambuService::mqttEventHandler(esp_mqtt_event_handle_t event) {
    if (event == nullptr || event->user_context == nullptr) {
        return ESP_OK;
    }
    return static_cast<BambuService *>(event->user_context)->handleMqttEvent(event);
}

esp_err_t BambuService::handleMqttEvent(esp_mqtt_event_handle_t event) {
    switch (event->event_id) {
    case MQTT_EVENT_CONNECTED:
        log_i("Bambu-MQTT verbunden");
        setConnected(true);
        esp_mqtt_client_subscribe(event->client, reportTopic_, 0);
        esp_mqtt_client_publish(event->client, requestTopic_, kPushAll, 0, 0, 0);
        break;
    case MQTT_EVENT_DISCONNECTED:
        log_w("Bambu-MQTT getrennt");
        setConnected(false);
        break;
    case MQTT_EVENT_ERROR:
        log_w("Bambu-MQTT Fehler");
        setConnected(false);
        break;
    case MQTT_EVENT_DATA:
        if (event->data == nullptr || event->data_len <= 0) {
            break;
        }
        if (event->current_data_offset == 0) {
            payloadBuffer_ = "";
            payloadBuffer_.reserve(event->total_data_len > 0 ? event->total_data_len
                                                              : kMqttBufferSize);
        }
        if (static_cast<int>(payloadBuffer_.length()) != event->current_data_offset) {
            payloadBuffer_ = "";
            break;
        }
        payloadBuffer_.concat(event->data, event->data_len);
        if (event->total_data_len <= 0 ||
            event->current_data_offset + event->data_len >= event->total_data_len) {
            handlePayload(payloadBuffer_.c_str(), payloadBuffer_.length());
            log_i("Bambu-Statusdaten empfangen (%u Bytes)",
                  static_cast<unsigned>(payloadBuffer_.length()));
            payloadBuffer_ = "";
        }
        break;
    default:
        break;
    }
    return ESP_OK;
}

void BambuService::handlePayload(const char *payload, size_t length) {
    JsonDocument document;
    if (deserializeJson(document, payload, length)) {
        return;
    }

    JsonVariant printValue = document["print"];
    if (printValue.isNull()) {
        printValue = document["pushall"]["print"];
    }
    JsonObject print = printValue.as<JsonObject>();
    if (print.isNull()) {
        return;
    }

    BambuData next = snapshot();
    next.valid = true;
    copyText(next.state, sizeof(next.state), print["gcode_state"] | "UNKNOWN");
    const char *fileName = print["subtask_name"] | "";
    if (fileName[0] == '\0') {
        fileName = print["gcode_file"] | "";
    }
    copyText(next.fileName, sizeof(next.fileName), fileName);
    next.percent = print["mc_percent"] | next.percent;
    next.remainingMinutes = print["mc_remaining_time"] | next.remainingMinutes;
    next.currentLayer = print["layer_num"] | next.currentLayer;
    next.totalLayers = print["total_layer_num"] | next.totalLayers;
    next.nozzleTemperature = print["nozzle_temper"] | next.nozzleTemperature;
    next.bedTemperature = print["bed_temper"] | next.bedTemperature;
    next.errorCode = print["print_error"] | next.errorCode;
    next.updatedMs = millis();

    portENTER_CRITICAL(&dataMux_);
    data_ = next;
    portEXIT_CRITICAL(&dataMux_);
}

} // namespace services
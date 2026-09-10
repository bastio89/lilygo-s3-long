#include "services/govee_protocol.h"

namespace services {

bool decodeGoveeH5074(const uint8_t *manufacturerData, size_t length,
                      GoveeH5074Reading &reading) {
    if (manufacturerData == nullptr) {
        return false;
    }

    const uint8_t *payload = manufacturerData;
    size_t payloadLength = length;
    if (length >= 2 && manufacturerData[0] == 0x88 && manufacturerData[1] == 0xEC) {
        payload += 2;
        payloadLength -= 2;
    }

    if (payloadLength != 7 || payload[0] != 0x00) {
        return false;
    }

    const uint16_t temperatureBits = static_cast<uint16_t>(payload[1]) |
                                     (static_cast<uint16_t>(payload[2]) << 8);
    const int16_t temperatureRaw = static_cast<int16_t>(temperatureBits);
    const uint16_t humidityRaw = static_cast<uint16_t>(payload[3]) |
                                 (static_cast<uint16_t>(payload[4]) << 8);
    const uint8_t batteryPct = payload[5];

    if (temperatureRaw < -4000 || temperatureRaw > 10000 || humidityRaw > 10000 ||
        batteryPct > 100) {
        return false;
    }

    reading.temperatureC = static_cast<float>(temperatureRaw) / 100.0f;
    reading.humidityPct = static_cast<float>(humidityRaw) / 100.0f;
    reading.batteryPct = batteryPct;
    return true;
}

} // namespace services
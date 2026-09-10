#pragma once

#include <stddef.h>
#include <stdint.h>

namespace services {

struct GoveeH5074Reading {
    float temperatureC = 0.0f;
    float humidityPct = 0.0f;
    uint8_t batteryPct = 0;
};

bool decodeGoveeH5074(const uint8_t *manufacturerData, size_t length,
                      GoveeH5074Reading &reading);

} // namespace services
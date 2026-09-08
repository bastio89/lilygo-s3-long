// Wetter von Open-Meteo (kein API-Schluessel noetig).
#pragma once

#include <stdint.h>

#include "services/service.h"

namespace services {

constexpr uint8_t kWeatherForecastDays = 3;

struct WeatherForecastDay {
  bool valid = false;
  char date[11] = {};
  float low = 0.0f;
  float high = 0.0f;
  int code = 0;
  int precipitationProb = 0;
};

struct WeatherData {
    bool valid = false;
    float temperature = 0.0f;  // Grad C
    float apparent = 0.0f;     // gefuehlt
    float windKmh = 0.0f;
    int humidity = 0;          // Prozent
    int code = 0;              // WMO-Wettercode
    float todayMin = 0.0f;
    float todayMax = 0.0f;
    int precipitationProb = 0; // Prozent
    WeatherForecastDay forecast[kWeatherForecastDays];
    uint32_t updatedMs = 0;
};

class WeatherService : public Service {
  public:
    const char *name() const override { return "Wetter"; }
    void begin() override;
    void loop(uint32_t nowMs) override;
    void refresh() override;
    bool ready() const override { return data_.valid; }
    const char *statusText() const override;

    const WeatherData &data() const { return data_; }

  private:
    bool fetch();

    WeatherData data_;
    uint32_t nextFetchMs_ = 0;
    bool pending_ = true;
    uint8_t failures_ = 0;
};

WeatherService &weather();

// Deutsche Kurzbeschreibung zum WMO-Code.
const char *weatherDescription(int code);
// Akzentfarbe (RGB) passend zur Wetterlage.
uint32_t weatherAccentColor(int code);

} // namespace services

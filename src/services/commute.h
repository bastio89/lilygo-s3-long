// Pendelroute per OSRM/OpenStreetMap.
#pragma once

#include <stdint.h>

#include "services/service.h"

namespace services {

enum class CommuteDirection : uint8_t {
    ToOffice,
    Home,
};

struct CommuteData {
    bool valid = false;
    uint32_t durationSeconds = 0;
    uint32_t distanceMeters = 0;
  uint32_t trafficDelaySeconds = 0;
  bool trafficKnown = false;
    uint32_t updatedMs = 0;
};

class CommuteService : public Service {
  public:
    const char *name() const override { return "Pendelzeit"; }
    void begin() override;
    void loop(uint32_t nowMs) override;
    void refresh() override;
    bool ready() const override { return data_.valid; }
    const char *statusText() const override;

    const CommuteData &data() const { return data_; }
    CommuteDirection direction() const { return direction_; }
    void setDirection(CommuteDirection direction);
    bool configured() const;
    const char *originName() const;
    const char *destinationName() const;

  private:
    bool fetch();

    CommuteData data_;
    CommuteDirection direction_ = CommuteDirection::ToOffice;
    uint32_t nextFetchMs_ = 0;
    bool pending_ = true;
    uint8_t failures_ = 0;
};

CommuteService &commute();

} // namespace services
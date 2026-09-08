#include "desk/presets.h"

#include <stdio.h>
#include <string.h>

namespace desk {

PresetTable::PresetTable() { resetToDefaults(); }

void PresetTable::resetToDefaults() {
    for (uint8_t i = 0; i < kPresetCount; ++i) {
        presets_[i] = Preset{};
        presets_[i].mode = PresetMode::BoxMemory;
        presets_[i].boxSlot = static_cast<uint8_t>(i + 1);
        presets_[i].heightCm = 0.0f;
        presets_[i].name[0] = '\0';
    }
}

Preset &PresetTable::at(uint8_t index) {
    return presets_[index < kPresetCount ? index : 0];
}

const Preset &PresetTable::at(uint8_t index) const {
    return presets_[index < kPresetCount ? index : 0];
}

bool PresetTable::apply(uint8_t index, FlexiSpot &desk, uint32_t nowMs) {
    if (index >= kPresetCount) {
        return false;
    }
    const Preset &p = presets_[index];
    if (p.mode == PresetMode::TargetHeight) {
        desk.moveTo(p.heightCm, nowMs);
    } else {
        desk.preset(p.boxSlot, nowMs);
    }
    return true;
}

bool PresetTable::capture(uint8_t index, const FlexiSpot &desk) {
    if (index >= kPresetCount || !desk.heightKnown()) {
        return false;
    }
    presets_[index].mode = PresetMode::TargetHeight;
    presets_[index].heightCm = desk.heightCm();
    return true;
}

void PresetTable::label(uint8_t index, char *out, size_t capacity) const {
    if (capacity == 0) {
        return;
    }
    if (index >= kPresetCount) {
        out[0] = '\0';
        return;
    }
    const Preset &p = presets_[index];
    if (p.name[0] != '\0') {
        snprintf(out, capacity, "%s", p.name);
    } else if (p.mode == PresetMode::TargetHeight) {
        snprintf(out, capacity, "%.0f", static_cast<double>(p.heightCm));
    } else {
        snprintf(out, capacity, "%u", static_cast<unsigned>(p.boxSlot));
    }
}

} // namespace desk

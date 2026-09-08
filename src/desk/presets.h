// Speicherplaetze der Bedienoberflaeche.
//
// Ein Platz kann zweierlei bedeuten:
//   BoxMemory     -- den gleichnamigen Speicherplatz der Steuerbox ausloesen.
//                    Die Box faehrt dann selbst, wir sehen nur die Hoehe.
//   TargetHeight  -- eine hier hinterlegte Hoehe geregelt anfahren.
//
// Bewusst ohne Arduino-Abhaengigkeiten; das Ablegen im NVS erledigt
// core/settings.
#pragma once

#include <stddef.h>
#include <stdint.h>

#include "desk/flexispot.h"

namespace desk {

enum class PresetMode : uint8_t { BoxMemory = 0, TargetHeight = 1 };

constexpr uint8_t kPresetCount = 4;
constexpr size_t kPresetNameLen = 12;

struct Preset {
    PresetMode mode = PresetMode::BoxMemory;
    uint8_t boxSlot = 1; // 1..4, nur fuer BoxMemory
    float heightCm = 0.0f;
    char name[kPresetNameLen] = {};
};

class PresetTable {
  public:
    PresetTable();

    Preset &at(uint8_t index);
    const Preset &at(uint8_t index) const;

    // Loest den Platz aus. false, wenn der Index ungueltig ist.
    bool apply(uint8_t index, FlexiSpot &desk, uint32_t nowMs);

    // Uebernimmt die aktuelle Hoehe auf den Platz und schaltet ihn damit auf
    // TargetHeight. false, wenn keine Hoehe bekannt ist.
    bool capture(uint8_t index, const FlexiSpot &desk);

    // Beschriftung fuer die Taste: Name, sonst Hoehe bzw. Platznummer.
    void label(uint8_t index, char *out, size_t capacity) const;

    void resetToDefaults();

  private:
    Preset presets_[kPresetCount];
};

} // namespace desk

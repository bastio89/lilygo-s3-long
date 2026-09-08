// Wetter von Open-Meteo (kein API-Schluessel noetig).
#pragma once

#include <stdint.h>

namespace weather {

struct Data {
    bool valid = false;
    float temperature = 0.0f;    // Grad C
    float apparent = 0.0f;       // gefuehlt
    float windKmh = 0.0f;
    int humidity = 0;            // Prozent
    int code = 0;                // WMO Wettercode
    float todayMin = 0.0f;
    float todayMax = 0.0f;
    int precipitationProb = 0;   // Prozent
    uint32_t updatedMs = 0;
};

void begin();
// Regelmaessig aufrufen; holt selbstaendig neue Daten.
void loop(uint32_t nowMs);
void requestRefresh();

const Data &data();
// Deutsche Kurzbeschreibung zum WMO-Code.
const char *description(int code);
// Akzentfarbe (RGB) passend zur Wetterlage -- fuer den farbigen Punkt in der UI.
uint32_t accentColor(int code);

} // namespace weather

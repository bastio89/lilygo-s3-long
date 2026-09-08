// Uhrzeit per NTP, formatiert fuer die Anzeige.
//
// Die Datei heisst bewusst nicht time.h: src/ liegt im Include-Pfad, ein
// time.h dort wuerde das <time.h> der Standardbibliothek verdecken.
#pragma once

#include <stdint.h>

namespace core {

void clockBegin();
void clockLoop(uint32_t nowMs);

// true, sobald mindestens einmal per NTP synchronisiert wurde.
bool clockSynced();

const char *clockText(); // "13:45"
const char *dateText();  // "Mo, 8. Sep"

} // namespace core

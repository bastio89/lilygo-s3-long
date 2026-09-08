// Gemeinsame Basis fuer Hintergrunddienste (Wetter, spaeter Kalender, ...).
//
// Ein Dienst holt periodisch Daten und stellt sie der UI bereit. Neue Dienste
// muessen nur von Service erben und sich in main.cpp registrieren -- Start und
// Takt uebernimmt die Registry.
#pragma once

#include <stddef.h>
#include <stdint.h>

namespace services {

class Service {
  public:
    virtual ~Service() = default;

    virtual const char *name() const = 0;
    virtual void begin() {}
    virtual void loop(uint32_t nowMs) {}

    // Liegen brauchbare Daten vor?
    virtual bool ready() const { return false; }
    // Kurze Zustandsbeschreibung fuer die Einstellungsseite.
    virtual const char *statusText() const { return ready() ? "ok" : "keine Daten"; }
    // Erzwingt beim naechsten loop() einen Abruf.
    virtual void refresh() {}
};

constexpr size_t kMaxServices = 8;

bool registerService(Service &service);
void beginAll();
void loopAll(uint32_t nowMs);
void refreshAll();

size_t count();
Service *at(size_t index);

} // namespace services

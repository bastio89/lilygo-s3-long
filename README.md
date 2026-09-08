# Schreibtischsteuerung auf dem LilyGo T-Display-S3 Long

Firmware, die das Handbedienteil eines höhenverstellbaren **Flexispot**-Tisches
(Steuerbox **HCB103A-1**, Loctek) durch das Touchdisplay eines
**LilyGo T-Display-S3 Long** ersetzt — und den Platz auf den 640 x 180 Pixeln
gleich mit Uhrzeit und Wetter füllt.

```
 ┌─────────────────────────────────────────────────────────────────┐
 │ 13:45   Mo, 8. Sep                                        WLAN  │
 ├─────────────────────────────────────────────────────────────────┤
 │  ┌──────────────────┐  ┌────┐  ┌───────┐ ┌───────┐  ┌──┐        │
 │  │  73.5 cm         │  │ ▲  │  │   1   │ │   2   │  │  │        │
 │  │  bereit          │  ├────┤  ├───────┤ ├───────┤  │■ │        │
 │  └──────────────────┘  │ ▼  │  │   3   │ │   4   │  │  │        │
 │                        └────┘  └───────┘ └───────┘  └──┘        │
 └─────────────────────────────────────────────────────────────────┘
       ← wischen →   Schreibtisch · Wetter · System
```

## Stand

| | |
|---|---|
| Protokoll der Steuerbox | fertig, gegen die bekannten Referenzframes getestet |
| Fahrlogik (halten, Speicherplätze, geregeltes Anfahren) | fertig, mit Host-Tests |
| Display, Touch, LVGL-Oberfläche | fertig, **noch nicht auf echter Hardware verifiziert** |
| WLAN, Uhrzeit, Wetter (Open-Meteo) | fertig |
| Verkabelung zur Steuerbox | **muss noch gemessen und gelötet werden**, siehe [docs/hardware.md](docs/hardware.md) |

Alles außer der Verkabelung lässt sich schon jetzt aufspielen: ohne
angeschlossenen Tisch zeigt die Schreibtischseite einfach „Steuerbox meldet
nichts", Uhr und Wetter laufen.

## Loslegen

```bash
cp include/secrets.example.h include/secrets.h   # WLAN eintragen
# Ort und Fahrbereich in include/config.h anpassen

pio test -e native                 # Protokoll- und Ablauflogik prüfen
pio run  -e t-display-s3-long -t upload
pio device monitor
```

PlatformIO bringt Toolchain, LVGL 8.3 und ArduinoJson selbst mit. Die
Boarddefinition liegt in `boards/T-Display-Long.json`.

Beim ersten Anschließen an den Tisch hilft der Sniffer-Modus:

```bash
pio run -e t-display-s3-long -t upload \
  --project-option="build_flags=-DDESK_SNIFFER=1"
```

## Bedienung

* **▲ / ▼** — gedrückt halten, der Tisch fährt; loslassen, er stoppt. Genau
  wie am Originalbedienteil.
* **1 – 4** — Speicherplätze. Standardmäßig werden die Speicherplätze *der
  Steuerbox* ausgelöst.
* **Systemseite → „Geregelt fahren"** — schaltet auf eigene Zielhöhen um.
  Dann fährt die Firmware selbst auf den Zentimeter genau, und ein **langer
  Druck** auf eine der vier Tasten speichert die aktuelle Höhe darauf
  (bleibt im NVS erhalten).
* **■** — Not-Stopp.
* Seiten mit einem Wisch wechseln: Schreibtisch · Wetter · System.

## Aufbau

```
src/
├── board/    Panel (AXS15231B, QSPI), Touch (CST3xx oder AXS15231B), Backlight
├── desk/     Frameformat + CRC, Ablauflogik, Arduino-Transport
├── net/      WLAN, NTP, Wetter von Open-Meteo
├── ui/       LVGL-Oberfläche
└── main.cpp
test/         Host-Tests für desk/ (laufen ohne Hardware)
docs/         Verkabelung und Protokollbeschreibung
```

`src/desk/` ist bewusst frei von Arduino-Abhängigkeiten — Zeit und I/O kommen
von außen herein. Deshalb lässt sich die komplette Fahrlogik gegen einen
simulierten Tisch testen, ohne etwas zu flashen:

```
$ pio test -e native
14 test cases: 14 succeeded
```

## Sicherheit

Der Klemmschutz sitzt in der Steuerbox und bleibt unangetastet — ersetzt wird
nur das Bedienteil. Die Firmware sendet fortlaufend den aktuellen
Tastenzustand; fällt sie aus, sieht die Box „keine Taste" und der Tisch bleibt
stehen. Geregelte Fahrten brechen bei Blockade, Zeitüberschreitung oder
fehlender Höhenrückmeldung ab und fahren nie blind.

## Weiterführend

* [docs/hardware.md](docs/hardware.md) — Pinbelegung, RJ45-Verkabelung,
  Pegelwandlung, Stromversorgung, Inbetriebnahme Schritt für Schritt
* [docs/loctek-protokoll.md](docs/loctek-protokoll.md) — Frameaufbau, CRC,
  Tastenmaske, 7-Segment-Dekodierung

Vorarbeit von anderen, auf der dieses Projekt aufbaut:
[iMicknl/LoctekMotion_IoT](https://github.com/iMicknl/LoctekMotion_IoT) für das
Protokoll und [Xinyuan-LilyGO/T-Display-S3-Long](https://github.com/Xinyuan-LilyGO/T-Display-S3-Long)
für Panel-Initialisierung und Schaltplan.

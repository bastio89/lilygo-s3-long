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
       ← wischen →   Schreibtisch · Wetter · Einstellungen
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
* **Vier Speicherplätze** — jeder Platz ist einzeln eingestellt:
  * *Box-Platz 1–4*: löst den gleichnamigen Speicherplatz **der Steuerbox**
    aus, die Box fährt dann selbst.
  * *Zielhöhe*: die Firmware regelt selbst auf den hinterlegten Wert.
  Ein **langer Druck** auf eine Taste übernimmt die aktuelle Höhe und schaltet
  den Platz damit auf Zielhöhe. Alles landet im NVS und übersteht einen
  Neustart.
* **■** — Not-Stopp.
* Seiten mit einem Wisch wechseln: Schreibtisch · Wetter · Einstellungen.

Auf der Einstellungsseite: Helligkeit, Display-Ruhezeit, Fahrbereich,
Belegung der vier Plätze, Touch-Spiegelung für die Einbaulage — und darunter
eine Diagnosezeile mit WLAN, erkanntem Touchcontroller, gezählten Frames und
CRC-Fehlern. Das ist beim Anschließen an die Steuerbox das wichtigste
Werkzeug.

## Aufbau

```
src/
├── core/       Netz, Einstellungen (NVS), Zeit
├── ui/         Dashboard + Seiten (Schreibtisch, Wetter, Einstellungen)
├── desk/       Protokoll der Steuerbox, Fahrlogik, Speicherplätze
├── services/   Hintergrunddienste hinter einer gemeinsamen Basisklasse
├── board/      Panel (AXS15231B), Touch, Backlight — die Hardwareschicht
└── main.cpp
test/           Host-Tests für desk/ (laufen ohne Hardware)
docs/           Verkabelung und Protokollbeschreibung
```

`desk/`, inklusive der Speicherplatzlogik, ist bewusst frei von
Arduino-Abhängigkeiten — Zeit und I/O kommen von außen herein. Deshalb lässt
sich alles gegen einen simulierten Tisch testen, ohne etwas zu flashen:

```
$ pio test -e native
19 test cases: 19 succeeded
```

Ein neuer Dienst (Kalender, Feinstaub, Zugverbindungen …) erbt von
`services::Service`, überschreibt `loop()` und wird in `main.cpp` mit
`services::registerService()` angemeldet — Start, Takt und die Statusanzeige
auf der Einstellungsseite laufen dann von selbst.

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

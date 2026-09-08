# Schreibtischsteuerung auf dem LilyGo T-Display-S3 Long

Firmware, die einen höhenverstellbaren **Flexispot**-Tisch (Steuerbox
**HCB103A-1**, Loctek) über das Touchdisplay eines **LilyGo T-Display-S3 Long**
bedient — und den Platz auf den 640 x 180 Pixeln gleich mit Uhrzeit und Wetter
füllt.

Das Display hängt an der freien **HS**-Buchse der Steuerbox. Die Tasten an der
Box selbst bleiben dabei funktionsfähig, das Display kommt also dazu und
ersetzt nichts.

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
| Verkabelung zur Steuerbox | **offen** — RJ45-Belegung ausmessen, Pegelwandler, siehe [docs/hardware.md](docs/hardware.md) |
| Höhenrückmeldung über HS | **noch unbestätigt** — die Box zeigt die Höhe in cm auf ihrem eigenen Display; ob sie sie auch über HS schickt, zeigt der Mithör-Modus |

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

Beim ersten Anschließen an den Tisch hilft der Mithör-Modus. Er gibt aus,
was von der Steuerbox hereinkommt — dekodierte Frames *und* eine Rohbilanz,
damit auch der Fall „es kommen Bytes an, aber keine gültigen Frames" sichtbar
wird:

```bash
pio run -e sniffer -t upload
pio device monitor
```

```
[roh] 216 Bytes | 24 Frames ok | 0 CRC-Fehler | zuletzt: 9B 07 12 07 CF 6D ...
[frame] typ=0x12 07 CF 6D -> Anzeige "735" = 73.5
```

## Bedienung

* **▲ / ▼** — gedrückt halten, der Tisch fährt; loslassen, er stoppt. Genau
  wie mit den Tasten an der Box.
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
20 test cases: 20 succeeded
```

Ein neuer Dienst (Kalender, Feinstaub, Zugverbindungen …) erbt von
`services::Service`, überschreibt `loop()` und wird in `main.cpp` mit
`services::registerService()` angemeldet — Start, Takt und die Statusanzeige
auf der Einstellungsseite laufen dann von selbst.

## Sicherheit

Der Klemmschutz sitzt in der Steuerbox und bleibt unangetastet — die Firmware
bedient sie nur so, wie es ein zusätzliches Bedienteil täte, und die
Originaltasten funktionieren weiter. Die Firmware sendet fortlaufend den aktuellen
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

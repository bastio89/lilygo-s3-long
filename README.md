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

### 1. PlatformIO installieren

Entweder **VS Code** mit der Erweiterung *PlatformIO IDE* (Extensions öffnen,
nach „PlatformIO IDE" suchen, installieren, VS Code neu starten) — oder nur
das Kommandozeilenwerkzeug:

```bash
pip install platformio
```

Beides zieht Toolchain, LVGL und ArduinoJson beim ersten Build selbst nach.
Das dauert einmalig ein paar Minuten und braucht etwa 1 GB.

### 2. Projekt holen und konfigurieren

```bash
git clone https://github.com/bastio89/lilygo-s3-long.git
cd lilygo-s3-long

cp include/secrets.example.h include/secrets.h   # WLAN eintragen
```

In `include/config.h` noch den Ort fürs Wetter setzen (`LOCATION_*`,
Koordinaten z. B. von [open-meteo.com](https://open-meteo.com/en/docs)).
Alles andere lässt sich später auf der Einstellungsseite ändern.

Ob die Logik stimmt, lässt sich schon ohne Hardware prüfen:

```bash
pio test -e native
```

### 3. Aufspielen

Board per **USB-C** anschließen — mit einem Datenkabel; reine Ladekabel haben
keine Datenadern und das Board taucht dann gar nicht auf.

```bash
pio run -e t-display-s3-long -t upload
pio device monitor
```

In VS Code stattdessen die Pfeil-Schaltfläche (→) unten links, danach das
Stecker-Symbol für die Konsole.

Danach sollte das Display angehen und die Schreibtischseite zeigen. Ohne
angeschlossenen Tisch steht dort „Steuerbox meldet nichts" — das ist richtig
so. Uhr und Wetter laufen, sobald das WLAN steht.

### Wenn der Upload nicht klappt

Der ESP32-S3 meldet sich über sein eingebautes USB an, ein extra Treiber ist
normalerweise nicht nötig (Windows zeigt „USB Serial Device (COMx)").

Wird trotzdem kein Port gefunden oder bricht der Upload ab, hilft der
Bootloader-Modus von Hand:

1. **BOOT** gedrückt halten
2. kurz **RST** drücken
3. **RST** loslassen
4. **BOOT** loslassen
5. Upload erneut starten, danach einmal **RST** für den Neustart

Geht das Display gar nicht an, den kleinen Schiebeschalter an der Boardkante
prüfen — der trennt den Akku ab.

### Mithör-Modus

Beim ersten Anschließen an den Tisch hilft ein eigenes Environment. Es gibt
aus, was von der Steuerbox hereinkommt — dekodierte Frames *und* eine
Rohbilanz, damit auch der Fall „es kommen Bytes an, aber keine gültigen
Frames" sichtbar wird:

```bash
pio run -e sniffer -t upload
pio device monitor
```

```
[roh] 216 Bytes | 24 Frames ok | 0 CRC-Fehler | zuletzt: 9B 07 12 07 CF 6D ...
[frame] typ=0x12 07 CF 6D -> Anzeige "735" = 73.5
```

Was die Ausgabe bedeutet, steht in
[docs/hardware.md](docs/hardware.md) — inklusive der Tabelle, die vom
Beobachteten auf die Ursache schließt.

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

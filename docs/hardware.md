# Hardware & Verkabelung

## 1. Das Display

**LilyGo T-Display-S3 Long** — ESP32-S3R8, 16 MB Flash, 8 MB PSRAM,
3,4" IPS mit 180 x 640 Pixeln am AXS15231B (QSPI), kapazitiver Touch,
SY6970-PMU mit Akkuanschluss.

Die UI läuft im Querformat 640 x 180. Das Panel selbst bleibt im Hochformat;
LVGL dreht per Software (`sw_rotate`, `LV_DISP_ROT_90`). Touchkoordinaten
werden von LVGL automatisch mitgedreht — der Treiber liefert deshalb
absichtlich rohe Panelkoordinaten (x 0…179, y 0…639).

### Belegte GPIOs

| Funktion | GPIO |
|---|---|
| LCD CS / SCK / D0 / D1 / D2 / D3 / RST | 12 / 17 / 13 / 18 / 21 / 14 / 16 |
| Backlight (PWM) | 1 |
| Touch I2C SDA / SCL / IRQ / RST | 15 / 10 / 11 / 2 |
| PMU SY6970 (I2C) | 15 / 10 |
| BOOT-Taster | 0 |

### Freie GPIOs

Auf der 2×15-Stiftleiste (P5, 1,27 mm) liegen laut Schaltplan:
`3, 4, 5, 6, 7, 8, 38, 39, 40, 41, 42, 43 (U0TXD), 44 (U0RXD), 45, 46, 47, 48`
sowie `GND`, `3V3` und `VBUS`.

> **Achtung:** Es gibt zwei Boardvarianten. Auf der Variante *mit* SD-Slot
> belegt der Kartenleser GPIO 38/39/40/41. GPIO 45/46 sind Strapping-Pins.
> Deshalb benutzt dieses Projekt **47, 48 und 42**.

### Zwei Touch-Revisionen

Je nach Fertigungslos sitzt der Touchcontroller entweder im AXS15231B selbst
(I2C `0x3B`) oder es ist ein separater CST3xx (I2C `0x1A`). `board/touch.cpp`
erkennt beim Start automatisch, welcher vorhanden ist — die Einstellungsseite
der UI zeigt das Ergebnis an.

## 2. Die Steuerbox

**Flexispot HCB103A-1** (Hersteller Loctek Ergonomic Technology). Angaben vom
Typenschild:

| | |
|---|---|
| Input | 29 V ⎓, 3,5 A |
| Output | 29 V ⎓, 1,8 A |
| Duty Cycle | **2 Minuten AN, 18 Minuten AUS** |

### Die vier Anschlüsse

Von links nach rechts, so wie sie auf dem Gehäuse beschriftet sind:

| Buchse | Bedeutung | belegt |
|---|---|---|
| **HS** | **Hand Switch** — hier hängt das Bedienteil. Das ist unser Anschluss. | frei |
| **DC IN** | Netzteil, 29 V | ja |
| **M1** | Motor 1 | ja |
| **M2** | Motor 2 (Zweimotorentische) | frei |

> **M1 und M2 führen die Motorspannung von 29 V.** Dort darf niemals etwas vom
> ESP32 angeschlossen werden. Sie sehen den Steckern von DC IN ähnlich (weißer
> 2×3-Einsatz im schwarzen Gehäuse) — HS ist der einzige Anschluss, der für
> Signale gedacht ist.

### Nur eine HS-Buchse

Manche Loctek-Boxen haben zwei Bedienteil-Anschlüsse, sodass Original und
Eigenbau parallel laufen können. **Die HCB103A-1 hat nur einen.** Das Display
*ersetzt* das Bedienteil also, es läuft nicht daneben.

Ein Y-Kabel wäre technisch möglich, ist aber keine gute Idee: beide Panels
senden fortlaufend auf derselben Leitung und würden sich gegenseitig stören.
Wer eine Rückfallebene will, legt sich lieber ein Verlängerungskabel bei, um
im Zweifel schnell wieder auf das Originalteil zu stecken.

### Belegung der HS-Buchse

**HCB103A-1 ist in den bekannten Projekten
([iMicknl/LoctekMotion_IoT](https://github.com/iMicknl/LoctekMotion_IoT))
nicht dokumentiert** — es gibt dazu nur einen offenen Feature-Request. Das
Rahmenprotokoll ist bei allen Loctek-Boxen gleich, die Pinbelegung des
Bedienteil-Anschlusses unterscheidet sich aber zwischen den Modellen. Vor dem
Anschließen also nachmessen (Schritt 3).

#### Falls HS eine RJ45-Buchse ist (HS13B-1 / HS01B-1)

| RJ45-Pin | Ader (T568B) | Funktion | ans Board |
|---|---|---|---|
| 1 | weiß-orange | RESET | frei lassen |
| 2 | orange | SWIM | frei lassen |
| 3 | weiß-grün | — | frei lassen |
| 4 | blau | PIN 20 (Wake) | GPIO 42 |
| 5 | weiß-blau | Daten Box → Panel | GPIO 47 (RX) |
| 6 | grün | Daten Panel → Box | GPIO 48 (TX) |
| 7 | weiß-braun | GND | GND |
| 8 | braun | +5 V | siehe Stromversorgung |

Bei der HS13A-1 (Flexispot EK5) ist die Reihenfolge eine andere
(Pin 2 = PIN 20, 3 = RX, 4 = TX, 5 = GND, 6 = +5 V, 7/8 = 29 V!). Deshalb
unbedingt erst messen.

> Falls nach dem Anschließen keine Daten ankommen, aber auch nichts warm wird:
> zuerst **Pin 5 und Pin 6 tauschen**. Das ist der häufigste Fehler und
> ungefährlich.

### Pegelwandlung — nicht überspringen

Die Steuerbox arbeitet mit **5 V TTL**. Der ESP32-S3 ist **nicht 5-V-tolerant**.

* **Box → ESP (RX, GPIO 47):** zwingend herunterteilen. Entweder ein
  bidirektionaler Levelshifter (z. B. TXS0108E) oder als Minimum ein
  Spannungsteiler 1 kΩ / 2 kΩ (5 V → 3,3 V).
* **ESP → Box (TX, GPIO 48):** 3,3 V werden von 5-V-Logik in aller Regel
  sicher als High erkannt, das kann direkt gehen. Sauberer ist auch hier der
  Levelshifter.
* **PIN 20 (GPIO 42):** wird vom ESP nur getrieben, 3,3 V genügen.

### Stromversorgung

Zwei Möglichkeiten:

1. **USB-C** — für Inbetriebnahme und Entwicklung. Immer damit anfangen.
2. **5 V von der Steuerbox** auf den `VBUS`-Pin der Stiftleiste — aber nur,
   wenn an der HS-Buchse tatsächlich 5 V anliegen (bei der RJ45-Variante
   Pin 8; erst messen). Das ist dieselbe Schiene, an der auch USB hängt, und
   speist die SY6970-PMU. Zu bedenken:
   * Der 5-V-Ausgang der Steuerbox ist für ein Bedienteil ausgelegt. Das
     Display zieht mit WLAN und voller Helligkeit **150–350 mA** — messen,
     bevor du dich darauf verlässt.
   * **Nicht gleichzeitig** USB und Steuerbox-5 V einspeisen, ohne die beiden
     Quellen zu entkoppeln (Schottky-Diode je Quelle).
   * Ist ein Akku angeschlossen, lädt die PMU ihn aus dieser Schiene mit.

## 3. Inbetriebnahme, Schritt für Schritt

1. **Messen, bevor du steckst.** Nur an der **HS**-Buchse messen, nie an
   M1/M2 (29 V). Passendes Breakout in HS stecken, Netzteil eingesteckt
   lassen und mit dem Multimeter alle Kontakte gegen Gehäusemasse durchgehen:
   * ein Kontakt liegt fest auf 0 V → GND,
   * einer auf ~5 V → Versorgung,
   * zwei liegen im Leerlauf auf ~5 V und brechen beim Tastendruck am
     Original-Bedienteil kurz ein → das sind die Datenleitungen (UART-Idle
     ist High),
   * findet sich irgendwo mehr als 5 V, sofort abbrechen — dann ist es nicht
     die Signalbuchse.
2. **Nur mithören.** Erst nur GND und RX (über Teiler) anschließen, TX und
   PIN 20 noch offen lassen. Firmware mit Sniffer bauen:

   ```bash
   pio run -e t-display-s3-long -t upload \
     --project-option="build_flags=-DDESK_SNIFFER=1"
   pio device monitor
   ```

   Am Handbedienteil eine Taste drücken — auf der Konsole müssen Frames mit
   `type=0x12` erscheinen. Die Einstellungsseite der UI zählt gültige Frames und
   CRC-Fehler mit; viele CRC-Fehler heißen: falscher Pin oder fehlende
   Pegelwandlung.
3. **Höhe prüfen.** Die große Zahl auf der Schreibtischseite muss der Anzeige
   am Original-Bedienteil entsprechen. Falls dort Zoll statt Zentimeter steht,
   ist die Steuerbox auf Zoll konfiguriert — dann `DESK_MIN_HEIGHT_CM` /
   `DESK_MAX_HEIGHT_CM` entsprechend interpretieren.
4. **Erst dann senden.** TX und PIN 20 anschließen und mit den Pfeiltasten
   testen. Finger auf der Taste = Tisch fährt, loslassen = Tisch stoppt.
5. **Fahrbereich eintragen.** Die tatsächlichen Endlagen ablesen und auf der
   Einstellungsseite unter „Fahrbereich min / max" eintragen (bzw. dauerhaft
   als `DESK_MIN_HEIGHT_CM` / `DESK_MAX_HEIGHT_CM` in `include/config.h`).
   Erst danach einen Speicherplatz auf „Zielhöhe" umstellen — vorher fehlt
   der Regelung die Begrenzung.

## 4. Sicherheit

* Der Klemmschutz sitzt in der Steuerbox und bleibt aktiv — die Firmware
  ersetzt nur das Bedienteil.
* `desk::FlexiSpot` bricht eine geregelte Fahrt ab, wenn sich die Höhe
  nicht mehr ändert (`Stalled`), wenn zu viel Zeit vergeht (`TimedOut`) oder
  wenn gar keine Höhe zurückgemeldet wird (`NoFeedback`). Es wird nie blind
  gefahren.
* Solange keine Taste gehalten wird, sendet die Firmware „keine Taste" —
  fällt sie aus oder stürzt ab, bleibt der Tisch stehen.
* Die HCB103A-1 hat nur **eine** HS-Buchse. Das Display ersetzt das
  Bedienteil, es läuft nicht daneben — für den Notfall das Originalteil
  griffbereit halten statt es parallel zu stecken.
* **Einschaltdauer beachten:** Das Typenschild nennt 2 Minuten AN auf
  18 Minuten AUS, also 10 %. Einzelne Fahrten sind harmlos (der Hub dauert
  wenige Sekunden), aber eine Automatik, die den Tisch minütlich bewegt,
  überlastet Motor und Steuerbox.

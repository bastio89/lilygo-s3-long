# Das serielle Protokoll der Loctek-/Flexispot-Steuerbox

UART, **9600 Baud, 8N1**, 5 V TTL. Bedienteil und Steuerbox tauschen
Frames gleichen Aufbaus aus.

## Frameaufbau

```
0x9B | LEN | TYPE | PAYLOAD … | CRC_HI | CRC_LO | 0x9D
```

* `LEN` zählt sich selbst, `TYPE`, die Payload und die beiden CRC-Bytes.
  Bei einem Tastenkommando mit 2 Byte Payload ist `LEN = 0x06`.
* `CRC` ist **CRC-16/MODBUS** (Polynom 0xA001 reflektiert, Init 0xFFFF)
  über **`LEN` bis einschließlich Payload** — das Startbyte gehört *nicht*
  dazu. Übertragen wird **High-Byte zuerst**.

Das ist der Punkt, an dem die meisten Nachbauten scheitern: die verbreiteten
Beschreibungen sagen nicht, dass das Längenbyte in die Prüfsumme eingeht.
`test/test_desk/test_protocol.cpp` rechnet alle acht bekannten Referenzframes
gegen die Implementierung nach.

## Bedienteil → Box: Tastenzustand (`TYPE = 0x02`)

Die Payload ist eine 16-Bit-Maske, little-endian:

| Bit | Wert | Taste |
|---|---|---|
| 0 | 0x0001 | Hoch |
| 1 | 0x0002 | Runter |
| 2 | 0x0004 | Speicherplatz 1 |
| 3 | 0x0008 | Speicherplatz 2 |
| 4 | 0x0010 | Speicherplatz 3 |
| 5 | 0x0020 | M (Speichern) |
| 8 | 0x0100 | Speicherplatz 4 |

Maske `0x0000` ist das „Wake Up"-Frame.

Daraus ergeben sich die bekannten Byte-Folgen:

| Kommando | Bytes |
|---|---|
| Wake Up | `9b 06 02 00 00 6c a1 9d` |
| Hoch | `9b 06 02 01 00 fc a0 9d` |
| Runter | `9b 06 02 02 00 0c a0 9d` |
| Platz 1 | `9b 06 02 04 00 ac a3 9d` |
| Platz 2 | `9b 06 02 08 00 ac a6 9d` |
| Platz 3 | `9b 06 02 10 00 ac ac 9d` |
| Platz 4 | `9b 06 02 00 01 ac 60 9d` |
| M | `9b 06 02 20 00 ac b8 9d` |

**Ein Tastendruck ist kein einzelnes Kommando, sondern ein gehaltener
Zustand.** Das echte Bedienteil sendet permanent, was gerade gedrückt ist —
auch „nichts". Genau das macht `desk::FlexiSpot`: solange die Klasse wach ist, geht
alle 100 ms ein Frame raus, und ein Zustandswechsel wird sofort gesendet
statt auf den nächsten Takt zu warten.

## Box → Bedienteil: Anzeigeinhalt (`TYPE = 0x12`)

Payload sind drei Bytes — die **7-Segment-Muster** der drei Ziffern, so wie
sie das Bedienteil anzeigen soll. Bit 0…6 sind die Segmente a…g, Bit 7 ist
der Dezimalpunkt.

| Muster | Zeichen | | Muster | Zeichen |
|---|---|---|---|---|
| 0x3F | 0 | | 0x7F | 8 |
| 0x06 | 1 | | 0x6F | 9 |
| 0x5B | 2 | | 0x77 | A |
| 0x4F | 3 | | 0x79 | E |
| 0x66 | 4 | | 0x50 | r |
| 0x6D | 5 | | 0x40 | − |
| 0x7D | 6 | | 0x00 | (leer) |
| 0x07 | 7 | | | |

Beispiel: `07` `CF` `6D` ergibt 7 / 3. / 5 und damit **73,5 cm**
(`CF` ist `4F` mit gesetztem Dezimalpunkt-Bit 0x80).

Nicht jede Anzeige ist eine Zahl. Die Box schickt hier auch Textcodes wie
`ASr` (nach einem Reset ist eine Referenzfahrt nötig), `LOC` (Tastensperre)
oder `E0x` (Fehler). `loctek::decodeDisplay()` liefert deshalb immer den
Text und nur bei rein numerischem Inhalt zusätzlich einen Zahlenwert.

## PIN 20

Pin 4 der RJ45-Buchse. HIGH weckt die Steuerbox und hält ihre serielle
Schnittstelle aktiv. Ohne dieses Signal antworten manche Boxen gar nicht.
Die Firmware zieht die Leitung nur so lange hoch, wie sie tatsächlich mit dem
Tisch spricht, damit die Box danach wieder schlafen kann.

## Quellen

* <https://github.com/iMicknl/LoctekMotion_IoT> — Referenzframes und
  RJ45-Belegungen der Modelle HS13B-1, HS13A-1, HS01B-1.
* <https://github.com/JulianKropp/loctekmotion_iot> — Fork mit
  Aderfarben-Tabelle.
* <https://github.com/iMicknl/LoctekMotion_IoT/issues/25> — offener
  Feature-Request für die hier verbaute **HCB103A-1**.

# Entwurf der Oberfläche

Quelldateien des Designs. Der Auftrag dazu steht in
[../docs/design-prompt.md](../docs/design-prompt.md).

```
build.mjs        erzeugt die Artboards aus einem gemeinsamen Grundgerüst
*.dc.html        je ein Artboard: 13 Gerätescreens à 640 × 180
                 plus das Systemblatt (Farbtoken, Schrift, Raster, Bausteine)
canvas.json      Anordnung auf der Leinwand samt Notizen zu jedem Artboard
```

Die Artboards werden generiert, nicht von Hand gepflegt — Statuszeile, Raster
und Bausteine stecken als gemeinsames CSS in `build.mjs`, damit eine Änderung
am System nicht dreizehnmal nachgezogen werden muss:

```bash
node design/build.mjs
```

Die daraus zusammengesetzte Leinwand (`smartdesk-designsystem.html`, rund
2,5 MB) ist ein Bauergebnis und liegt deshalb nicht im Repo.

## Was das Design festlegt

Farbtoken und Schriftstufen entsprechen dem, was `src/ui/theme.cpp` heute
schon benutzt — nachgeschärft, nicht ersetzt. Die acht Zeichensätze
(Regular 12/14/16, SemiBold 16/20/24/28/48) sind genau die Größen, die bereits
übersetzt werden.

Das Raster im Inhaltsbereich (640 × 152 unter der 28 px hohen Statuszeile):

| Zone | x | Breite |
|---|---|---|
| Fokuskachel | 8 | 244 |
| Haltetasten | 260 | 96 |
| Aktionen links | 364 | 108 |
| Aktionen rechts | 480 | 108 |
| Randstreifen | 596 | 36 |

Zeilen liegen auf `top: 8` und `top: 80`, jeweils 64 px hoch; eine Kachel über
die volle Höhe misst 136 px.

# Prompt für Claude Design

Diese Datei enthält den Auftrag, mit dem das visuelle Design der Oberfläche
entworfen wird. Beim Weiterentwickeln bitte hier nachziehen, damit spätere
Design-Runden von denselben Randbedingungen ausgehen.

---

Entwirf die Oberfläche für ein Touchdisplay, das an der Vorderkante eines
höhenverstellbaren Schreibtischs sitzt und ihn steuert. Ich brauche kein
einzelnes hübsches Bild, sondern ein **Designsystem**, in das sich später
beliebige weitere Seiten einfügen lassen, ohne dass es auseinanderfällt.

## Das Gerät

* **640 × 180 Pixel, Querformat.** Ein extrem breiter Streifen, Seitenverhältnis
  gut 3,5:1. Das ist die prägende Eigenschaft — Layouts aus der App-Welt
  funktionieren hier nicht, alles ordnet sich nebeneinander statt untereinander.
* 3,4" IPS, also rund 5,7 Pixel pro Millimeter. Kein OLED: Schwarz ist ein
  dunkles Grau, sehr feine Kontraste verschwinden.
* Kapazitiver Touch, ein Finger, keine Gesten außer Wischen und Tippen.

## Wie es benutzt wird

* Montiert an der Tischkante, **von oben in spitzem Winkel** betrachtet,
  Abstand 60–80 cm.
* Wechselnde Raumhelligkeit, von Sonne bis Abendlicht. Die Helligkeit ist
  regelbar, aber der Entwurf muss auch bei gedimmtem Display lesbar bleiben.
* Der häufigste Griff dauert zwei Sekunden: hinschauen, Höhe ablesen, Taste
  drücken, weiterarbeiten. **Auf einen Blick erfassbar** ist wichtiger als
  hübsch. Der Rest der Zeit hängt das Display einfach da und sollte nicht
  nerven.
* Hoch und Runter werden **gedrückt gehalten**, solange der Tisch fahren soll —
  wie echte Tasten. Das braucht sofortige, deutliche Rückmeldung und eine
  Fläche, die man ohne Hinsehen trifft.

## Was heute drin ist

Eine dauerhaft sichtbare **Statuszeile** (Uhrzeit, Datum, WLAN-Zustand) und
drei wischbare Seiten:

1. **Schreibtisch** — aktuelle Höhe in cm als dominierender Wert, ein
   Statustext darunter („bereit", „fährt hoch", „Ziel 112,0 cm",
   „blockiert – gestoppt"), Hoch und Runter zum Halten, vier
   Speicherplätze (beschriftbar, z. B. „Sitzen", „Stehen", „112"), ein
   Not-Stopp.
2. **Wetter** — Temperatur groß, Wetterlage als Text, dazu Tagesminimum und
   -maximum, gefühlte Temperatur, Luftfeuchte, Wind, Regenwahrscheinlichkeit,
   Ortsname.
3. **Einstellungen** — scrollbare Liste: Helligkeitsregler, Auswahlfeld für die
   Display-Ruhezeit, Fahrbereich mit −/+ Feldern, vier Zeilen für die
   Speicherplätze (Auswahlfeld plus −/+ plus Knopf „Jetzt"), zwei Schalter,
   ein Diagnoseblock mit sechs Zeilen Kleintext, ein Knopf
   „Werkseinstellungen".

## Worauf es mir wirklich ankommt: Erweiterbarkeit

Später sollen weitere Seiten dazukommen — **Spotify** (Titel, Interpret,
Fortschritt, Play/Pause/Weiter, Lautstärke), ein **Kalender** (nächste
Termine), vielleicht Abfahrtszeiten oder Raumklima. Der Entwurf muss das
tragen, ohne dass jede neue Seite eine Sonderlocke wird.

Entwirf deshalb ein **Baukastensystem**: ein Raster für den 640 × 180 breiten
Streifen, eine Handvoll wiederverwendbarer Bausteine (Kachel, große Taste,
Wertanzeige mit Einheit, Listenzeile, Regler, Statusanzeige) und klare Regeln,
wie eine neue Seite daraus zusammengesetzt wird. Zeig das System, indem du es
anwendest — nicht, indem du es beschreibst.

**Ein Problem, das du mitlösen sollst:** Bei drei Seiten ist Wischen in
Ordnung. Bei acht Seiten dauert es acht Wischer zurück zum Schreibtisch — und
genau der wird am häufigsten gebraucht. Denk dir dafür etwas aus. Ich habe
keine Vorgabe, aber ich will eine Antwort im Entwurf sehen.

## Technische Grenzen, die den Entwurf betreffen

Bitte nichts entwerfen, was sich auf dem Gerät nicht bauen lässt:

* **Farbtiefe 16 Bit (RGB565).** Große weiche Verläufe zeigen sichtbare
  Streifen. Flächige Farben oder sehr kurze Verläufe funktionieren, aufwendige
  Verlaufslandschaften nicht.
* Gezeichnet wird in Streifen, nicht als ganzes Bild. **Animationen kleiner
  Flächen sind gut** (ein Knopf, ein Balken, eine Zahl), bildschirmfüllende
  Übergänge, Weichzeichner oder Glaseffekte sind zu teuer.
* Weiche Schatten und sehr große Eckenradien kosten Rechenzeit — sparsam
  einsetzen, mit Absicht statt überall.
* **Schrift:** Als Grundlage steht Montserrat in den Größen 12 bis 48 zur
  Verfügung. Andere Schnitte und Größen sind möglich, kosten aber Flash und
  müssen einzeln erzeugt werden — halte dich also an eine überschaubare
  Skala und benenne sie.
* **Symbole:** Es gibt keine mitgelieferte Icon-Bibliothek. Du kannst Icons
  entwerfen, sie müssen dann aber als Zeichensatz erzeugt werden. Liste
  deshalb genau auf, welche Symbole du brauchst, und komm mit möglichst
  wenigen aus. Schrift ist oft die bessere Antwort als ein zweideutiges
  Piktogramm.
* Deutsche Beschriftungen mit korrekten Umlauten — der Zeichensatz wird
  entsprechend erzeugt.
* Keine Bilder aus dem Netz zur Laufzeit. Alles Grafische wird mit
  einkompiliert.

## Zustände nicht vergessen

Das Gerät ist die meiste Zeit nicht im Idealzustand. Entwirf mit:

* Höhe unbekannt, weil die Steuerbox schläft oder nichts meldet
* kein WLAN, dadurch kein Wetter und keine Uhrzeit
* Dienst lädt gerade / Abruf fehlgeschlagen
* Fehlermeldung der Steuerbox statt einer Zahl (dreistellige Codes wie „ASr")
* Tisch fährt gerade, mit Ziel und ohne

## Was ich als Ergebnis möchte

Artboards mit exakt **640 × 180** (Ausnahme: die Systemübersicht darf größer
sein):

1. **Designsystem** — Farbtoken mit Hex-Werten, Schriftskala, Abstandsraster,
   die Bausteine nebeneinander, Liste der benötigten Symbole
2. **Statuszeile** im Detail, samt Navigationslösung für viele Seiten
3. **Schreibtisch** — Ruhezustand
4. **Schreibtisch** — während der Fahrt, mit Zielhöhe
5. **Schreibtisch** — ohne Rückmeldung der Steuerbox
6. **Wetter**
7. **Einstellungen** — sichtbarer Ausschnitt der Liste
8. **Spotify** — der Beweis, dass das System trägt
9. **Kalender** — der zweite Beweis
10. **Nachtvariante**, falls dein Entwurf das braucht

Zu jedem Artboard ein, zwei Sätze, warum es so aufgebaut ist. Wo du dich
zwischen zwei Wegen entschieden hast, nenn den anderen kurz.

## Ton

Ruhig, sachlich, hoher Kontrast. Das Ding hängt acht Stunden am Tag im
Blickfeld — es soll etwas hermachen, wenn man hinsieht, und ansonsten still
sein. Keine verspielten Illustrationen, keine Schreibtisch-Piktogramme, keine
Verzierung ohne Aufgabe.

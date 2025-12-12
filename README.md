# CO₂-Sensor für Höhlen

![Build workflow status](https://github.com/keppler/co2/actions/workflows/cmake-avr.yml/badge.svg)

![fertiger CO₂-Sensor](https://github.com/keppler/co2/raw/master/docs/sensor-v2.webp)

## Hintergrund

Kohlenstoffdioxid (_CO₂_) spielt bei der Entstehung von Höhlen eine zentrale Rolle. In Wasser gelöst - als Kohlensäure
(_H₂CO₃_) - kann es Kalk aufnehmen und somit Hohlräume entstehen lassen. Anders herum wachsen Tropfsteine, wenn mit
Kalziumkarbonat gesättigte Kohlensäure in einem Hohlraum wieder _CO₂_ an die Umgebungsluft abgibt und sich der
überschüssige Kalk absetzt. Unterirdische Wasserläufe können unter bestimmten Bedingungen ebenfalls _CO₂_ einbringen,
auch biologische und chemische Prozesse können unter Tage _CO₂_ erzeugen.

## Problem

Kohlenstoffdioxid ist ein geruchs-, geschmacks- und farbloses Gas. Zu hohe Konzentrationen sind für den menschlichen
Körper lebensgefährlich. In einigen Höhlen können sich - je nach Bewetterung, Vegetation und Jahreszeit - durchaus
bedrohliche _CO₂_-Konzentrationen bilden. In den meisten betroffenen Höhlen ist eine eventuelle _CO₂_-Problematik
bekannt und in der Regel wird auf Höhlenplänen oder vor Ort darauf hingewiesen. Befährt man jedoch eine bis dato
unbekannte Höhle oder möchte die tatsächlichen Werte messen, ist es gut, ein entsprechendes Messgerät zu haben. :-)

Handelsübliche (günstige) _CO₂_-Messgeräte für die Raumluftqualität messen nur bis 2.000 oder 5.000 ppm (0,2% bzw. 0,5%).
In entsprechenden Höhlen sind Werte über 20.000 ppm (2%) aber keine Seltenheit. Rettungskräfte, Berg- und
Industriearbeiter nutzen professionelle _CO₂_-Warngeräte (z.B. _Dräger Pac® 8000_ oder _CO2Meter PRO-10_) - für Privatpersonen sind diese aber
schwer erhältlich und zudem ziemlich kostspielig. Der häufig durchgeführte "Flammentest" (mit Feuerzeug prüfen ob eine
Flamme erzeugt werden kann) ist viel zu ungenau und zu gefährlich - eine Flamme erlöscht meist erst unter 16% Sauerstoff,
was schon auf eine _CO₂_-Konzentration von >50.000 ppm hindeuten kann!

## Lösung

Die hier vorgestellte Schaltung soll eine günstige Messung der Kohlenstoffdioxid-Konzentration auch in hohen
Bereichen ermöglichen - zuverlässiger als ein "Flammen-Test", und dennoch bezahlbar.

> [!CAUTION]
> Der Sensor darf dennoch nur als **zusätzlicher** Indikator angesehen werden! Die üblichen Anzeichen eines zu hohen
> _CO₂_-Gehalts müssen unabhängig davon beobachtet werden. Machen Sie Ihr Leben nicht von der korrekten Funktion eines
> einzelnen Sensors abhängig!  
> Zudem ersetzt diese Schaltung unter keinen Umständen ein professionelles Messgerät, das regelmäßig gewartet und
> kalibriert wird!

### Der Sensor

In den letzten Jahren hat sich in der Entwicklung von _CO₂_-Sensoren viel getan - nicht zuletzt aufgrund der
Corona-Pandemie. Der Sensor [SCD41](https://sensirion.com/de/produkte/katalog/SCD41) der Schweizer _Sensirion AG_ nutzt
das photo-akustische NDIR-Verfahren, um auf kleinstem Raum (rund 10x10x6 mm) den Anteil von _CO₂_-Molekülen in der
Umgebungsluft zu bestimmen.
Sofern die Membran des Sensors nicht zu stark verschmutzt wird, ist der Sensor technisch bedingt äußerst lange haltbar.
Die regelmäßige Kalibrierung kann automatisch anhand des _CO₂_-Gehalts in freier Umgebung erfolgen (etwa 400 ppm).

### Der Mikrocontroller

Als Mikrocontroller zur Ansteuerung des Sensors und des Displays dient ein simpler [ATtiny85](https://www.microchip.com/en-us/product/attiny85). Dieser ist robust,
günstig, gut verfügbar und mit vergleichsweise einfachen Mitteln programmierbar. Nutzt man seinen Funktionsumfang
voll aus, kann dieser ohne weitere Hilfsmittel auch die Batteriespannung messen und benötigt im Standby weniger
Strom als die Selbstentladung der Batterie.

### Das Display

Zur Anzeige dient ein schlichtes OLED-Display vom Typ _SSD1306_. Auch dieses ist günstig, gut verfügbar und einfach
zu programmieren. Zudem ist das Display - wie der Sensor auch - über einen I²C-Bus ansteuerbar, was die Verdrahtung
dramatisch vereinfacht.

## Die Schaltung

### Variante "Minimalistisch"

Für den ersten Prototypen reicht eine simple Lochrasterplatine, da im Prinzip nur die Stromversorgung und der I²C-Bus
vom Controller zum Sensor und Display (parallel) durchverdrahtet werden müssen:

<!-- ![Bild des ersten Sensor-Prototypen im Einsatz](https://github.com/keppler/co2/raw/master/docs/sensor.webp) -->

![einfacher Prototyp](https://github.com/keppler/co2/raw/master/docs/prototype.webp)

### Variante "Komfort" (Version 1.3)

Die "Komfort-Schaltung" nutzt darüber hinaus noch die restlichen Pins, um einen Piezo-Piepser zur Alarmierung sowie
eine Taste zur Konfiguration zu integrieren. Hierfür wurde eine Platine gestaltet, um alle Komponenten ordentlich
zu platzieren.

![Schaltplan](https://github.com/keppler/co2/raw/master/docs/schematic.webp)

![Platine](https://github.com/keppler/co2/raw/master/docs/pcb.webp)

Die Platine kann per SMD oder "klassisch" mit Lötkolben bestückt werden.

![SMD-bestückte Platine](https://github.com/keppler/co2/raw/master/docs/pcb-assembled.webp)

### Bauteilliste

| Bezeichnung | Bauteil                           |
|-------------|-----------------------------------|
| U1          | ATtiny85                          |
| U2          | SCD41                             |
| Q1, Q2      | SI2302DS oder BS170               |
| R1          | Widerstand 100kΩ                  |
| R2          | Widerstand 47Ω                    |
| C1          | Kondensator 0.1µF                 |
| C2          | _entfällt_                        |
| DISPLAY     | 0,96" OLED-Display SSD1306 (I²C!) |
| BUZZER      | 12x8.5mm passive Buzzer Typ 12085 |

### Bezugsquellen

Nahezu alle Teile sind bei jedem gut sortierten Elektronikdistributor bestellbar, die meisten Teile lassen sich auch
über eBay oder AliExpress besorgen. Nur beim Sensor sollte man etwas aufpassen: bei AliExpress werden diese sowohl "lose"
als auch auf kleinen Breakout-Platinen für rund $20 verkauft. Ich habe ein halbes Dutzend solcher Sensoren von
verschiedenen AliExpress-Händlern gekauft - interessanterweise hatten alle _keine_ aufgelaserte Seriennummer. Es wird
vermutet, dass es sich hierbei um Exemplare handelt, die aus anderen Produkten/Schaltungen ausgebaut wurden. Auch diese
Sensoren lieferten plausible Daten (ich glaube also nicht dass das "Kopien" sind), aber da es kaum eine Ersparnis
gegenüber einem offiziellen Distributor gibt, würde ich da eher die Finger von lassen.

Die Platine kann man sich mit den hier im Repository hinterlegten Gerber-Dateien bei jedem beliebigen Auftragsfertiger
erstellen lassen, z.B. bei JLCPCB oder Elecrow.

Ich habe normalerweise immer die eine oder andere Platine "übrig" - ein offizieller Verkauf ist aufgrund der Elektronikregulierung
aber nicht möglich. Bei Interesse bitte kurze Mail an `klaus (at) fund-ev.de`.

## Die Software

Zum Compilieren werden die AVR-Toolchain und CMake benötigt. Alternativ kann ein fertig compiliertes Binary auch hier
aus dem Repository heruntergeladen werden.

Das Flashen erfolgt mit folgendem Befehl (ggf. angepasst an den verwendeten Programmer):

```console
avrdude -c usbasp -p t85 -P usb -v \
  -U lfuse:w:0x62:m -U hfuse:w:0xd7:m -U efuse:w:0xff:m \
  -U flash:w:co2-scd41.hex:i \
  -U eeprom:w:co2-scd41.eep:i
```

Aktuell belegt die Software mit 8.100 Bytes fast den gesamten verfügbaren Speicher, somit ist ohne größere Tricks
vorerst keine nennenswerte Erweiterung der Funktionalität möglich.
Anders formuliert: die verfügbaren Ressourcen werden optimal ausgenutzt. :-)

Die Programmierung kann "in system" erfolgen, auf der Rückseite der Platine sind Pads zum Anlöten oder für Pogo-Pins
vorbereitet.

## Bedienungsanleitung

Nach dem Anschluss an die Stromversorgung oder dem Wiedereinschalten per Taster startet der Sensor:
der Piezo-Piepser wird kurz getestet sowie die Batteriespannung ausgelesen und angezeigt.
Anschließend wechselt die Software in die automatische Messung. Die Daten
werden alle 5 Sekunden aktualisiert, der höchste gemessene CO₂-Wert wird dauerhaft angezeigt.  
Da stabile Messdaten erst nach etwa 90 Sekunden zur Verfügung stehen, werden die Daten in dieser Zeit zusammen mit
einem Countdown farblich abgesetzt ("ausgegraut") angezeigt.

Jedes Mal wenn der Sensor eine weitere 2.000 ppm-Schwelle überschreitet, gibt dieser ein akustisches Signal aus.
Ab 10.000 ppm piepst er dann je 2x, ab 20.000 ppm 3x und ab 24.000 ppm 4x. Wird eine Schwelle für mehr als 60 Sekunden um
mehr als 2.000 ppm wieder unterschritten, gibt es einen kurzen Ton zur "Entwarnung".

Der Button unterscheidet zwischen kurzer Betätigung (>50ms) und langer Betätigung (>1s). In den meisten Fällen wird ein
kurzer Drücker zur Auswahl und ein langer Drücker zur Bestätigung genutzt.

Aus der Messung heraus erreicht man über einen kurzen Drücker das Menü:

- **`AUTO-CALIB: [ON|OFF]`**: Auto-Kalibrierung ein-/ausschalten. Bei aktivierter Autokalibrierung geht der Sensor
  davon aus, dass der niedrigste innerhalb von sieben Tagen gemessene Wert einer CO₂-Konzentration von 400ppm entspricht.  
  **Es wird empfohlen, die Auto-Kalibrierung zu deaktiveren und den Sensor regelmäßig manuell zu kalibrieren!**
- **`FORCE CALIBRATE`**: Kalibrierung mit einer CO₂-Konzentration von 420ppm (Außenluft) erzwingen. Wichtig ist hierfür, dass der Sensor seit mindestens 3 Minuten durchgehend an der Außenluft betrieben wird.
- **`ALTITUDE: xxx`**: Einstellung der Höhe (verbessert die Genauigkeit der CO₂-Messung). Kann in 100m-Schritten von
  0-3000m eingestellt werden, die Einstellung wird dauerhaft im Sensor gespeichert.
- **`SELF TEST`**: Selbsttest des SCD41-Sensors ausführen. Dieser Vorgang dauert 10 Sekunden und sollte eigentlich immer `OK` zurückgeben.
- **`VOLUME`**: Einstellung der Piepser-Lautstärke (0=aus, 1=laut, 2=mittel, 3=leise; Standard=3).
- **`POWER OFF`**: Gerät ausschalten. Im Standby benötigt die Schaltung nur 210nA/0.2µA (das liegt weit unterhalb der Selbstentladung der Batterie). Mit einen langen Tastendruck kann man den Sensor wieder einschalten.
- **`BACK`**: zurück zur Messung (erfolgt ansonsten auch automatisch nach 10 Sekunden)

### Inbetriebnahme

Vor der ersten Verwendung sollten folgende Schritte durchgeführt werden:
- Auto-Kalibrierung deaktivieren
- barometrische Höhe einstellen
- Sensor mind. 3 Minuten an der Außenluft betreiben, anschließend manuelle Kalibierung durchführen


## Ausblick

Inspiriert von dieser Schaltung gibt es noch einen "großen Bruder": die Version 2 dieses Geräts kann die gemessenen
Daten bis zu 22 Stunden lang minütlich aufzeichnen und wahlweise per NFC oder Bluetooth Low Energy (BLE) an ein
Android-Smartphone übermitteln. Ein zusätzlich integrierter Sensor sorgt zudem für exakte Luftdruck-, Temperatur-
und Luftfeuchtigkeitsdaten, eine automatische barometrische Kompensation verbessert die Genauigkeit der
_CO₂_-Daten.  
Sowohl dessen Schaltung als auch die Software sind aber um ein Vielfaches komplexer. Bei Interesse bitte kurze Mail.

### Langzeitbetrieb / Datenlogger

Es kommt immer wieder mal die Frage auf, ob auch eine automatisierte _CO₂_-Messung über einen längeren Zeitraum möglich ist.

Das Problem hierbei ist inzwischen weniger die Stromversorgung als vielmehr der sogenannte "Drift" des Sensors.
Laut Hersteller sollte der SCD41 regelmäßig (am besten automatisch etwa einmal wöchentlich) kalibriert werden.

Es gibt aber auch photo-optische Sensoren mit einer separaten Messkammer zur Drift-Kompensation, wie z.B. den
[Sensirion SCD30](https://sensirion.com/de/produkte/katalog/SCD30) oder den
[E+E EE895](https://www.epluse.com/de/produkte/co2-messung/co2-module-und-fuehler/ee895/). Diese dürften für solche
Zwecke wesentlich besser geeignet sein. Hohe Luftfeuchtigkeit oder _CO₂_-Konzentrationen > 10.000 ppm können
aber auf Dauer auch ein Problem darstellen.

Ein Datenlogger-Prototyp ist derzeit in Vorbereitung - da aber auch Referenzmessungen über längere Zeiträume stattfinden
müssen wird das alles noch ein wenig dauern.

### Alternative Ansätze

Ein anderer Ansatz ist, ein ESP32-Board mit integriertem Display zu verwenden, z.B. ein _TTGO T-Display_.
Der Code kann mit der _Arduino IDE_ entwickelt und bequem geflashed werden, den Sensor schließt man direkt an die
Pins des Board an. Beispiele hierzu gibt es z.B. unter https://emariete.com/en/co2-meter-gadget/

Ein solches Board benötigt im Betrieb allerdings wesentlich mehr Strom als ein einzelner, kleiner Mikrocontroller.
Diese Variante ist also insbesondere dann interessant, wenn entweder Strom keine Rolle spielt (weil der Einsatz
auf wenige Stunden begrenzt ist), oder man bereits mit dem Arduino-Ökosystem vertraut ist.

### Weiterführende Informationen

- Smith, Garry K. (2003): Carbon Dioxide in Limestone Caves and its Effect on Cavers.  
  British Cave Research Association, Speleology 3 pp10-12.
  https://bcra.org.uk/pub/docs/downloads.html?f=sp003010.f
- Bad Air in Caves  
  https://www.youtube.com/watch?v=hUKdhmzOqtQ


## ToDo

Stand: 03.11.2025

- [ ] 3D-Druckvorlage für Gehäuse bereitstellen

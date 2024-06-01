# co2-Sensor für Höhlen

![Bild des ersten Sensor-Prototypen im Einsatz](https://github.com/keppler/co2/raw/master/sensor.webp)

## Hintergrund

Regenwasser kann bei der Versickerung _CO₂_ aus dem Boden aufnehmen. Je aktiver die Vegetation, desto höher ist oft die mögliche _CO₂_-Konzentration (die Zersetzung organischer Substanzen läuft praktisch "rückwärts" zur Photosynthese).

Das mit Kohlendioxid angereicherte Wasser (Kohlensäure - _H₂CO₃_) löst auf seinem Weg nach unten Kalk auf. Tritt so eine kalkgesättigte Lösung an einer Höhlendecke aus, wird bei passendem _CO₂_-Partialdruck sowohl das gelöste Kalk wieder freigegeben (was dann "Tropfsteine" bildet), als auch _CO₂_ wieder freigesetzt.

Da _CO₂_ schwerer ist als Luft, setzt es sich in Bodennähe ab. Besonders an tiefen und nicht bewetterten Stellen in Höhlen können sich so _CO₂-Sümpfe_ mit erheblicher Kohlendioxid-Konzentration bilden.

## Problem

Kohlendioxid ist ein geruchs-, geschmacks- und farbloses Gas. Zu hohe Konzentrationen sind für den menschlichen Körper lebensgefährlich. In einigen Höhlen können sich - je nach Bewetterung, Vegetation und Jahreszeit - durchaus entsprechende _CO₂_-Konzentrationen bilden.

Handelsübliche _CO₂_-Messgeräte für die Raumluftqualität messen nur bis 2.000 oder 5.000 ppm (0,2% bzw. 0,5%). In einigen Höhlen in z.B. Südfrankreich oder wohl auch in der Schwäbischen Alb sind aber 20.000-25.000 ppm eher "normal". 

Ab welchem Wert genau es "gefährlich" wird, kann man nicht exakt beziffern - das hängt von vielen Faktoren ab, u.a. der Dauer der erhöhten Exponation. Auch setzen die üblichen Symptome unterschiedlich stark ein. Grundsätzlich sollte man ab 20.000 ppm aber aufmerksam werden, spätestens ab 30.000 ppm sehr vorsichtig.

## Lösung

Es gibt bereits sehr gute kommerzielle Geräte zur Messung der _CO₂_-Konzentration in der Raumluft, u.a. Feuerwehr, Höhlenrettung oder Kanalarbeiter sind damit ausgestattet. Diese sind allerdings schwer verfügbar (nur über Fachhandel zu beziehen) und/oder relativ teuer (zumindest für Privatpersonen).

Das hier vorgestellte Projekt stellt eine vergleichsweise günstige (~ 60 €), robuste und nahezu wartungsfreie Lösung dar. Es war ein ausdrückliches Ziel, dass finanzielle Gründe einen nicht davon abhalten, zumindest einen groben Richtwert für den _CO₂_-Gehalt in einer Höhle zu erhalten.

> [!CAUTION]
> Der Sensor darf dennoch nur als **zusätzlicher** Indikator angesehen werden! Die üblichen Anzeichen eines zu hohen _CO₂_-Gehalts müssen unabhängig davon beobachtet werden. Machen Sie Ihr Leben nicht von der korrekten Funktion eines einzelnen Sensors abhängig!

### Der Sensor

Kern des Messgerätes ist der [SCD41](https://sensirion.com/de/produkte/katalog/SCD41) der Sensirion AG. Er basiert auf dem _NDIR_-Verfahren: das Licht einer LED versetzt in einer kleinen Hitzekammer die in der Luft enthaltenen _CO₂_-Moleküle bei definierter Temperatur in Schwingung, und deren Resonanzfrequenz wird wiederum akustisch gemessen. Das Ganze findet auf kleinstem Raum statt (der Sensor hat eine Kantenlänge von etwa 10x10x6mm) und verbraucht angenehm wenig Strom (max. etwa 200mA für weniger als eine Sekunde während eines Messvorgangs).

Sofern die Membran des Sensors nicht zu stark verschmutzt wird, ist der Sensor technisch bedingt äußerst lange haltbar. Die regelmäßige Kalibrierung kann automatisch anhand des _CO₂_-Gehalts in freier Umgebung erfolgen (etwa 400 ppm). 

### Der Mikrocontroller

Als Mikrocontroller zur Ansteuerung des Sensors und des Displays dient ein simpler [ATtiny85](https://www.microchip.com/en-us/product/attiny85). Dieser ist günstig, gut verfügbar und mit vergleichsweise einfachen Mitteln programmierbar. Der Controller ist auch ziemlich stromsparend (es gäbe durchaus noch sparsamere Alternativen, aber bei dem Stromverbrauch des Sensors spielt das keine Rolle) und kommt in der Low-Voltage-Variante auch mit Spannungen deutlich unter 2V noch gut zurecht.

Last but not least kann der ATtiny85 ohne zusätzlichen Pin bzw. A/D-Wandler auch gleich noch die Spannung der angeschlossenen Batterie ermitteln.

### Das Display

Zur Anzeige kommt ein schlichtes OLED-Display vom Typ _SSD1306_ zum Einsatz. Auch dieses ist günstig, gut verfügbar und einfach zu programmieren. Zudem ist das Display - wie der Sensor auch - über einen I²C-Bus ansteuerbar, was die Verdrahtung dramatisch vereinfacht.

## Die Schaltung

### Variante "Minimalistisch"

Für den ersten Prototypen reichte eine simple Lochrasterplatine, da im Prinzip nur die Stromversorgung und der I²C-Bus vom Controller zum Sensor und Display (parallel) durchverdrahtet werden mussten.

### Variante "Komfort"

Da noch nicht alle Pins genutzt wurden, umfasst die "Komfort-Schaltung" noch einen Piezo-Piepser sowie eine Taste zur Quittierung bzw. Konfiguration.

-- ToDo: ATtiny-Schaltplan --

## Ausblick

Eine mögliche Erweiterung wäre noch ein kleiner Speicher und ein NFC-Modem, um den gemessenen _CO₂_-Pegel über die Zeit aufzuzeichnen und per NFC/RFID auslesbar zu machen.

Zudem ist eine Variante angedacht, welche auf einen Langzeitbetrieb ausgelegt ist (hier stellt die Kalibrierung aber durchaus noch eine Herausforderung dar).

# Cruise controll
Hoe maak je op basis van standaard onderdelen een cruise controll die met behulp van een servo de gasveer aantrekt en loslaat aan de hand van de gemeten snelheid.

### Benodigdheden

* Arduino Micro (Micro Pro kan ook)
* 30kg cm servo
* Spanningsregelaar 3A - 14V -> 6V
* remkabel fiets (versnellingskabel kan ook)
* Pick-up spoel (hall sensor is vogens mij beter)
* Twee knopjes op je richtingaanwijzer/dashboard met LEDJE

### Werking
Op het dashboard zit een knop. Deze schakeld de stroom aan/uit naar de cruiscontroll. Bij uitschakelen trekt de gasveer (die die brandstof toevoer regelt) altijd hard genoeg om het gas 'los te laten'.

Na inschakelen van de spanning, druk je tegelijkertijd een seconde op de twee knopjes. Dan staat de CruiseControll aan. Als je nu de Plus knop twee seconden vasthoud is de CC actief en gaat deze de gemeten snelheid die opgeslagen is bij indrukken van de Plusknop, proberen te behouden door de servo verder uit te draaien. 

Ik meet de sneheid met een pickup spoel die dicht bij de draaiende magneet op de cardan as zit.

Als je de rem of koppeling indruk zal zal de waarde op de betreffende pin van GND naar Hoog gaan (door de interne pullup weerstand van de arduino)

Aansluitschema
![](images/Aansluitschema.jpg)

### Arduino Pro Micro


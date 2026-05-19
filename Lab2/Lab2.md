# Lab2: Upravljanje potrošnjom energije mikrokontrolera (Arduino Uno)

## Sažetak

| Stavka | Odgovor |
| :--- | :--- |
| Platforma | Arduino Uno (ATmega328P) |
| Varijanta | A |
| Sleep mode | PowerDown / Standby / Idle |
| Buđenje | Eksterno (Gumb na Pin 2) / Tajmersko (Timer1) |
| Čuvanje stanja | Automatsko unutar SRAM-a (kod nastavlja nakon sna) |
| Debouncing | Softverski debouncing pomoću vremenskog praga |
| Wokwi link | [https://wokwi.com/projects/464447866134208513](https://wokwi.com/projects/464447866134208513) |

## 1. Opis sustava

Uz pomoć Wokwi simulacijskog okruženja i Arduino Uno mikrokontrolera, simuliramo i uspoređujemo režime mirovanja mikrokontrolera te njihov utjecaj na potrošnju energije. Sustav nakon izvršavanja definiranog glavnog zadatka ulazi u zadani režim mirovanja, te je konfiguriran tako da se vrati natrag u aktivno stanje nakon vanjskog (događajnog) ili tajmerskog (periodičkog) mehanizma buđenja. Definirani zadatak u ovoj implementaciji obuhvaća dva bljeska LED diode spojene na digitalni izlaz, nakon kojih mikrokontroler isključuje periferiju i započne mirovati radi uštede energije.

## 2. Razine režima mirovanja Arduino mikrokontrolera (LowPower.h biblioteka)

Korištenjem službene `<LowPower.h>` biblioteke, imamo izravan pristup energetskim registrima ATmega328P čipa. U kodu je isprobano:

* **PowerDown (`LowPower.powerDown()`)** – Najdublje stanje mirovanja. Isključuju se sistemski sat, glavni oscilator i gotovo sva periferija. Potrošnja energije pada na minimum. Sustav se iz ovog stanja može probuditi isključivo eksternim prekidom (gumb na pinu 2).

## 3. Korišteni načini buđenja sustava

### 3.1. Prekid tipkalom (Eksterno buđenje)
Kao eksterni prekid sustav koristi mehaničko tipkalo povezano na digitalni **pin 2**. Tipkalo koristi **FALLING** brid (pad napona s 5V na 0V pri pritisku prema masi) za aktivaciju prekida putem funkcije `attachInterrupt()`. Aktivacija ovog prekida budi sustav iz sna, poziva funkciju filtra te podiže zastavicu `isAwake` za izvedbu glavne funkcije (treptanje LED diode).

### 3.2. Prekid tajmerom (Periodičko buđenje)
Uz pomoć biblioteke `<TimerOne.h>`, konfiguriran je hardverski 16-bitni tajmer koji generira prekid svakih **5 sekundi** (`timerDuration`). Prilikom svakog tajmerskog prekida poziva se ISR
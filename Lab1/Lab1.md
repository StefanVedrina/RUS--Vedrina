# Lab1: Sustavi prekida i hardverski tajmeri na ESP32 mikrokontroleru

## Sažetak

| Stavka | Odgovor |
| :--- | :--- |
| Platforma | ESP32 (Xtensa 32-bit dvojezgreni) |
| Varijanta | A |
| Periferija | HC-SR04 ultrazvučni senzor, 2x LED, 2x Tipkalo |
| Prekidi | Eksterni (GPIO s FALLING bridom) i Hardverski Tajmer |
| Wokwi link | [https://wokwi.com/projects/464433723157599233(https://wokwi.com/projects/464433723157599233) |

## 1. Opis sustava

Uz pomoć Wokwi simulacijskog okruženja i ESP32 razvojne pločice, u okviru ove laboratorijske vježbe simuliramo asinkrone događaje i upravljanje vremenom u realnom vremenu. Sustav je dizajniran kao višezadaćni sustav s tri razine prioriteta koji reagira na vanjske podražaje (pritisak tipkala), mjeri fizikalne veličine iz okoline (udaljenost preko ultrazvučnog senzora) te periodički izvršava pozadinske zadatke (žmiganje LED diode) pomoću hardverskog tajmera.

## 2. Hijerarhija i prioritizacija zadataka

Sustav dijeli zadatke u tri strogo definirane razine prioriteta, koje se softverski provjeravaju unutar glavne `loop()` petlje preko prekidnih zastavica (`volatile bool` flags):

1. **Visoki prioritet (Hitno tipkalo):** Aktivira se pritiskom na prvo tipkalo. Sustav trenutno pali crvenu LED diodu i ispisuje kritično stanje na Serial monitor. Namijenjeno je simulaciji sigurnosnih ili hitnih prekida u sustavu.
2. **Srednji prioritet (Senzorsko mjerenje):** Aktivira se pritiskom na drugo tipkalo. Sustav pokreće mjerenje udaljenosti pomoću ultrazvučnog senzora HC-SR04, računa udaljenost u centimetrima te ispisuje podatke.
3. **Niski prioritet (Periodički tajmer):** Pokreće se automatski svakih nekoliko sekundi. Sustav invertira stanje zelene LED diode (žmiganje) i ispisuje informativnu poruku u pozadini.

## 3. Korišteni mehanizmi prekida

### 3.1. Eksterni hardverski prekidi (GPIO)
Tipkala za visoki i srednji prioritet povezana su na digitalne pinove **GPIO 12** i **GPIO 14**. Pinovi su konfigurirani u `INPUT_PULLUP` načinu rada kako bi napon bio stabilan na 3.3V dok tipkalo miruje. Prekidi su povezani preko funkcije `attachInterrupt()` i reagiraju na **FALLING** brid (trenutak pritiska tipkala kada napon pada na GND). Pripadajuće prekidne rutine (ISR) isključivo podižu zastavice `prekidVisokiAktivan` i `prekidSrednjiAktivan`.

### 3.2. Interni hardverski tajmer (ESP32 v3.x API)
Za razliku od klasičnih 8-bitnih Arduino ploča, ESP32 koristi napredne 64-bitne hardverske tajmere. U ovoj implementaciji korišten je novi, ažurirani ESP32 API za upravljanje tajmerima:
* `timerBegin(1000000)` konfigurira tajmer s frekvencijom od $1\text{ MHz}$ (takt od 1 mikrosekunde).
* `timerAttachInterrupt()` povezuje objekt tajmera s prekidnom funkcijom `onTimer()`.
* `timerAlarm()` postavlja alarm na vremensku vrijednost od **2.000.000 mikrosekundi (2 sekunde)** s omogućenim automatskim ponavljanjem (autoreload).

## 4. Opis implementacije i senzorskog podsustava

Vježba uspješno integrira **HC-SR04 ultrazvučni senzor** za mjerenje udaljenosti. Kada se detektira prekid srednjeg prioriteta, sustav šalje visokofrekventni okidni impuls trajanja $10\ \mu\text{s}$ na **TRIG** pin (GPIO 5). Senzor odašilje ultrazvučni val, a funkcija `pulseIn()` na **ECHO** pinu (GPIO 18) mjeri vrijeme (u mikrosekundama) koje je valu bilo potrebno da ode do prepreke i vrati se natrag.

Udaljenost se računa prema fizikalnoj formuli:
$$Udaljenost = \frac{Trajanje \times 0.034}{2}$$
gdje $0.034\ \text{cm/}\mu\text{s}$ predstavlja brzinu zvuka u zraku.

Izlazni indikatori su realizirani pomoću dvije LED diode:
* Crvena LED dioda na **GPIO 26** (vizualna signalizacija visokog prioriteta).
* Zelena LED dioda na **GPIO 27** (vizualna signalizacija rada tajmera).

## 5. Ograničenja simulacije

Wokwi simulacijsko okruženje izvrsno emulira logičke valne oblike i tajminge ESP32 mikrokontrolera, no nosi određena ograničenja:
* **Idealizirani ultrazvučni senzor:** U simulatoru udaljenost postavljamo ručno pomicanjem klizača (slidera) na grafičkom sučelju, što eliminira stvarni fizikalni šum, refleksiju valova i pogreške u mjerenju koje se javljaju na realnom sklopovlju.
* **Izvršavanje ISR-a:** Budući da se radi o simulaciji unutar web preglednika, stvarna brzina reakcije na prekid može minimalno odstupati u mikrosekundama u odnosu na pravi silicijski čip zbog opterećenja procesora računala na kojem se vrti preglednik.

## 6. Zaključak

Kroz ovu laboratorijsku vježbu uspješno je demonstriran koncept asinkronog upravljanja hardverskim prekidima i tajmerima na 32-bitnoj ESP32 arhitekturi pomoću novog v3.x API-ja. Sustav učinkovito rješava probleme prioritizacije u jednolitnoj arhitekturi koda zadržavajući prekidne rutine (ISR) iznimno kratkima i brzima, prebacujući tešku logiku obrade podataka i mjerenja unutar glavne `loop()` petlje.

## 7. Podzadatak: Analiza `volatile` varijabli i IRAM memorije

### 7.1. Nužnost ključne riječi `volatile`
Sve prekidne zastavice u kodu (`prekidVisokiAktivan`, `prekidSrednjiAktivan`, `prekidTajmerAktivan`) eksplicitno su deklarirane uz ključnu riječ `volatile`. To govori kompajleru (prevoditelju) da se vrijednost tih varijabli može promijeniti izvan uobičajenog tijeka programa (unutar hardverskog prekida). Bez ove ključne riječi, kompajler bi optimizirao kod i držao vrijednost varijable u brzom registru procesora, ignorirajući promjenu koja se dogodila u prekidu, što bi uzrokovalo da sustav nikada ne prepozna pritisak gumba ili okidanje tajmera.

### 7.2. Značaj prekidnih rutina u realnom vremenu
Prekidne rutine u ovom kodu ne sadrže teške matematičke operacije niti blokirajuće funkcije kao što su `delay()` ili `Serial.println()`. One isključivo podižu zastavicu na logičku istinu (`true`) i trenutno vraćaju kontrolu glavnom programu. Ovakav pristup jamči stabilnost sustava, sprječava gubljenje drugih hardverskih prekida i eliminira opasnost od aktivacije Watchdog tajmera koji bi u suprotnom srušio i resetirao ESP32 mikrokontroler.

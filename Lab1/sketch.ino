#include <Arduino.h>

// 1. Definiranje pinova
const int PIN_TIPKALO_VISOKI = 12;
const int PIN_TIPKALO_SREDNJI = 14;
const int PIN_LED_CRVENA = 26;
const int PIN_LED_ZELENA = 27;
const int PIN_TRIG = 5;
const int PIN_ECHO = 18;

// Zastavice
volatile bool prekidVisokiAktivan = false;
volatile bool prekidSrednjiAktivan = false;
volatile bool prekidTimerAktivan = false;

// Brojači
int brojacVisoki = 0;
int brojacSrednji = 0;

// Timer
hw_timer_t *mojTimer = NULL;

void isrVisokiPrioritet() {
  prekidVisokiAktivan = true;
}

void isrSrednjiPrioritet() {
  prekidSrednjiAktivan = true;
}

void onTimer() {
  prekidTimerAktivan = true;
}

void setup() {
  Serial.begin(115200);
  delay(500);

  // Postavljanje pinova za LED i senzor
  pinMode(PIN_LED_CRVENA, OUTPUT);
  pinMode(PIN_LED_ZELENA, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  // Tipkala s internim pull-up otpornikom
  pinMode(PIN_TIPKALO_VISOKI, INPUT_PULLUP);
  pinMode(PIN_TIPKALO_SREDNJI, INPUT_PULLUP);

  // Povezivanje vanjskih prekida za tipkala (FALLING)
  attachInterrupt(digitalPinToInterrupt(PIN_TIPKALO_VISOKI), isrVisokiPrioritet, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_TIPKALO_SREDNJI), isrSrednjiPrioritet, FALLING);

  mojTimer = timerBegin(1000000);

  // Dodavanje listenera na timer
  timerAttachInterrupt(mojTimer, &onTimer);

  // Timer na 2s
  timerAlarm(mojTimer, 2000000, true, 0);
}

void loop() {

  // 1. OBRADA VISOKOG PRIORITETA (Tipkalo 1)
  if (prekidVisokiAktivan) {
    prekidVisokiAktivan = false;
    brojacVisoki++;

    Serial.print("VISOKI PRIORITET - Hitno tipkalo! Broj pritisaka: ");
    Serial.println(brojacVisoki);

    digitalWrite(PIN_LED_CRVENA, HIGH);
    delay(300);
    digitalWrite(PIN_LED_CRVENA, LOW);
  }

  // 2. OBRADA SREDNJEG PRIORITETA (Tipkalo 2 + Senzor)
  if (prekidSrednjiAktivan) {
    prekidSrednjiAktivan = false;
    brojacSrednji++;

    // Mjerenje udaljenosti
    digitalWrite(PIN_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(PIN_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_TRIG, LOW);

    long trajanje = pulseIn(PIN_ECHO, HIGH);
    int udaljenost = trajanje * 0.034 / 2;

    Serial.print("SREDNJI PRIORITET - Udaljenost: ");
    Serial.print(udaljenost);
    Serial.print(" cm | Tipkalo 2 stisnuto: ");
    Serial.print(brojacSrednji);
    Serial.println(" puta.");
  }

  // 3. OBRADA NISKOG PRIORITETA (Timer)
  if (prekidTimerAktivan) {
    prekidTimerAktivan = false;

    // Promijeni stanje zelene LED (blinkanje svake 2 sekunde)
    digitalWrite(PIN_LED_ZELENA, !digitalRead(PIN_LED_ZELENA));
    Serial.println("NISKI PRIORITET - Timer je izvršio blinkanje Zelene LED.");
  }

  delay(10);
}

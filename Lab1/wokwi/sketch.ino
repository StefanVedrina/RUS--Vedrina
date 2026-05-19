#include <Arduino.h>

// 1. Definiranje pinova
const int PIN_TIPKALO_VISOKI = 12;
const int PIN_TIPKALO_SREDNJI = 14;
const int PIN_LED_CRVENA = 26;
const int PIN_LED_ZELENA = 27;
const int PIN_TRIG = 5;
const int PIN_ECHO = 18;

volatile bool prekidVisokiAktivan = false;
volatile bool prekidSrednjiAktivan = false;
volatile bool prekidTajmerAktivan = false;

// Brojači
int brojacVisoki = 0;
int brojacSrednji = 0;

// Struktura za timer
hw_timer_t *mojTajmer = NULL;

void isrVisokiPrioritet() {
  prekidVisokiAktivan = true;
}

void isrSrednjiPrioritet() {
  prekidSrednjiAktivan = true;
}

void onTimer() {
  prekidTajmerAktivan = true;
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n--- ESP32 LAB 1: SUSTAV POKRENUT (v3.x API) ---");

  pinMode(PIN_LED_CRVENA, OUTPUT);
  pinMode(PIN_LED_ZELENA, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  pinMode(PIN_TIPKALO_VISOKI, INPUT_PULLUP);
  pinMode(PIN_TIPKALO_SREDNJI, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(PIN_TIPKALO_VISOKI), isrVisokiPrioritet, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_TIPKALO_SREDNJI), isrSrednjiPrioritet, FALLING);

  // Postavljanje timera na 1s
  mojTajmer = timerBegin(1000000); 
  
  // Povezivanje timera s funkcijom
  timerAttachInterrupt(mojTajmer, &onTimer);
  
  // Alarm: 2.000.000 mikrosekundi (2 sekunde)
  timerAlarm(mojTajmer, 2000000, true, 0);
}

void loop() {
  
  // 1. OBRADA VISOKOG PRIORITETA (Tipkalo 1)
  if (prekidVisokiAktivan) {
    prekidVisokiAktivan = false; 
    brojacVisoki++;

    digitalWrite(PIN_LED_CRVENA, HIGH); 
    Serial.print("[!!!] VISOKI PRIORITET - Hitno tipkalo! Broj pritisaka: ");
    Serial.println(brojacVisoki);
    
    delay(300); // Kratka pauza za vizualni efekt
    digitalWrite(PIN_LED_CRVENA, LOW);
  }

  // 2. OBRADA SREDNJEG PRIORITETA (Tipkalo 2 + Senzor)
  if (prekidSrednjiAktivan) {
    prekidSrednjiAktivan = false;
    brojacSrednji++;

    // Mjerenje udaljenosti preko HC-SR04
    digitalWrite(PIN_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(PIN_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_TRIG, LOW);

    long trajanje = pulseIn(PIN_ECHO, HIGH);
    int udaljenost = trajanje * 0.034 / 2;

    Serial.print("[i] SREDNJI PRIORITET - Udaljenost: ");
    Serial.print(udaljenost);
    Serial.print(" cm | Tipkalo 2 stisnuto: ");
    Serial.print(brojacSrednji);
    Serial.println(" puta.");
  }

  // 3. OBRADA NISKOG PRIORITETA (Times)
  if (prekidTajmerAktivan) {
    prekidTajmerAktivan = false;
    digitalWrite(PIN_LED_ZELENA, !digitalRead(PIN_LED_ZELENA));
    Serial.println("[.] NISKI PRIORITET - Tajmer je izvršio blinkanje Zelene LED.");
  }

  delay(10);
}
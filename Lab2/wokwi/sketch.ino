#include <TimerOne.h>
#include <LowPower.h>

#define BUTTON 2
#define LED 4

volatile unsigned long lastTimeTriggerPressed = 0;
volatile unsigned long lastTimeWentToSleep = 0;
unsigned long triggerPressThresholdTime = 100;
const int timerDuration = 5;

volatile bool isAwake = true;

void setup() {
  Serial.begin(115200); 

  pinMode(LED, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  
  attachInterrupt(digitalPinToInterrupt(BUTTON), pressTrigger, FALLING);

  Timer1.initialize(1000000 * timerDuration);
  Timer1.attachInterrupt(timerWakeUp); 
}

void loop() {

  if (isAwake) {
    ledBlink(2);
    isAwake = false;
    goToSleep();
  }

}

// Deklaracija funkcije za slanje mikrokontrolera u sleep mod
void goToSleep() {
  Serial.println("Went to sleep");
  lastTimeWentToSleep = millis();
  Timer1.restart();
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
}

// Deklaracija funkcije za izvršavanje zadatka (treptanje LED-ice)
void ledBlink(int blinkCount) {

  Serial.println("Led Blinking");

  // For petlja koja se ponavlja onoliko puta koliki je 'blinkCount'
  for (int i = 0; i < blinkCount; i++) {
    digitalWrite(LED, HIGH);
    delay(250);
    digitalWrite(LED, LOW);
    delay(250);
  }

}

// Deklaracija prekidne rutine (ISR) za detekciju pritiska tipkala
void pressTrigger(){

  if (millis() - lastTimeTriggerPressed > triggerPressThresholdTime) {
    buttonWakeUp();
  }
  lastTimeTriggerPressed = millis();
}

// Deklaracija prekidne rutine (ISR) za buđenje putem tajmera
void timerWakeUp() {
  Serial.println("Woke up by timer");
  isAwake = true;
}

// Deklaracija funkcije za obradu logike buđenja preko gumba
void buttonWakeUp() {
    Serial.println("Woke up by button");
    isAwake = true;
}
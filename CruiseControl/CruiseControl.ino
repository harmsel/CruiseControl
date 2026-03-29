```cpp
#include <Servo.h>

// =====================================================
// 🔧 TUNING (HIER MEE SPELEN IN DE PRAKTIJK)
// =====================================================

// --- PID regeling ---
float Kp = 0.6;   // ↑ hoger = sneller reageren, ↓ lager = rustiger
float Ki = 0.04;  // ↑ hoger = meer kracht op heuvel, ↓ lager = minder "duwen"
float Kd = 0.9;   // ↑ hoger = minder overshoot, ↓ lager = sneller maar instabiel

// --- HELLING VOORSPELLING ---
float Ks = 1.5;   // ↑ hoger = eerder reageren op heuvel, ↓ lager = rustiger

// --- STABILITEIT ---
const int hysteresis = 1;  // ↑ hoger = minder trillen, ↓ lager = preciezer

// --- SNELHEID VAN REGELING ---
const int servoInterval = 120;  // ms → lager = sneller reageren

// --- SENSOR ---
const int threshold = 550;  // ⚠️ AFSTELLEN! midden tussen hoog/laag signaal


// =====================================================
// ⚙️ NIET AANPASSEN (tenzij je weet wat je doet)
// =====================================================

Servo mijnServo;
int servoHoek = 2;

// pulsmeting (tijd tussen pulsen)
unsigned long laatstePulsTijd = 0;
unsigned long pulsInterval = 0;
bool vorigeStatus = false;

// snelheid
float gemetenSnelheid = 0;
float gefilterdeSnelheid = 0;

// helling detectie
float slopeFactor = 0;
float vorigeSnelheid = 0;

// regeling
float snelheidDoel = 0;
bool ccActief = false;

// PID geheugen
float foutSom = 0;
float vorigeFout = 0;

// knoppen
const int plusKnop = 10;
const int minKnop = 9;
const int remSchakelaar = 12;

bool plusIngedrukt = 0;
bool minIngedrukt = 0;
unsigned long ingedruktSinds = 0;
const int PlusMinTijd = 400;

// buzzer
const int buzzerPin = 6;
bool isBeeping = false;
unsigned long beepStartTime = 0;
unsigned long beepDuration = 0;


// =====================================================
// SETUP
// =====================================================
void setup() {
  Serial.begin(9600);

  pinMode(plusKnop, INPUT_PULLUP);
  pinMode(minKnop, INPUT_PULLUP);
  pinMode(remSchakelaar, INPUT_PULLUP);

  mijnServo.attach(2);
  mijnServo.write(servoHoek);
}


// =====================================================
// LOOP
// =====================================================
void loop() {

  plusIngedrukt = digitalRead(plusKnop) == LOW;
  minIngedrukt = digitalRead(minKnop) == LOW;

  handleBeep();

  meetPulsen();        // 🔥 moet altijd snel blijven lopen!
  snelheidBerekenen();

  // ---------- CRUISE CONTROL AAN ----------
  if (plusIngedrukt) {
    if (ingedruktSinds == 0) {
      ingedruktSinds = millis();
    } 
    else if ((millis() - ingedruktSinds >= 1000) && !ccActief && gefilterdeSnelheid > 30) {

      snelheidDoel = gefilterdeSnelheid;
      ccActief = true;
      beep(3000, 100);
    }
  } else {
    ingedruktSinds = 0;
  }

  // ---------- ACTIEVE CC ----------
  if (ccActief) {
    servoAansturing();
    remFunctie();
    handmatigBijstellen();
  }
}


// =====================================================
// 🔥 PULS DETECTIE (ANALOOG)
// =====================================================
void meetPulsen() {
  int sensorValue = analogRead(A0);

  bool huidigeStatus = sensorValue > threshold;

  // detecteer stijgende flank
  if (huidigeStatus && !vorigeStatus) {
    unsigned long nu = micros();
    pulsInterval = nu - laatstePulsTijd;
    laatstePulsTijd = nu;
  }

  vorigeStatus = huidigeStatus;
}


// =====================================================
// 📈 SNELHEID BEREKENEN
// =====================================================
void snelheidBerekenen() {
  if (pulsInterval > 0) {

    float frequentie = 1000000.0 / pulsInterval;

    // ⚠️ HIER KALIBREREN!
    // rij bijv. 80 km/u → aanpassen tot het klopt
    gemetenSnelheid = frequentie * 1.25;

    // filter → maakt het signaal rustiger
    gefilterdeSnelheid = gefilterdeSnelheid * 0.7 + gemetenSnelheid * 0.3;

    // helling detectie (verschil in snelheid)
    slopeFactor = gefilterdeSnelheid - vorigeSnelheid;
    vorigeSnelheid = gefilterdeSnelheid;
  }
}


// =====================================================
// 🚗 SERVO (PID + HELLING)
// =====================================================
void servoAansturing() {
  static unsigned long vorigeTijd = 0;

  if (millis() - vorigeTijd >= servoInterval) {
    vorigeTijd = millis();

    float fout = snelheidDoel - gefilterdeSnelheid;

    // ---- I-term (heuvel)
    foutSom += fout;
    foutSom = constrain(foutSom, -100, 100);

    // ---- D-term (demping)
    float foutDelta = fout - vorigeFout;

    // ---- helling voorspelling
    float hellingCorrectie = -slopeFactor * Ks;

    // ---- totaal
    float correctie = (Kp * fout) + (Ki * foutSom) + (Kd * foutDelta) + hellingCorrectie;

    if (abs(fout) > hysteresis) {
      servoHoek += correctie;
      servoHoek = constrain(servoHoek, 0, 179);
      mijnServo.write(servoHoek);
    }

    vorigeFout = fout;

    // 🔧 DEBUG (optioneel aanzetten)
    /*
    Serial.print("fout:");
    Serial.print(fout);
    Serial.print(" slope:");
    Serial.print(slopeFactor);
    Serial.print(" servo:");
    Serial.println(servoHoek);
    */
  }
}


// =====================================================
// HANDMATIG BIJSTELLEN
// =====================================================
void handmatigBijstellen() {
  static unsigned long vorigePlus = 0;
  static unsigned long vorigeMin = 0;

  if (plusIngedrukt && millis() - vorigePlus > PlusMinTijd) {
    snelheidDoel += 1;
    vorigePlus = millis();
  }

  if (minIngedrukt && millis() - vorigeMin > PlusMinTijd) {
    snelheidDoel -= 1;
    vorigeMin = millis();
  }
}


// =====================================================
// REM
// =====================================================
void remFunctie() {
  if (digitalRead(remSchakelaar) == HIGH) {
    mijnServo.write(10);
    ccActief = false;
    beep(2000, 200);
  }
}


// =====================================================
// BUZZER
// =====================================================
void beep(int freq, int dur) {
  if (isBeeping) return;
  tone(buzzerPin, freq);
  beepStartTime = millis();
  beepDuration = dur;
  isBeeping = true;
}

void handleBeep() {
  if (isBeeping && millis() - beepStartTime >= beepDuration) {
    noTone(buzzerPin);
    isBeeping = false;
  }
}
```

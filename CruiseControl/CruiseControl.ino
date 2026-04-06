#include <Servo.h>

// =====================================================
// CRUISE CONTROL - TUNING
// =====================================================

const float PULS_PER_KMH_FACTOR = 1.0; 
const float SPEED_FILTER_ALPHA = 0.25; 
// 🔧 hoger = sneller reageren, lager = rustiger

const int HYSTERESIS_PULS = 2; 
// 🔧 dode zone → voorkomt constant bijregelen

const unsigned long SERVO_INTERVAL_MS = 200; 
// 🔧 hoe vaak servo bijstuurt

const float SERVO_STEP_MAX = 1.5; 
// was 2.0 🔧 max stap per correctie

const int PLUSMIN_STEP_PULSES = 2;
const unsigned long PLUSMIN_HOLD_MS = 400;

const unsigned long PULSE_TIMEOUT_US = 500000;
const unsigned long DEBUG_INTERVAL_MS = 1000;

const int SERVO_SAFE = 10;

const unsigned long SOFTSTART_MS = 1500;


// =====================================================
// HARDWARE
// =====================================================

Servo mijnServo;
const int servoPin = 2;

const int plusKnop = 10;
const int minKnop = 9;
const int remSchakelaar = 12;
const int ledPin = 11;
const int buzzerPin = 6;
const int sensorPin = A0;


// =====================================================
// VARIABELEN
// =====================================================

bool ccActief = false;
int pulsDoel = 0;

int servoHoek = 2;

int vorigeSensorStatus = LOW;
unsigned long laatstePulsTijd = 0;
unsigned long pulsIntervalUs = 0;
unsigned long vorigeGeldigePulsUs = 0;

float gemetenSnelheid = 0.0;
float gefilterdeSnelheid = 0.0;

bool plusIngedrukt = false;
bool minIngedrukt = false;
unsigned long ingedruktSinds = 0;
unsigned long vorigePlusTijd = 0;
unsigned long vorigeMinTijd = 0;

bool isBeeping = false;
unsigned long beepStartTime = 0;
unsigned long beepDuration = 0;

unsigned long vorigeServoTijd = 0;
unsigned long vorigeDebugTijd = 0;

unsigned long ccStartTijd = 0;


// =====================================================
// FEEDFORWARD STARTPOSITIE
// =====================================================

int berekenStartServo(float snelheid) {
  int servo = (int)(snelheid * 1.0 + 60.0);
  return constrain(servo, 0, 179);
}


// =====================================================
// SETUP
// =====================================================

void setup() {
  Serial.begin(9600);

  pinMode(ledPin, OUTPUT);
  pinMode(plusKnop, INPUT_PULLUP);
  pinMode(minKnop, INPUT_PULLUP);
  pinMode(remSchakelaar, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);

  mijnServo.attach(servoPin);
  mijnServo.write(servoHoek);

  Serial.println(F("Cruise control gestart"));
}


// =====================================================
// LOOP
// =====================================================

void loop() {
  plusIngedrukt = (digitalRead(plusKnop) == LOW);
  minIngedrukt = (digitalRead(minKnop) == LOW);

  handleBeep();
  pulsDetectie();
  snelheidBerekenen();

  // ---------- CC ACTIVEREN ----------
  if (plusIngedrukt) {
    if (ingedruktSinds == 0) {
      ingedruktSinds = millis();
    } 
    else if ((millis() - ingedruktSinds >= 1000) && !ccActief && gefilterdeSnelheid > 30) {

      pulsDoel = (int)round(gefilterdeSnelheid + 1);

      servoHoek = berekenStartServo(gefilterdeSnelheid);
      mijnServo.write(servoHoek);

      ccActief = true;
      ccStartTijd = millis();

      beep(3000, 100);
      Serial.println(F("CC geactiveerd"));
    }
  } else {
    ingedruktSinds = 0;
  }

  // ---------- ACTIEVE CC ----------
  if (ccActief) {
    remFunctie();
    fadeLed(2);
    servoAansturing();
    handmatigBijstellen();
  }

  // ---------- DEBUG ----------
  if (millis() - vorigeDebugTijd >= DEBUG_INTERVAL_MS) {
    vorigeDebugTijd = millis();

    Serial.print(F("pulsDoel="));
    Serial.print(pulsDoel);

    Serial.print(F("  snelheid="));
    Serial.print(gefilterdeSnelheid, 2);

    Serial.print(F("  servo="));
    Serial.print(servoHoek);

    Serial.print(F("  cc="));
    Serial.println(ccActief ? F("AAN") : F("UIT"));
  }
}


// =====================================================
// PULS DETECTIE
// =====================================================

void pulsDetectie() {
  int sensorValue = analogRead(sensorPin);
  int currentStatus = (sensorValue > 550) ? HIGH : LOW;

  if (currentStatus == HIGH && vorigeSensorStatus == LOW) {
    unsigned long nu = micros();

    if (laatstePulsTijd > 0) {
      pulsIntervalUs = nu - laatstePulsTijd;
      vorigeGeldigePulsUs = pulsIntervalUs;
    }

    laatstePulsTijd = nu;
  }

  vorigeSensorStatus = currentStatus;
}


// =====================================================
// SNELHEID
// =====================================================

void snelheidBerekenen() {
  if (laatstePulsTijd > 0 && (micros() - laatstePulsTijd) > PULSE_TIMEOUT_US) {
    gemetenSnelheid = 0;
    gefilterdeSnelheid *= 0.9;
    return;
  }

  if (vorigeGeldigePulsUs > 0) {
    float speedRaw = PULS_PER_KMH_FACTOR * (1000000.0 / (float)vorigeGeldigePulsUs);

    gemetenSnelheid = speedRaw;
    gefilterdeSnelheid = (1.0 - SPEED_FILTER_ALPHA) * gefilterdeSnelheid +
                         SPEED_FILTER_ALPHA * gemetenSnelheid;
  }
}


// =====================================================
// SERVO AANSTURING (ASymmetrisch)
// =====================================================

void servoAansturing() {
  if (millis() - vorigeServoTijd < SERVO_INTERVAL_MS) return;
  vorigeServoTijd = millis();

  float fout = (float)pulsDoel - gefilterdeSnelheid;

  // 🔥 SOFT START → alleen gas geven
  if (millis() - ccStartTijd < SOFTSTART_MS) {
    if (fout > 0) {
      float correctie = fout * 0.5;
      correctie = constrain(correctie, 0, SERVO_STEP_MAX);

      servoHoek += (int)round(correctie);
      servoHoek = constrain(servoHoek, 0, 179);
      mijnServo.write(servoHoek);
    }
    return;
  }

  if (abs(fout) <= HYSTERESIS_PULS) return;

  float correctie;

  if (fout > 0) {
    // 🔼 GAS GEVEN → sneller reageren
    correctie = fout * 0.5;
  } else {
    // 🔽 GAS LOS → rustiger (voorkomt dip!)
    correctie = fout * 0.3; 
  }

  // 🔧 asymmetrische begrenzing
  correctie = constrain(correctie, -0.7, SERVO_STEP_MAX);

  servoHoek += (int)round(correctie);
  servoHoek = constrain(servoHoek, 0, 179);

  mijnServo.write(servoHoek);
}


// =====================================================
// HANDMATIG
// =====================================================

void handmatigBijstellen() {
  if (plusIngedrukt && (millis() - vorigePlusTijd >= PLUSMIN_HOLD_MS)) {
    pulsDoel += PLUSMIN_STEP_PULSES;
    pulsDoel = constrain(pulsDoel, 0, 200);
    vorigePlusTijd = millis();
    beep(1000, 50);
  }

  if (minIngedrukt && (millis() - vorigeMinTijd >= PLUSMIN_HOLD_MS)) {
    pulsDoel -= PLUSMIN_STEP_PULSES;
    pulsDoel = constrain(pulsDoel, 0, 200);
    vorigeMinTijd = millis();
    beep(1000, 50);
  }
}


// =====================================================
// REM
// =====================================================

void remFunctie() {
  if (digitalRead(remSchakelaar) == HIGH) {
    mijnServo.write(SERVO_SAFE);
    pulsDoel = 0;
    ccActief = false;
    beep(2000, 200);
    Serial.println(F("CC gedeactiveerd"));
  }
}


// =====================================================
// LED
// =====================================================

void fadeLed(int fadeInterval) {
  static int helderheid = 0;
  static int richting = 1;
  static unsigned long vorige = 0;

  if (millis() - vorige >= (unsigned long)fadeInterval) {
    vorige = millis();

    helderheid += richting;
    if (helderheid <= 0 || helderheid >= 255) richting = -richting;

    analogWrite(ledPin, helderheid);
  }
}


// =====================================================
// BUZZER
// =====================================================

void beep(int frequency, int duration) {
  if (isBeeping) return;
  tone(buzzerPin, frequency);
  beepStartTime = millis();
  beepDuration = duration;
  isBeeping = true;
}

void handleBeep() {
  if (isBeeping && (millis() - beepStartTime >= beepDuration)) {
    noTone(buzzerPin);
    isBeeping = false;
  }
}
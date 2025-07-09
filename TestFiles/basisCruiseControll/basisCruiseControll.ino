// Cruise Control voor auto met servo- en pulsemeting

#include <Servo.h>

// --- Servo
int servoHoek = 0;
Servo mijnServo;
int servoPin = 2;

// --- Pulsmeting
const int analogPin = A0;
const int threshold = 550;
unsigned int pulseCounter = 0;
int gemetenPuls = 0;
int pulseDoel = 0;
bool pulseDetected = false;
unsigned long lastTime = 0;
const int interval = 500;  // bij 1000 is 90 rpm gelijk aan 90km

// --- Knopjes
const int plusKnop = 10;
const int minKnop = 9;
unsigned long beideIngedruktSinds = 0;
bool ccAan = false;
bool ccActief = false;
unsigned long ingedruktSinds = 0;

// --- LED
int ledPin = 11;
int helderheid = 0;
int fadeRichting = 1;
unsigned long vorigeFadeTijd = 0;

// --- Remschakelaar
int remSchakelaar = 12;

// --- piezo
const int buzzerPin = 6;
int eersteKeerActief = true;

void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  pinMode(plusKnop, INPUT_PULLUP);
  pinMode(minKnop, INPUT_PULLUP);
  pinMode(remSchakelaar, INPUT);
  mijnServo.attach(servoPin);
  servoHoek = constrain(servoHoek, 0, 179);
  mijnServo.write(10);
}

void loop() {
  bool plusIngedrukt = digitalRead(plusKnop) == LOW;
  bool minIngedrukt = digitalRead(minKnop) == LOW;

  if (plusIngedrukt) {
    if (ingedruktSinds == 0) {
      ingedruktSinds = millis();
    } else if ((millis() - ingedruktSinds >= 1000) && (!ccActief)) {
      ccActief = true;
      Serial.println(" !!!!!! !!!!! CC Geactiveerd Pulsdoel = gemetenPuls !!!!! ");
      pulseDoel = gemetenPuls;
      servoHoek = 140;
      mijnServo.write(servoHoek);
      beep(1000, 200);
    }
  } else {
    ingedruktSinds = 0;
  }

  if (ccActief) {
    fadeLedLangzaam(2000);
    servoAansturing();
    remFunctie();
  }

  pulsDetectie();
}

// ---------- SERVO AANSTURING met P-regelaar ----------
void servoAansturing() {
  static unsigned long vorigeAanpassingTijd = 0;
  unsigned long huidigeTijd = millis();

  if (huidigeTijd - vorigeAanpassingTijd >= 500) {  // was 100.
    vorigeAanpassingTijd = huidigeTijd;

    int fout = pulseDoel - gemetenPuls;
    float Kp = 1.0;  // was 1.0

    // Dode zone om kleine fouten te negeren
    if (abs(fout) > 1) {
      int aanpassing = fout * Kp;
      aanpassing = constrain(aanpassing, -3, 3);  // was 5. Is het Maximaal  graden per stap

      servoHoek += aanpassing;
      mijnServo.write(servoHoek);

      /* 
      Serial.print("fout: ");
      Serial.print(fout);
     */
      Serial.print(" | aanpassing: ");
      Serial.print(aanpassing);
      Serial.print(" | servoHoek: ");
      Serial.print(servoHoek);
    }
  }
}

// ---------- PULS DETECTIE ----------
void pulsDetectie() {
  int sensorValue = analogRead(analogPin);
  if (sensorValue > threshold) {
    if (!pulseDetected) {
      pulseCounter++;
      pulseDetected = true;
    }
  } else {
    pulseDetected = false;
  }

  unsigned long currentTime = millis();
  if (currentTime - lastTime >= interval) {
    Serial.print(" | pulse DOEL: ");
    Serial.print(pulseDoel);
    Serial.print("  | Pulse Huidig: ");
    Serial.println(pulseCounter);

    gemetenPuls = pulseCounter;
    pulseCounter = 0;
    lastTime = currentTime;
  }
}

// ---------- LED FADE ----------
void fadeLedLangzaam(int fadeInterval) {
  unsigned long huidigeTijd = micros();
  if (huidigeTijd - vorigeFadeTijd >= fadeInterval) {
    vorigeFadeTijd = huidigeTijd;

    helderheid += fadeRichting;
    if (helderheid <= 0 || helderheid >= 255) {
      fadeRichting = -fadeRichting;
    }

    analogWrite(ledPin, helderheid);
  }
}

// ---------- PIEZO BEEP ----------
void beep(int frequency, int duration) {
  tone(buzzerPin, frequency);
  delay(duration);
  noTone(buzzerPin);
}

// ---------- REM FUNCTIE ----------
void remFunctie() {
  if (digitalRead(remSchakelaar) == HIGH) {
    Serial.println("Remschakelaar geactiveerd");
    mijnServo.write(10);
    pulseDoel = 0;
    ccActief = false;
  }
}

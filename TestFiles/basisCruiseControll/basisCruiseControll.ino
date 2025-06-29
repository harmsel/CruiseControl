// Cruise Control voor auto met servo- en pulsemeting

//90km = 93 rps
//80km = 84 per sec
//60km = 30 per sec
//50km = 26 per sec

#include <Servo.h>

// --- Servo
int servoHoek = 0;
Servo mijnServo;
int servoPin = 2;

// --- Pulsmeting
const int analogPin = A0;       // hier zit de magneetspoel aan vast
const int threshold = 550;      //voltage waarop de puls geteld gaat worden
unsigned int pulseCounter = 0;  //de teller elke seconden de omwentelingen meet en ook weer worrdt gereset
int gemetenPuls = 0;            // deze is nodig zodat pulseDoel opgeslagen kan worden (pulseCounter reset elke 0.5 seconden)
int pulseDoel = 0;              // ingestelde snelheid
bool pulseDetected = false;
unsigned long lastTime = 0;
const int interval = 1000;  //zo lang gaat hij pulsen tellen

// --- Knopjes
const int plusKnop = 10;
const int minKnop = 9;
unsigned long beideIngedruktSinds = 0;
bool ccAan = false;
bool ccActief = false;
unsigned long ingedruktSinds = 0;

bool vorigeKnopStatus = HIGH;           //voor debounce
unsigned long laatsteDebounceTijd = 0;  // debounce plusknop


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
  servoHoek = constrain(servoHoek, 0, 179);  //beperk de servo in zijn beweging
  mijnServo.write(10);
}
//// ----------------------------------    BEGIN LOOOOOOOP -----------------------------------------------------------
void loop() {
  bool plusIngedrukt = digitalRead(plusKnop) == LOW;
  bool minIngedrukt = digitalRead(minKnop) == LOW;


  // --- ACTIveren maken met plusknop

  if (plusIngedrukt) {

    if (ingedruktSinds == 0) {
      ingedruktSinds = millis();
    } else if (millis() - ingedruktSinds >= 1000) {  // pas na het inhouden van de knop gaat het aan
      ccActief = true;
      Serial.println("CC Geactiveerd Pulsdoen = gemetenPuls");
      pulseDoel = gemetenPuls;
      servoHoek = 150;
      beep(1000, 200);  // Frequentie in Hz, Duur in ms
    }
  } else {
    ingedruktSinds = 0;
  }

  if (ccActief) {
    fadeLedLangzaam(2000);  // in MIcro's
    servoAansturing();
    remFunctie();
  }
  pulsDetectie();
}
// ---------------------------------------------    EINDE LOOP Begin Functies ------------------------------------- //


void servoAansturing() {
  static unsigned long vorigeAanpassingTijd = 0;
  unsigned long huidigeTijd = millis();

  if (huidigeTijd - vorigeAanpassingTijd >= 1100) {  // aanpassing van de servostand elke zoveel tijd
    vorigeAanpassingTijd = huidigeTijd;

    if (pulseDoel > gemetenPuls) {
      servoHoek = servoHoek + (pulseDoel - gemetenPuls);
      Serial.print(" + + + + + +   Servohoek:  ");
      Serial.println(servoHoek);

    } else if (pulseDoel < gemetenPuls) {
      servoHoek = servoHoek - (gemetenPuls - pulseDoel);
      Serial.print("- - - - - - - - - -  Servohoek: ");
      Serial.println(servoHoek);
    }
    mijnServo.write(servoHoek);
  }
}



// ---------- PULSE detectie:
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
    Serial.print(" pulse DOEL: ");
    Serial.print(pulseDoel);
    Serial.print(" Pulse Huidig: ");
    Serial.println(pulseCounter);

    gemetenPuls = pulseCounter;
    pulseCounter = 0;
    lastTime = currentTime;
  }
}

/// - LED FADE
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

// ---  Pieeeeep
void beep(int frequency, int duration) {
  tone(buzzerPin, frequency);
  delay(duration);
  noTone(buzzerPin);
}

//// -- remmen
void remFunctie() {
  if (digitalRead(remSchakelaar) == HIGH) {
    Serial.println("Remschakelaar geactiveerd");
    mijnServo.write(10);
    pulseDoel = 0;
    ccActief = false;
  }
}
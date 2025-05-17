// Cruise Control voor auto met servo- en pulsemeting

//90km = 93 rps
//80km = 84 per sec
//50km = 26 per sec

#include <Servo.h>

// --- Servo
float servoHoek = 0;
float aanstuurFactor = 1.5;
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
int plusKnop = 10;
int minKnop = 9;
unsigned long beideIngedruktSinds = 0;
bool ccAan = false;
bool ccActief = false;
unsigned long ingedruktSinds = 0;

bool vorigeKnopStatus = HIGH; //voor debounce
unsigned long laatsteDebounceTijd = 0;// debounce plusknop


// --- LED
int ledPin = 11;
int helderheid = 0;
int fadeRichting = 1;
unsigned long vorigeFadeTijd = 0;

// --- Remschakelaar
int remSchakelaar = 12;

void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  pinMode(plusKnop, INPUT_PULLUP);
  pinMode(minKnop, INPUT_PULLUP);
  pinMode(remSchakelaar, INPUT);
  mijnServo.attach(servoPin);
  servoHoek = constrain(servoHoek, 0, 70);  //beperk de servo in zijn beweging
  mijnServo.write(0);
}
//// ----------------------------------    BEGIN LOOOOOOOP -----------------------------------------------------------
void loop() {
  bool plusIngedrukt = digitalRead(plusKnop) == LOW;
  bool minIngedrukt = digitalRead(minKnop) == LOW;

  // --- Aanzetten met beide knoppen
  if (plusIngedrukt && minIngedrukt) {
    if (beideIngedruktSinds == 0) {
      beideIngedruktSinds = millis();
    } else if (millis() - beideIngedruktSinds >= 1000 && !ccAan) {
      ccAan = true;
      Serial.println("CC -- AANGEZET");
    }
  } else {
    beideIngedruktSinds = 0;
  }

  // --- ACTIveren maken met plusknop
  if (ccAan) {
    if (plusIngedrukt) {

      if (ingedruktSinds == 0) {
        ingedruktSinds = millis();
      } else if (millis() - ingedruktSinds >= 2000) {
        ccActief = true;
        Serial.println("CC Geactiveerd Pulsdoen = gemetenPuls");
        pulseDoel = gemetenPuls;
      }
    } else {
      ingedruktSinds = 0;
    }
  }


  // --- LED fade
  if (ccAan && !ccActief) {
    fadeLedLangzaam(9);  // langzaam knipperen van de led

  } else if (ccActief) {
    fadeLedLangzaam(1);  // snel
    servoAansturing();
    remFunctie();
  }
  pulsDetectie();
}
// ---------------------------------------------    EINDE LOOP Begin Functies ------------------------------------- //


void servoAansturing() {
  static unsigned long vorigeAanpassingTijd = 0;
  unsigned long huidigeTijd = millis();

  if (huidigeTijd - vorigeAanpassingTijd >= 30) {// nummer is de sneheid van het draaien van de servo
    vorigeAanpassingTijd = huidigeTijd;

    if (pulseDoel > gemetenPuls) {
      servoHoek++;
    } else if (pulseDoel < gemetenPuls) {
      servoHoek--;
    }

    servoHoek = constrain(servoHoek, 1, 71);
    mijnServo.write((int)servoHoek);

    // Debug:
    // Serial.print("Servohoek: ");
    // Serial.println(servoHoek);
  }
}






// ---- Pulsdetectie
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
    Serial.print("pulseCounter: ");
    Serial.println(pulseCounter);
    Serial.print("pulseDoel: ");
    Serial.println(pulseDoel);
    gemetenPuls = pulseCounter;
    pulseCounter = 0;
    lastTime = currentTime;
  }
}
// ---- EINDE Pulsdetectie


void fadeLedLangzaam(int fadeInterval) {
  unsigned long huidigeTijd = millis();
  if (huidigeTijd - vorigeFadeTijd >= fadeInterval) {
    vorigeFadeTijd = huidigeTijd;

    helderheid += fadeRichting;
    if (helderheid <= 0 || helderheid >= 255) {
      fadeRichting = -fadeRichting;
    }

    analogWrite(ledPin, helderheid);
  }
}

void remFunctie() {
  if (digitalRead(remSchakelaar) == HIGH) {
    Serial.println("Remschakelaar geactiveerd");
    pulseDoel = 0;
    ccActief = false;
    mijnServo.write(2);
  }
}

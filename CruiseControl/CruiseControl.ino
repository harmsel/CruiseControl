// Cruise Control voor auto met servo- en pulsemeting

//90km = 93 rps
//80km = 84 per sec
//50km = 26 per sec

#include <Servo.h>

// --- Servo
int servoHoek = 10;
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

bool vorigeKnopStatus = HIGH;           //voor debounce
unsigned long laatsteDebounceTijd = 0;  // debounce plusknop


// --- LED
int ledPin = 11;
int helderheid = 0;
int fadeRichting = 1;
unsigned long vorigeFadeTijd = 0;

// --- Remschakelaar en koppelingschakelaar
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
  servoHoek = constrain(servoHoek, 10, 176);  //beperk de servo in zijn beweging
  mijnServo.write(10);
  pinMode(buzzerPin, OUTPUT);
}
//// ----------------------------------    BEGIN LOOOOOOOP -----------------------------------------------------------
void loop() {
  bool plusIngedrukt = digitalRead(plusKnop) == LOW;
  bool minIngedrukt = digitalRead(minKnop) == LOW;


  // --- Aanzetten van de CC =  beide knoppen indrukken
  if (plusIngedrukt && minIngedrukt) {
    if (beideIngedruktSinds == 0) {
      beideIngedruktSinds = millis();
    } else if (millis() - beideIngedruktSinds >= 500 && !ccAan) {
      ccAan = true;
      Serial.println("CC -- AANGEZET");
    }
  } else {
    beideIngedruktSinds = 0;
  }

  // --- Inschakelen van de CC =  plusknop ingedrukt houden
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
    fadeLed(9000);  // langzaam knipperen van de led

  } else if (ccActief) {  //CC is ingeschakeld
    fadeLed(1000);        //  faden in MICRO's !!
    servoAansturing();
    remFunctie();
    if (eersteKeerActief) {  //alleen bij het actief maken een keer piepen
      beep(1000, 100);       // Frequentie: 500 Hz, Duur: 100 ms
      eersteKeerActief = false;
    }
  }

  if ((plusIngedrukt || minIngedrukt) && ccAan) {  // feedback bij indrukken knopjes
    fadeLed(200);                                  //  snel faden in MICRO's !!
  }


  pulsDetectie();
}
// ---------------------------------------------    EINDE LOOP Begin Functies ------------------------------------- //


void servoAansturing() {  // pas de servohoek aan, op basis van de ingestelde doelsnelheid en de huidige snelheid
  static unsigned long vorigeAanpassingTijd = 0;
  unsigned long huidigeTijd = millis();

  // if (pulseDoel > 10 && pulseDoel < 110) {  // zorg dat hij niet gaat werken bij vreemde pulswaarden

  // if (eersteKeerActief) {
  //   mijnServo.write(150);
  // }



  if (huidigeTijd - vorigeAanpassingTijd >= 30) {  // nummer is de sneheid van het draaien van de servo
    vorigeAanpassingTijd = huidigeTijd;

    if (pulseDoel > gemetenPuls) {
      servoHoek++;

      Serial.print("Servohoek: ");
      Serial.println(servoHoek);
    } else if (pulseDoel < gemetenPuls) {
      servoHoek--;

      Serial.print("Servohoek: ");
      Serial.println(servoHoek);
    }

    mijnServo.write((int)servoHoek);
  }
  //}
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


void fadeLed(int fadeInterval) {
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


// ---  Functie die een piep laat horen
void beep(int frequency, int duration) {
  tone(buzzerPin, frequency);
  delay(duration);
  noTone(buzzerPin);
}



void remFunctie() {
  if (digitalRead(remSchakelaar) == HIGH) {
    Serial.println("Remschakelaar geactiveerd");
    pulseDoel = 0;
    ccActief = false;
    mijnServo.write(10);
    eersteKeerActief = true;
  }
}

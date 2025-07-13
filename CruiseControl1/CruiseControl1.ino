// Cruise Control voor auto met servo- en pulsmeting

/// allleen bij >60km

#include <Servo.h>

// --- Servo
Servo mijnServo;
int servoHoek = 10;

// --- Pulsmeting
unsigned int pulsCounter = 0;
int gemetenPuls = 0;
int pulsDoel = 0;
bool pulsDetected = false;
unsigned long lastTime = 0;

// --- Knopjes
const int plusKnop = 10;
const int minKnop = 9;
bool ccActief = false;
unsigned long ingedruktSinds = 0;
bool plusIngedrukt = 0;
bool minIngedrukt = 0;



// --- LED
int ledPin = 11;


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
  pinMode(remSchakelaar, INPUT_PULLUP);
  mijnServo.attach(2);
  mijnServo.write(servoHoek);

}

void loop() {
  plusIngedrukt = digitalRead(plusKnop) == LOW;
  minIngedrukt = digitalRead(minKnop) == LOW;

  if (plusIngedrukt && (gemetenPuls > 10 && gemetenPuls < 55)) {  //> 30km om te voorkomen dat hij per ongeluk aangaat, bovenwaarde als de sensor raar gaat doen
    if (ingedruktSinds == 0) {
      ingedruktSinds = millis();
    } else if ((millis() - ingedruktSinds >= 1000) && (!ccActief)) {
      ccActief = true;
      Serial.println("  !!!!!! !!!!! CC Geactiveerd. Pulsdoel = gemetenPuls !!!!! ");
      pulsDoel = gemetenPuls;
      servoHoek = 140;
      mijnServo.write(servoHoek);
      beep(2000, 200);  //frequentie, duur
    }
  } else {
    ingedruktSinds = 0;
  }

  if (ccActief) { // alleen aan vanaf 30 km/h en tot 110 km/h
    fadeLed(5);
    handmatigBijstellen();
    servoAansturing();
    remFunctie();
  }

  pulsDetectie();
}

void handmatigBijstellen() {
  static unsigned long vorigePlusTijd = 0;  // Met 'static' wordt vorigePlusTijd alleen bij de eerste aanroep 0
  static unsigned long vorigeMinTijd = 0;

  // Plusknop ingedrukt → verhogen
  unsigned long nu = millis();

  // Plusknop ingedrukt → verhogen
  if (plusIngedrukt && nu - vorigePlusTijd >= 500) {  // getal is de wachttijd tussen de plussen
    pulsDoel += 1;
    servoHoek += 4; // was eerder 3
    mijnServo.write(servoHoek);
    Serial.println("+ + + + + + + Verhogen ");
    beep(1000, 50);  //frequentie, duur
    vorigePlusTijd = nu;
  }

  // Minknop ingedrukt → verlagen
  if (minIngedrukt && nu - vorigeMinTijd >= 500) {
    pulsDoel -= 1;
    servoHoek -= 4; // was eerder 3
    mijnServo.write(servoHoek);
    Serial.println(" - - - - - - - Verlagen");
    beep(1000, 50);  //frequentie, duur
    vorigeMinTijd = nu;
  }
}


// ---------- SERVO AANSTURING ----------
void servoAansturing() {
  static unsigned long vorigeAanpassingTijd = 0;
  unsigned long huidigeTijd = millis();

  if (huidigeTijd - vorigeAanpassingTijd >= 500) {  // elke halve seconde kijken of aanpassing nodig is
    vorigeAanpassingTijd = huidigeTijd;

    int fout = (pulsDoel - gemetenPuls)  * 1.1; // Hier mika FOUT. kan 0.9 , 1.0 , 1.1 zijn

    if (abs(fout) > 0) {   // mika aanpas - AFWIJKING            //reageer niet op elke afwijking. Abs kijkt wat de waarde is tov 0
      fout = constrain(fout, -5, 5);  // Maximaal  graden per aanpassing
      servoHoek += fout;
      servoHoek = constrain(servoHoek, 0, 179);  //
      mijnServo.write(servoHoek);


      Serial.print(" | aanpassing: ");
      Serial.print(fout);
      Serial.print(" | servoHoek: ");
      Serial.print(servoHoek);
    }
  }
}

// ---------- PULS DETECTIE ----------
void pulsDetectie() {
  int sensorValue = analogRead(A0);
  if (sensorValue > 550) {  //de treshold op 550 zetten, anders meet hij te weinig pulsn of te veel
    if (!pulsDetected) {
      pulsCounter++;
      pulsDetected = true;
    }
  } else {
    pulsDetected = false;
  }

  unsigned long currentTime = millis();
  if (currentTime - lastTime >= 500) {  // waarde is de duur van de meting
    Serial.print(" | puls DOEL: ");
    Serial.print(pulsDoel);
    Serial.print("  | Pulse Huidig: ");
    Serial.println(pulsCounter);

    gemetenPuls = pulsCounter;
    pulsCounter = 0;
    lastTime = currentTime;
  }
}

// ---------- LED FADE ----------
void fadeLed(int fadeInterval) {
  static int helderheid = 0;
  static int fadeRichting = 1;
  static unsigned long vorigeFadeTijd = 0;  // Deze moet ook static zijn

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
    pulsDoel = 0;
    ccActief = false;
  }
}

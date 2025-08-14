// 19 pulsen / 200 ms = 91
// 51 km h = 143 servo = 25 pulsen per second
// 35km = 18 pulsen per seconde

#include <Servo.h>

// AAANPASSINGEN OM DE REGELING RUSTIGER TE MAKEN
// 0.7 werkte goed, maar bij 94 pulsen bleef het heen een weer gaan // 1.2 is te extreem
const float correctieVersnellen = 0.8;  // als de snelheid te LAAG is dit de factor. fout X correctieVertraging = servoverdraaing, hoe lager het getal des te trager de correctie per servoInterval

// 1.2 werkt best goed ook in de bergen
const float correctieVertragen = 1.3;  // als de snelheid te hoog is is dit de snelheid van correctie
const int hysteresis = 1;              // Als pulsMeting +1 of -1  groter dan  de histeresis, dan corrigeren, bij interval van 1000 en een inteval van 0 zit er dus max 2km verschil tussen de snelheid en de gewenste snelheid


const int meetInterval = 1000;   // zoveel tijd de pulsen tellen, hoger getal is tragere meting, maar naukeuriger
const int servoInterval = 1000;  // elke interval gaat de servo mogelijk verdraaien

// --- Servo
Servo mijnServo;
int servoHoek = 2;


// --- Pulsmeting
unsigned int pulsCounter = 0;
int gemetenPuls = 0;  // het aantal pulsen per meetInterval
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
int const PlusMinTijd = 400;  // zo lang moet je het knop je vasthouden voordat er een druk gemeten is

// --- LED
int ledPin = 11;

// --- Remschakelaar
const int remSchakelaar = 12;

// --- Piezo (Buzzer)
const int buzzerPin = 6;
bool isBeeping = false;
unsigned long beepStartTime = 0;
unsigned long beepDuration = 0;


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

  handleBeep();  //piep ook weer stoppen

  // ----------------------------------------   Cruise control activeren. ------------------
  if (plusIngedrukt) {
    if (ingedruktSinds == 0) {
      ingedruktSinds = millis();
    } else if ((millis() - ingedruktSinds >= 1000) && !ccActief && gemetenPuls > 60) {
      Serial.println(" ~~~~~~~~. mika help!  hij is CC Geactiveerd. <3 :-)  ");
      pulsDoel = gemetenPuls;

      servoHoek = map(gemetenPuls, 80, 110, 126, 175);  //servohoek is nu 142 bij 90 Pulsen p/s
      mijnServo.write(servoHoek);                       //servohoek 145 is ontgeveer 90km/h

      beep(3000, 100);  //frequentie, duur
      ccActief = true;
    }
  } else {
    ingedruktSinds = 0;
  }

  // Als cruise control actief is
  if (ccActief) {
    remFunctie();
    fadeLed(2);
    servoAansturing();
    handmatigBijstellen();
  }

  pulsDetectie();
}




// ---------- SERVO AANSTURING -------------------------------------------------------------
// ---------- SERVO AANSTURING -------------------------------------------------------------
// ---------- SERVO AANSTURING -------------------------------------------------------------
void servoAansturing() {
  static unsigned long vorigeAanpassingTijd = 0;
  unsigned long huidigeTijd = millis();

  if (huidigeTijd - vorigeAanpassingTijd >= servoInterval) {
    vorigeAanpassingTijd = huidigeTijd;
    int fout = pulsDoel - gemetenPuls;

    if (abs(fout) > hysteresis) {
      float correctie = fout > 0 ? correctieVersnellen : correctieVertragen;  // bij te hoge snelheid gebruikt hij correctievertragen anders correctieversnelllen
      servoHoek += fout * correctie;
      servoHoek = constrain(servoHoek, 0, 179);
      mijnServo.write(servoHoek);
      Serial.print(" | servoHoek: ");
      Serial.print(servoHoek);
    }

    // CC ER UIT LATEN KLAPPEN --- als de auto 9 km langzamer gaat dan het pulsdoel, dan stopt de cc
    if (gemetenPuls <= pulsDoel - 10) {
      Serial.println("Nu de cc er uit laten klappen");
      beep(5000, 1000);  //frequentie, duur
      delay(1000);       //pas na het piepen van het gas af
      pulsDoel = 0;
      mijnServo.write(1);
      ccActief = false;
    }
  }
}










// --------------------------------     HANDMATIG BIJSTELLEN MET PLUS EN MIN -------------
void handmatigBijstellen() {
  static unsigned long vorigePlusTijd = 0;  // Met 'static' wordt vorigePlusTijd alleen bij de eerste aanroep 0
  static unsigned long vorigeMinTijd = 0;

  // Plusknop ingedrukt → verhogen
  if (plusIngedrukt && (millis() - vorigePlusTijd >= PlusMinTijd) && (millis() - ingedruktSinds < 1000)) {  //laaste deel is om te voorkomen dat bij het inschakelen geljk een verhoging doorkomt
    pulsDoel += 1;
    servoHoek += 3;  //gelijk bijstellen
    mijnServo.write(servoHoek);

    pulsDoel = constrain(pulsDoel, 0, 110);  //voorkom dat pulsdoel negatief kan worden
    Serial.println("+ + + + + + + Verhogen ");
    beep(1000, 50);             //frequentie, duur
    vorigePlusTijd = millis();  // de timer weer naar huidige millis zetten
  }

  // Minknop ingedrukt → verlagen
  if (minIngedrukt && millis() - vorigeMinTijd >= PlusMinTijd) {
    pulsDoel -= 1;

    servoHoek -= 4;  //gelijk bijstellen
    mijnServo.write(servoHoek);



    pulsDoel = constrain(pulsDoel, 0, 120);  //voorkom dat pulsdoel negatief kan worden
    Serial.println(" - - - - - - - Verlagen");
    beep(1000, 50);  //frequentie, duur
    vorigeMinTijd = millis();
  }
}


// --------------------------- PULS DETECTIE ----------------------------------------------
void pulsDetectie() {
  int sensorValue = analogRead(A0);
  if (sensorValue > 550) {  //de treshold op 550 zetten, anders meet hij te weinig pulsen of te veel
    if (!pulsDetected) {
      pulsCounter++;
      pulsDetected = true;
    }
  } else {
    pulsDetected = false;
  }

  if (millis() - lastTime >= meetInterval) {  // waarde is de duur van de meting
    Serial.print(" || DOEL: ");
    Serial.print(pulsDoel);

    Serial.print("    | HUIDIG: ");
    Serial.println(pulsCounter);

    gemetenPuls = pulsCounter;
    pulsCounter = 0;
    lastTime = millis();
  }
}


// ---------- LED FADE ----------
void fadeLed(int fadeInterval) {
  static int helderheid = 0;
  static int fadeRichting = 1;
  static unsigned long vorigeFadeTijd = 0;

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

// ---------- PIEZO BEEP (zonder Delay) ----------
// Start een piep
void beep(int frequency, int duration) {
  if (isBeeping) return;  // Start geen nieuwe piep als er al een speelt
  tone(buzzerPin, frequency);
  beepStartTime = millis();
  beepDuration = duration;
  isBeeping = true;
}

// Kijk of de piep moet stoppen. Wordt aangeroepen vanuit de loop()
void handleBeep() {
  if (isBeeping && (millis() - beepStartTime >= beepDuration)) {
    noTone(buzzerPin);
    isBeeping = false;
  }
}

// ---------- REM FUNCTIE ----------
void remFunctie() {
  if (digitalRead(remSchakelaar) == HIGH) {
    mijnServo.write(10);
    pulsDoel = 0;
    ccActief = false;
    beep(2000, 200);  //frequentie, duur

    Serial.println(" HELP de Cruise Control is GEDEACTIVEEEEEEEEERD");
  }
}

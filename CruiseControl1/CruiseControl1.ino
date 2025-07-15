// Verbeterde Cruise Control voor auto met PID-regelaar
// - Snellere respons door PID-logica en kortere meet-interval.
// - Reageert proactief op veranderingen (heuvels) om snelheid stabieler te houden.
// - Werkt nu vanaf 60 km/u zoals gevraagd.

#include <Servo.h>

// --- Constanten voor configuratie ---
const int SERVO_MIN_HOEK = 10;      // Servohoek bij geen gas
const int SERVO_MAX_HOEK = 170;     // Maximale servohoek (begrenzing)
const int ACTIVATIE_PULS_MIN = 20;  // Minimale pulsen om CC te activeren (~60 km/u)
const int DEACTIVATIE_PULS_MAX = 55;// Maximale pulsen (veiligheidsgrens)
const long REGEL_INTERVAL = 100;    // Interval voor de PID-regelaar en pulsmeting in ms (was 500ms)

// --- Servo ---
Servo mijnServo;
int servoHoek = SERVO_MIN_HOEK;

// --- Pulsmeting ---
volatile unsigned int pulsCounter = 0; // 'volatile' omdat deze in een ISR gebruikt kan worden (optioneel, maar goede praktijk)
int gemetenPuls = 0;
int pulsDoel = 0;
bool pulsDetected = false;
unsigned long lastTime = 0;

// --- PID Regelaar Variabelen ---
double Kp = 2.5;  // Proportionele gain: Bepaalt de reactie op de HUIDIGE fout. Startpunt: 2.5
double Ki = 0.08; // Integrale gain: Elimineert fouten over TIJD (bv. op een helling). Startpunt: 0.08
double Kd = 0.7;  // Derivatieve gain: Voorspelt TOEKOMSTIGE fouten en dempt de reactie. Startpunt: 0.7

double integraal = 0;
double vorigeFout = 0;

// --- Knopjes ---
const int plusKnop = 10;
const int minKnop = 9;
bool ccActief = false;
unsigned long ingedruktSinds = 0;

// --- LED ---
const int ledPin = 11;

// --- Remschakelaar ---
const int remSchakelaar = 12;

// --- Piezo Buzzer ---
const int buzzerPin = 6;

void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  pinMode(plusKnop, INPUT_PULLUP);
  pinMode(minKnop, INPUT_PULLUP);
  pinMode(remSchakelaar, INPUT_PULLUP);
  mijnServo.attach(2);
  mijnServo.write(servoHoek);
  Serial.println("Cruise Control Systeem Gestart");
}

void loop() {
  // Lees de knoppen en de sensor
  bool plusIngedrukt = digitalRead(plusKnop) == LOW;
  bool minIngedrukt = digitalRead(minKnop) == LOW;
  pulsDetectie(); // Meet de snelheid

  // Cruise control activeren
  if (plusIngedrukt && (gemetenPuls > ACTIVATIE_PULS_MIN && gemetenPuls < DEACTIVATIE_PULS_MAX)) {
    if (ingedruktSinds == 0) {
      ingedruktSinds = millis();
    } else if ((millis() - ingedruktSinds >= 1000) && (!ccActief)) {
      ccActief = true;
      pulsDoel = gemetenPuls; // Stel huidige snelheid in als doel
      integraal = 0;         // Reset PID-variabelen bij activatie
      vorigeFout = 0;
      Serial.println("!!!!!! CC Geactiveerd !!!!!");
      Serial.print("Doel snelheid (pulsen): ");
      Serial.println(pulsDoel);
      beep(2000, 200);
    }
  } else if (!plusIngedrukt) {
    ingedruktSinds = 0;
  }

  // Als de cruise control actief is
  if (ccActief) {
    fadeLed(5);
    handmatigBijstellen(plusIngedrukt, minIngedrukt);
    pidAansturing(); // Gebruik de nieuwe PID-regelaar
    remFunctie();
  } else {
    digitalWrite(ledPin, LOW); // Zet LED uit als CC inactief is
  }
}

void handmatigBijstellen(bool plusIngedrukt, bool minIngedrukt) {
  static unsigned long vorigeKnopTijd = 0;
  unsigned long nu = millis();

  if (nu - vorigeKnopTijd >= 250) { // Wachttijd tussen aanpassingen
    if (plusIngedrukt) {
      pulsDoel += 1; // Verhoog het doel met 1 km/u (equivalent)
      vorigeKnopTijd = nu;
      Serial.print("+ Verhogen. Nieuw doel: ");
      Serial.println(pulsDoel);
      beep(1000, 50);
    } else if (minIngedrukt) {
      pulsDoel -= 1; // Verlaag het doel
      vorigeKnopTijd = nu;
      Serial.print("- Verlagen. Nieuw doel: ");
      Serial.println(pulsDoel);
      beep(1000, 50);
    }
  }
  // Let op: we passen de servo niet meer direct aan. De PID-lus doet dit nu automatisch.
}

// ---------- NIEUWE PID SERVO AANSTURING ----------
void pidAansturing() {
  static unsigned long vorigeAanpassingTijd = 0;
  unsigned long huidigeTijd = millis();

  if (huidigeTijd - vorigeAanpassingTijd >= REGEL_INTERVAL) {
    vorigeAanpassingTijd = huidigeTijd;

    // 1. Bereken de fout (het verschil tussen doel en werkelijkheid)
    double fout = pulsDoel - gemetenPuls;

    // 2. Bereken de integraal (som van fouten over tijd)
    // Dit heft de constante fout op die je op een heuvel ziet.
    integraal += fout;
    // Anti-windup: begrens de integraal om te voorkomen dat hij te groot wordt.
    integraal = constrain(integraal, -100, 100);

    // 3. Bereken de afgeleide (de snelheid waarmee de fout verandert)
    // Dit dempt de reactie en voorkomt dat de snelheid doorschiet.
    double afgeleide = fout - vorigeFout;

    // 4. Bereken de totale aanpassing
    double aanpassing = (Kp * fout) + (Ki * integraal) + (Kd * afgeleide);

    // 5. Pas de servohoek aan
    // We tellen de aanpassing op bij de HUIDIGE hoek
    servoHoek += aanpassing;
    servoHoek = constrain(servoHoek, SERVO_MIN_HOEK, SERVO_MAX_HOEK); // Begrens de servohoek
    mijnServo.write(servoHoek);

    // 6. Onthoud de huidige fout voor de volgende keer
    vorigeFout = fout;

    // Debugging output
    Serial.print(" | Fout: "); Serial.print(fout);
    Serial.print(" | Aanpassing: "); Serial.print(aanpassing, 2); // Toon met 2 decimalen
    Serial.print(" | Nieuwe Servohoek: "); Serial.println(servoHoek);
  }
}

// ---------- SNELLERE PULS DETECTIE ----------
void pulsDetectie() {
  // Leest de sensor. Dit deel is onveranderd.
  int sensorValue = analogRead(A0);
  if (sensorValue > 550) {
    if (!pulsDetected) {
      pulsCounter++;
      pulsDetected = true;
    }
  } else {
    pulsDetected = false;
  }

  // Meetinterval is nu veel korter voor een snellere update van de snelheid.
  unsigned long currentTime = millis();
  if (currentTime - lastTime >= REGEL_INTERVAL) {
    gemetenPuls = pulsCounter;
    pulsCounter = 0;
    lastTime = currentTime;

    // Verplaatst de seriële output hierheen om de loop schoner te houden
    Serial.print("Puls Doel: "); Serial.print(pulsDoel);
    Serial.print(" | Puls Huidig: "); Serial.print(gemetenPuls);
  }
}

// ---------- REM FUNCTIE ----------
void remFunctie() {
  if (digitalRead(remSchakelaar) == HIGH) {
    ccActief = false;
    pulsDoel = 0;
    integraal = 0; // Reset ook de integraal bij remmen
    servoHoek = SERVO_MIN_HOEK; // Zet servo direct naar ruststand
    mijnServo.write(servoHoek);
    Serial.println("!!!!!! CC Gedeactiveerd door rem !!!!!");
    beep(800, 300);
  }
}

// ---------- HULP FUNCTIES (onveranderd) ----------
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

void beep(int frequency, int duration) {
  tone(buzzerPin, frequency, duration);
}

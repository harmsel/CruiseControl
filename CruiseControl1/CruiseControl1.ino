#include <Servo.h>

// --- Constanten voor configuratie ---
const int SERVO_MIN_HOEK = 10;      // Servohoek bij geen gas
const int SERVO_MAX_HOEK = 179;     // Maximale servohoek (begrenzing)
const int ACTIVATIE_PULS_MIN = 10;  // Minimale pulsen om CC te activeren (~50 km/u bij 200ms interval)
const int DEACTIVATIE_PULS_MAX = 25;// Maximale pulsen (veiligheidsgrens), per REGEL_INTERVAL
const long REGEL_INTERVAL = 200;    // Probeer ook eens 100, Interval voor de PID-regelaar en pulsmeting in ms (was 500ms)

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

// --- Piezo Buzzer (Non-blocking) ---
const int buzzerPin = 6;
unsigned long beepEindTijd = 0; // Houdt bij wanneer de piep moet stoppen

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
  // --- Beheer van achtergrondtaken ---
  handleBeep(); // Beheert het stoppen van de piep zonder delay()

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
}

void pidAansturing() {
  static unsigned long vorigeAanpassingTijd = 0;
  unsigned long huidigeTijd = millis();

  if (huidigeTijd - vorigeAanpassingTijd >= REGEL_INTERVAL) {
    vorigeAanpassingTijd = huidigeTijd;
    double fout = pulsDoel - gemetenPuls;
    integraal += fout;
    integraal = constrain(integraal, -100, 100);
    double afgeleide = fout - vorigeFout;
    double aanpassing = (Kp * fout) + (Ki * integraal) + (Kd * afgeleide);
    servoHoek += aanpassing;
    servoHoek = constrain(servoHoek, SERVO_MIN_HOEK, SERVO_MAX_HOEK);
    mijnServo.write(servoHoek);
    vorigeFout = fout;
    Serial.print(" | Fout: "); Serial.print(fout);
    Serial.print(" | Aanpassing: "); Serial.print(aanpassing, 2);
    Serial.print(" | Nieuwe Servohoek: "); Serial.println(servoHoek);
  }
}

void pulsDetectie() {
  int sensorValue = analogRead(A0);
  if (sensorValue > 550) {
    if (!pulsDetected) {
      pulsCounter++;
      pulsDetected = true;
    }
  } else {
    pulsDetected = false;
  }
  unsigned long currentTime = millis();
  if (currentTime - lastTime >= REGEL_INTERVAL) {
    gemetenPuls = pulsCounter;
    pulsCounter = 0;
    lastTime = currentTime;
    Serial.print("Puls Doel: "); Serial.print(pulsDoel);
    Serial.print(" | Puls Huidig: "); Serial.print(gemetenPuls);
  }
}

void remFunctie() {
  if (digitalRead(remSchakelaar) == HIGH) {
    ccActief = false;
    pulsDoel = 0;
    integraal = 0;
    servoHoek = SERVO_MIN_HOEK;
    mijnServo.write(servoHoek);
    Serial.println("!!!!!! CC Gedeactiveerd door rem !!!!!");
    beep(800, 300);
  }
}

// ---------- HULP FUNCTIES ----------

// Beheert het stoppen van de piep zonder delay()
void handleBeep() {
  // Als er een piep actief is (eindtijd is gezet) en de huidige tijd is voorbij de eindtijd
  if (beepEindTijd > 0 && millis() >= beepEindTijd) {
    noTone(buzzerPin); // Stop de toon
    beepEindTijd = 0;  // Reset de timer, zodat een nieuwe piep kan starten
  }
}

// Start een piep 
void beep(int frequency, int duration) {
  // Start alleen een nieuwe piep als er niet al een speelt
  if (beepEindTijd == 0) {
    tone(buzzerPin, frequency); // Start de toon (zonder duur-parameter)
    beepEindTijd = millis() + duration; // Stel in wanneer de toon moet stoppen
  }
}

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

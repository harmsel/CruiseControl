#include <Servo.h>

//150 graden = 90km/h op vlak

// Pinconfiguratie
const int plusKnop = 10;
const int minKnop = 9;
const int ledPin = 11;
const int servoPin = 2;

Servo myservo;
int servoHoek = 10;

const unsigned long stapInterval = 500;  // 2 per seconde = 500 ms

unsigned long vorigePlusTijd = 0;
unsigned long vorigeMinTijd = 0;

// --- Remschakelaar
int remSchakelaar = 12;

void setup() {
  pinMode(plusKnop, INPUT_PULLUP);
  pinMode(minKnop, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  pinMode(remSchakelaar, INPUT);
  Serial.begin(9600);
  myservo.attach(servoPin);
  servoHoek = constrain(servoHoek, 0, 175);  //beperk de servo in zijn beweging
}

void loop() {
  // Lees knopstatussen (actief laag)
  bool plusIngedrukt = digitalRead(plusKnop) == LOW;
  bool minIngedrukt = digitalRead(minKnop) == LOW;
  unsigned long nu = millis();

  // Plusknop ingedrukt → verhogen
  if (plusIngedrukt && nu - vorigePlusTijd >= stapInterval) {

    if (servoHoek>=100) {
      servoHoek += 4;
      Serial.print("servoHoek verhoogd: ");
      Serial.println(servoHoek);
    }else{
      servoHoek = 150;
    }

    ledFeedback();
    vorigePlusTijd = nu;
  }

  // Minknop ingedrukt → verlagen
  if (minIngedrukt && nu - vorigeMinTijd >= stapInterval) {
    servoHoek -= 4;
    ledFeedback();
    vorigeMinTijd = nu;
  }

  myservo.write(servoHoek);
  delay(10);
  remFunctie();
  Serial.print("servoHoek --: ");
  Serial.println(servoHoek);
}



////.  ----------- FUNCTIES ------------------
void ledFeedback() {
  digitalWrite(ledPin, HIGH);
  delay(200);  // kort aan
  digitalWrite(ledPin, LOW);
}


void remFunctie() {
  if (digitalRead(remSchakelaar) == HIGH) {
    Serial.println("Remschakelaar geactiveerd");
    ledFeedback();
    servoHoek = 10;
  }
}

void draaiServo() {

}

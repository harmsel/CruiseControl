#include <Servo.h>

//56 graden = 90km/h op vlak

// Pinconfiguratie
const int plusKnop = 10;
const int minKnop = 9;
const int ledPin = 11;
const int servoPin = 2;

Servo myservo;

int servoHoek = 20;

const unsigned long stapInterval = 500;  // 2 per seconde = 500 ms

unsigned long vorigePlusTijd = 0;
unsigned long vorigeMinTijd = 0;

void setup() {
  pinMode(plusKnop, INPUT_PULLUP);
  pinMode(minKnop, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);

  Serial.begin(9600);
  myservo.attach(servoPin);
  myservo.write(servoHoek);  // beginpositie
   Serial.print(" +++++++ RESET +++++ ");
}

void loop() {
  // Lees knopstatussen (actief laag)
  bool plusIngedrukt = digitalRead(plusKnop) == LOW;
  bool minIngedrukt  = digitalRead(minKnop)  == LOW;
  unsigned long nu = millis();

  // Plusknop ingedrukt → verhogen
  if (plusIngedrukt && nu - vorigePlusTijd >= stapInterval) {
    servoHoek += 2;
    servoHoek = constrain(servoHoek, 0, 180);  // evt. grens
    Serial.print("servoHoek verhoogd: ");
    Serial.println(servoHoek);
    myservo.write(servoHoek);
    ledFeedback();
    vorigePlusTijd = nu;
  }

  // Minknop ingedrukt → verlagen
  if (minIngedrukt && nu - vorigeMinTijd >= stapInterval) {
    servoHoek -= 2;
    servoHoek = constrain(servoHoek, 0, 66);
    Serial.print("servoHoek --: ");
    Serial.println(servoHoek);
    myservo.write(servoHoek);
    ledFeedback();
    vorigeMinTijd = nu;
  }
}

void ledFeedback() {
  digitalWrite(ledPin, HIGH);
  delay(200);  // halve seconde aan
  digitalWrite(ledPin, LOW);
}

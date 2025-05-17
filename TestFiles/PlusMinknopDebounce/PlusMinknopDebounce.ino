const int plusKnop = 10;
const int minKnop = 9;
const int ledPin = 11;

int pulseDoel = 20;

// Debounce-instellingen
const unsigned long debounceDelay = 50;

int lastPlusState = HIGH;
int lastMinState = HIGH;

int plusState;
int minState;

unsigned long lastDebounceTimePlus = 0;
unsigned long lastDebounceTimeMin = 0;


/// servo 
#include <Servo.h>

Servo myservo;  // create Servo object to control a servo
// twelve Servo objects can be created on most boards

int pos = 0;    // variable to store the servo position




void setup() {
  pinMode(plusKnop, INPUT_PULLUP);
  pinMode(minKnop, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);

  Serial.begin(9600);
    myservo.attach(2);
}

void loop() {
  int plusReading = digitalRead(plusKnop);
  int minReading = digitalRead(minKnop);

  // -------- Debounce voor plusKnop
  if (plusReading != lastPlusState) {
    lastDebounceTimePlus = millis();
  }

  if ((millis() - lastDebounceTimePlus) > debounceDelay) {
    if (plusReading != plusState) {
      plusState = plusReading;
      if (plusState == LOW) {
        pulseDoel += 2;
        Serial.print("pulseDoel verhoogd: ");
        Serial.println(pulseDoel);
        ledFeedback();
      }
    }
  }

  // ----------Debounce voor minKnop
  if (minReading != lastMinState) {
    lastDebounceTimeMin = millis();
  }

  if ((millis() - lastDebounceTimeMin) > debounceDelay) {
    if (minReading != minState) {
      minState = minReading;
      if (minState == LOW) {
        pulseDoel -= 2;
        Serial.print("pulseDoel verlaagd: ");
        Serial.println(pulseDoel);
        ledFeedback();
      }
    }
  }

  lastPlusState = plusReading;
  lastMinState = minReading;
}

void servoDraai() {
    myservo.write(pulseDoel);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15 ms for the servo to reach the position
}


void ledFeedback() {
  digitalWrite(ledPin, HIGH);
  delay(500);  // halve seconde aan
  digitalWrite(ledPin, LOW);
}

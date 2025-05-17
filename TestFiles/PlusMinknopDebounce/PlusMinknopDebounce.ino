const int plusKnop = 10;
const int minKnop = 9;
const int ledPin = 11;

int plusState, lastPlusState = HIGH;
int minState, lastMinState = HIGH;

unsigned long lastDebounceTimePlus = 0;
unsigned long lastDebounceTimeMin = 0;
const unsigned long debounceDelay = 50;

int pulseDoel = 20;  // Startwaarde

void setup() {
  pinMode(plusKnop, INPUT_PULLUP);
  pinMode(minKnop, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  int plusReading = digitalRead(plusKnop);
  int minReading = digitalRead(minKnop);

  // Debounce voor plusKnop
  if (plusReading != lastPlusState) {
    lastDebounceTimePlus = millis();
  }

  if ((millis() - lastDebounceTimePlus) > debounceDelay) {
    if (plusReading != plusState) {
      plusState = plusReading;
      if (plusState == LOW) {
        if (pulseDoel < 60) {
          pulseDoel++;
          Serial.print("pulseDoel verhoogd: ");
          Serial.println(pulseDoel);
          ledFeedback();
        } else {
          grensKnipper();
        }
      }
    }
  }

  // Debounce voor minKnop
  if (minReading != lastMinState) {
    lastDebounceTimeMin = millis();
  }

  if ((millis() - lastDebounceTimeMin) > debounceDelay) {
    if (minReading != minState) {
      minState = minReading;
      if (minState == LOW) {
        if (pulseDoel > 20) {
          pulseDoel--;
          Serial.print("pulseDoel verlaagd: ");
          Serial.println(pulseDoel);
          ledFeedback();
        } else {
          grensKnipper();
        }
      }
    }
  }

  lastPlusState = plusReading;
  lastMinState = minReading;
}

// Korte feedback bij wijziging
void ledFeedback() {
  digitalWrite(ledPin, HIGH);
  delay(50);
  digitalWrite(ledPin, LOW);
}

// Knipper 3 keer als grens bereikt is
void grensKnipper() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(ledPin, HIGH);
    delay(100);
    digitalWrite(ledPin, LOW);
    delay(100);
  }
}

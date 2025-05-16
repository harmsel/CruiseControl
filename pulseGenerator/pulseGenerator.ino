const int potPin = A0;  // Potentiometer ingang
const int pulsPin = 11;
const int ledPin = 13;
int pulsLengte = 10;

void setup() {
  pinMode(pulsPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  // Lees de waarde van de potentiometer (0 - 1023)
  int sensorValue = analogRead(potPin);

  float pulslaag = map(sensorValue, 0, 1023, 200, 2);

  // Puls:  hoog
  digitalWrite(pulsPin, HIGH);
  pinMode(ledPin, HIGH);
  delay(pulsLengte);  // vaste duur van de puls

  // Laag houden tot volgende puls
  digitalWrite(pulsPin, LOW);
  pinMode(ledPin, LOW);
  delay(pulslaag);

  // Debug info (optioneel)
  Serial.print("Potmeter: ");
  Serial.print(sensorValue);
  Serial.print(".  .. aantal pulsen per seconde");
  Serial.println(1000 / (pulslaag + pulsLengte));
}
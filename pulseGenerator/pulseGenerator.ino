const int potPin = A0;   // Potentiometer ingang
const int ledPin = 11;   // LED op pin 13

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  // Lees de waarde van de potentiometer (0 - 1023)
  int sensorValue = analogRead(potPin);

  float pulslaag = map(sensorValue, 0, 1023, 300,10);

  // Puls: 20 ms hoog
  digitalWrite(ledPin, HIGH);
  delay(10);  // vaste duur van de puls

  // Laag houden tot volgende puls
  digitalWrite(ledPin, LOW);
  delay(pulslaag);

  // Debug info (optioneel)
  Serial.print("Sensor: "); Serial.print(sensorValue);
  Serial.print("  pulstijd "); Serial.println(pulslaag);
}
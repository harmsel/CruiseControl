const int buzzerPin = 6;  

void setup() {
  pinMode(buzzerPin, OUTPUT);
}

void loop() {
  tone(buzzerPin, 500);   // Speel een toon van 1000 Hz
  delay(100);              // Duur van de piep: 100 milliseconden
  noTone(buzzerPin);       // Stop de toon
  delay(2900);              // Wacht de rest van de seconde (100 + 900 = 1000 ms)
}
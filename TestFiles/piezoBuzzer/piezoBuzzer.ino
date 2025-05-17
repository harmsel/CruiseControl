const byte speakerPin = 6;
unsigned long lastPeriodStart = 0;

void setup() {
  lastPeriodStart = millis(); // Starttijd initialiseren
}

void loop() {
piezoPieper(1, 2000);//eerste getal = lengte piep, 2e getal = tijd tussen een piep
}

void piezoPieper(const int onDuration, const int periodDuration){
  if (millis() - lastPeriodStart >= periodDuration) {
    lastPeriodStart += periodDuration;
    tone(speakerPin, 550, onDuration); // Speel een 550 Hz toon voor onDuration
  }
}
// wat moet de code doen?
// als er spanning op komt mag er niets gebeuren totdat zowel + als - worden ingedrukt gedurende 2 seconden
// Als vervolgens op + gedrukt wordt (en  is >30 Rotaties <55) gaat hij de de rotaties per (0.5 seconde) proberen te behouden
// Als koppeling/rem is ingedrukt. Gaat de servo naar 2 graden en reset het ingestelde RPM naar 0. 

// ----- FASE 2 -----
// Als de  er nogmaals op + gedruk wordt gaat hij proberen 1 omwenteling meer te doen





// 46 = 90km/h
// 49 = 95km/h
// 51 = 100km/h

const int analogPin = A0;
const int threshold = 500;  // 500 = 2.4 volt. signaalniveau (1023 = 5v)
unsigned int pulseCount = 0;
unsigned long lastTime = 0;
const int interval = 500; // dit is de interval waarbinnen de omwentelingen (Rotaties)teteld worden

bool pulseDetected = false;

void setup() {
  Serial.begin(9600);
  
}

void loop() {

//------------ AANZETTEN EN UITSCHAKELEN ------------- ///




//------------ PULSEN DETECTEREN ------------- ///
  int sensorValue = analogRead(analogPin);
  // Puls gedetecteerd als signaal boven drempel komt
  if (sensorValue > threshold) {
    
    if (!pulseDetected) {  // voorkom dubbeltellen tijdens 1 puls
      pulseCount++;
      pulseDetected = true;
    }
  } else {
    pulseDetected = false;
  }

  // Bereken rotaties elke halve seconde
  unsigned long currentTime = millis();
  if (currentTime - lastTime >= interval) {
    Serial.println(pulseCount);
    pulseCount = 0;
    lastTime = currentTime;
  }







}
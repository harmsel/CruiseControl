int remSchakelaar = 12;

void setup() {
  Serial.begin(9600);
  pinMode(remSchakelaar, INPUT_PULLUP);
}

void loop() {
  remFunctie();
    delay(20);
}

void remFunctie() {
  int waarde = digitalRead(remSchakelaar);
  Serial.println(waarde);


  if (digitalRead(remSchakelaar) == HIGH) {
    Serial.println("Remschakelaar geactiveerd");
  }

}
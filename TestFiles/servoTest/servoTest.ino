
#include <Servo.h>

Servo myservo;
int hoekKlein = 0;
int hoekGroot = 180;


int servoHoek = 0;  // variable to store the servo position

void setup() {
  myservo.attach(2);                         // attaches the servo on pin 9 to the Servo object
  servoHoek = constrain(servoHoek, 0, 175);  //beperk de servo in zijn beweging
}

void loop() {

  for (int x = hoekKlein; x < hoekGroot; x++) {
    for (servoHoek = 0; servoHoek <= hoekGroot; servoHoek += 1) {  
      myservo.write(servoHoek);  // tell servo to go to position in variable 'pos'
      delay(10);              
    }


    delay(100);
    for (servoHoek = hoekGroot; servoHoek >= hoekKlein; servoHoek -= 1) {  
      myservo.write(servoHoek);                                    // tell servo to go to position in variable 'pos'
      delay(10);                                                           
    }
    delay(100);
  }
}

#include <Servo.h>

int pirPin = 8;
int servoPin = 9;
int hareket;

Servo motor;

void setup() {

 motor.attach(servoPin);
 pinMode(pirPin, INPUT);

}

void loop() {

  hareket = digitalRead(pirPin);

  if(hareket == HIGH) 
  {
    motor.write(150);
    delay(250);
    motor.write(30);
    delay(250);
    motor.write(150);
    delay(250);
    motor.write(30);
    delay(250);
    motor.write(150);
    delay(250);
    motor.write(30);
    delay(250);
    motor.write(90);
  }
  else
  {
    motor.write(90);
  }

}

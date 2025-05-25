#include <Servo.h>
Servo motor;

#define joystick A0

void setup() {
  motor.attach(3);
  Serial.begin(9600);
  Serial.println("Joystick Değerini Okuma");
}

void loop() {
 
  int derece = map(analogRead(joystick), 0, 1023, 0, 180);
  motor.write(derece);

  Serial.print("Joystick degeri: ");
  Serial.print(joydeger);
  Serial.print(" ");
  Serial.print("Motor derecesi: ");
  Serial.print(derece);
  Serial.println();
}
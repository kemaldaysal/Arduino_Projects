#include <Servo.h>
Servo motor;
int i;
int g;

void setup() {

  motor.attach(3);
  Serial.begin(9600);
  Serial.println("Servo derecesini okuma");
}

void loop() {
  
  for (i = 0 ; i <= 180 ; i++)
  {
    motor.write(i);
    delay(10);
    
    Serial.print("Motorun su anki derecesi: ");
    Serial.print(i);
    Serial.println();
  }
  
  for (g = 180 ; g >= 0 ; g--)
  {
    motor.write(g);
    delay(10);
    
    Serial.print("Motorun su anki derecesi: ");
    Serial.print(g);
    Serial.println();
  }
}

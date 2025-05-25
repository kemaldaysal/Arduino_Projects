#include <LiquidCrystal_I2C.h>
#include <Wire.h> 

LiquidCrystal_I2C lcd(0x27, 16, 2);

int trigPin = 7;
int echoPin = 6;
int sure;
int uzaklik;

void setup() {

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  lcd.begin();
  lcd.backlight();

}

void loop() {

  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  sure = pulseIn(echoPin, HIGH, 11600);

  uzaklik = sure*0.0345/2;

  delay(250);

  lcd.clear();
  lcd.setCursor(0,0); 
  lcd.print("Uzaklik:");
  lcd.setCursor(0,1); //0. sütun 1. satır)
  lcd.print(uzaklik); 
  lcd.print("cm");

}

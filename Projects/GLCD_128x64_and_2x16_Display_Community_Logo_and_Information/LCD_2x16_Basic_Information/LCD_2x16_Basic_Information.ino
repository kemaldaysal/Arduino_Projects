#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27 , 16 , 2); // LCD address , scr width, scr hegiht

void setup() {

  lcd.begin();

}

void loop() {

  Instagram();
  Twitter();
  DetayliBilgi();

}

void Instagram()
{
  lcd.setCursor(0, 0);
  lcd.print("Instagram @");
  delay(500);
  lcd.setCursor(7, 1);
  lcd.print("iottrakya");
  delay(4000);
  lcd.clear();
}

void Twitter()
{
  lcd.setCursor(0, 0);
  lcd.print("Twitter @");
  delay(500);
  lcd.setCursor(7, 1);
  lcd.print("IoTTrakya");
  delay(4000);
  lcd.clear();
}

void DetayliBilgi()
{
  lcd.setCursor(2, 0);
  lcd.print("Dahasi icin");
  lcd.setCursor(0, 1);
  lcd.print("QR kodu taratin.");
  delay(3000);
  lcd.clear();
}

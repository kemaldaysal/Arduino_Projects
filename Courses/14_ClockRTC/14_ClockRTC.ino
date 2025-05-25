#include <virtuabotixRTC.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

int CLK_PIN = 6;
int DAT_PIN = 7;
int RST_PIN = 8;

virtuabotixRTC myRTC(CLK_PIN,DAT_PIN,RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  
  //zaman sırası saniye, dakika, saat, haftanın günü, ayın günü, ay, yıl
  //myRTC.setDS1302Time(30 , 00 , 13 , 1 , 20 , 1 , 2020);
  lcd.begin();
  lcd.backlight();
}

void loop() {

  myRTC.updateTime();
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(myRTC.dayofmonth);
  lcd.print("/");
  lcd.print(myRTC.month);
  lcd.print("/");
  lcd.print(myRTC.year);

  lcd.setCursor(0,1);
  lcd.print(myRTC.hours);
  lcd.print(":");
  lcd.print(myRTC.minutes);
  lcd.print(":");
  lcd.print(myRTC.seconds);

  delay(1000);

}

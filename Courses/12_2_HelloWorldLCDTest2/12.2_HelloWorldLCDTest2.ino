#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup()
{
	// initialize the LCD
	lcd.begin();

	// Turn on the blacklight and print a message.
	lcd.backlight();

}

void loop()
{

  lcd.clear();
  lcd.print("Robojax");
  lcd.setCursor(0,1);
  lcd.print("Hello World");
  delay(500);
	
}

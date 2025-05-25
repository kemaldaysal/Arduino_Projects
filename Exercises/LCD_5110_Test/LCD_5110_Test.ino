#include <LCD5110_Basic.h>

LCD5110 myGLCD(8,9,10,11,12);

extern uint8_t SmallFont[];
extern uint8_t MediumNumbers[];
extern uint8_t BigNumbers[];


void setup() {

  myGLCD.InitLCD();
}


void loop() {

  myGLCD.setFont(SmallFont);
  myGLCD.print("Hello World",0,0); // LCD 84x

}

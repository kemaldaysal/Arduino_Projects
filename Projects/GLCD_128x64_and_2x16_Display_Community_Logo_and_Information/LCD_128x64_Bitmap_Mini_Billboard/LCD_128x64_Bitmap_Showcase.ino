#include <openGLCD.h>
#include "QR_Code_B4_Bitmap.h"
#include "IoT_Logo_Minimal_x.h"
#include "insta_logo_16x16.h"
#include "wp_16x16.h"
#include "tw_16x16.h"
#include "tl_16x16.h"
#include "ln_16x16.h"
#include "dc_16x16.h"

void setup() {
  
GLCD.Init();

}

void loop() {

GLCD.DrawBitmap(IoT_Logo_Minimal_x,0,0);

delay(3000);

GLCD.ClearScreen();

GLCD.DrawBitmap(QR_Code_B4_Bitmap,32,0);

GLCD.DrawBitmap(insta_logo_16x16,8,2);
GLCD.DrawBitmap(tw_16x16,8,24);
GLCD.DrawBitmap(wp_16x16,8,46);

GLCD.DrawBitmap(dc_16x16,104,2);
GLCD.DrawBitmap(ln_16x16,104,24);
GLCD.DrawBitmap(tl_16x16,104,46);

delay(7000);
  
}

#include <IRremote.h>

#define led 3
#define alici_pin 2 

#define BUTON1a 0x1001
#define BUTON1b 0x1801

#define BUTON2a 0x1002  
#define BUTON2b 0x1802

void setup() {

  Serial.begin(9600);
  IrReceiver.begin(alici_pin, ENABLE_LED_FEEDBACK);

  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);
}

void loop() {

  if (IrReceiver.decode())
  {
    Serial.print("Kod = ");
    Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);

    if (IrReceiver.decodedIRData.decodedRawData == BUTON1a || IrReceiver.decodedIRData.decodedRawData == BUTON1b)
    {
      digitalWrite(led, HIGH);
    }

    else if (IrReceiver.decodedIRData.decodedRawData == BUTON2a || IrReceiver.decodedIRData.decodedRawData == BUTON2b)
    {
      digitalWrite(led, LOW);
    }

    IrReceiver.resume();
  }
}
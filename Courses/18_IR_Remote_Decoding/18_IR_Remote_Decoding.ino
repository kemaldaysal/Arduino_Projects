#include <IRremote.h>

IRrecv alici(2);
decode_results results;

#define BUTON1a 0x801
#define BUTON1b 0x1

#define BUTON2a 0x802
#define BUTON2b 0x2

#define led 3

// Sayı 16'lık sistemde (hexadecimal) 
// oldugundan başına 0x getirdik.
// İkili (binary) sayı sistemi olsaydı Bx 

void setup() {

  Serial.begin(9600);
  alici.enableIRIn();

  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);
}

void loop() {

  if (alici.decode(&results))
  {
    Serial.println(results.value, HEX);

    if (results.value == BUTON1a || results.value == BUTON1b) 
    {
      digitalWrite(led, HIGH); 
    }

    else if (results.value == BUTON2a || results.value == BUTON2b) 
    {
      digitalWrite(led, LOW);
    }
   
    alici.resume();
  }
}

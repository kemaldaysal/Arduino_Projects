#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(40,53); // CE, CSN

const int buzzer = 2;
const byte address[6] = "00001";

void setup() {
  pinMode(buzzer, OUTPUT);
  radio.begin();
  radio.openReadingPipe(0,address);
  radio.setPALevel(RF24_PA_MAX);    // MIN, LOW, HIGH, MAX  // MAX POWER LEVEL COMBINED WITH LOWEST DATA RATE GIVES MAX RANGE theoretically
  radio.setDataRate(RF24_250KBPS); // (250 KBPS, 1 MB/S, 2 MB/S)
  radio.startListening();
}

void loop() {
  char text[32] = "";
  
  if (radio.available()) {
    radio.read(&text, sizeof(text));
    String transData = String(text);
    if (transData == "nrftest") {
        tone(buzzer, 1000);
        delay(500);
        noTone(buzzer);
      }    
  }
}

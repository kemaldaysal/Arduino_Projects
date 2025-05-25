#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9,10);

const byte address[6] = "00001";

void setup() {
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MAX); // MIN, LOW, HIGH, MAX   // MAX POWER LEVEL COMBINED WITH LOWEST DATA RATE GIVES MAX RANGE theoretically
  radio.setDataRate(RF24_250KBPS); // (250 KBPS, 1 MB/S, 2 MB/S)
  radio.stopListening();
}

void loop() {
  const char* text[] = "nrftest";
  radio.write(&text, sizeof(text));
  delay(2000);
}

int ledler[] = { 2, 3, 4, 5, 6, 7 };

void setup() {

  for (int i = 0; i < 6; i++) {
    pinMode(ledler[i], OUTPUT);
  }
}

void loop() {

  for (int i = 0; i <= 5; i++) {
    digitalWrite(ledler[i], HIGH);
    delay(1000);
    digitalWrite(ledler[i], LOW);
  }

  for (int j = 5; j >= 0; j--) {
    digitalWrite(ledler[j], HIGH);
    delay(1000);
    digitalWrite(ledler[j], LOW);
  }
}

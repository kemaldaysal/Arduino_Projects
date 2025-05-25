int trigger_pin = 8;
int echo_pin = 9;
int buzzer = 2;

float sure;
float mesafe;

void setup()
{
  pinMode(trigger_pin, OUTPUT); // Verici
  pinMode(echo_pin, INPUT); // Alıcı

  pinMode(buzzer, OUTPUT);

  Serial.begin(9600);
}

void loop()
{
  digitalWrite(trigger_pin, HIGH);
  delay(1); // 1 milisaniye gecikme
  digitalWrite(trigger_pin, LOW);

  sure = pulseIn(echo_pin, HIGH); // pin takibi
  mesafe = (sure / 2) / 28.97 ;

  // 24 derecelik bir odada sesin hızı 345,24 metre/saniye
  // 28,97 mikrosaniye / santimetre

  Serial.print("Cisme olan uzaklık = ");
  Serial.print(mesafe);
  Serial.println(" cm");
  Serial.println("-----------------");

  if ( mesafe <= 7)
  {
    digitalWrite(buzzer, HIGH);
    delay(50);
    digitalWrite(buzzer, LOW);
    delay(50);  
  }

  else if ( mesafe <= 15)
  {
    digitalWrite(buzzer, HIGH);
    delay(50);
    digitalWrite(buzzer, LOW);
    delay(250);
  }

  else if ( mesafe <= 30)
  {
    digitalWrite(buzzer, HIGH);
    delay(50);
    digitalWrite(buzzer, LOW);
    delay(500);
  }

  else if ( mesafe <= 45 )
  {
    digitalWrite(buzzer, HIGH);
    delay(50);
    digitalWrite(buzzer, LOW);
    delay(750);
  }

}

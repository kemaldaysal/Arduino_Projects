// Tek potansiyometre olduğundan 3 kontrollü RGB yapılamadı.
// Pot alınca Ders 8'e tekrar bak.

#define red 11
#define green 10
#define blue 9

#define pot_r A0

void setup() {

pinMode(red,OUTPUT);
pinMode(green,OUTPUT);
pinMode(blue,OUTPUT);




}

void loop() {

  int red_value = analogRead(pot_r);


  red_value=map(red_value, 0,1023, 0,255);

  analogWrite(red,red_value);
  

}

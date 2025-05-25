int lm35Pin = A1;
int led = 8;
#define buzzer 9  //  veya int buzzer = 9

int zaman = 50;
int okunan_deger = 0;
float sicaklik_gerilim = 0;
float sicaklik = 0;

void setup() {

  pinMode(led,OUTPUT);
  pinMode(buzzer,OUTPUT);
  Serial.begin(9600);

}

void loop() {

  okunan_deger = analogRead(lm35Pin);
  sicaklik_gerilim = (5000.0/1023.0) * okunan_deger; /* Okunan deger / max okuma yapılabilen çözünürlük, sensör miliVolt olarak okur. 5V = 5000 mV. Okunan deger 100 gelirse (5000/1023) ile çarpılır. */
  sicaklik = sicaklik_gerilim / 10.0 ; /*lm35 her  santigrat derece için 10 miliVolt iletir.*/ 

  Serial.println(sicaklik);
  
  if(sicaklik >= 20)
  {
    digitalWrite(led,HIGH);
    digitalWrite(buzzer,HIGH);
    delay(zaman);

    digitalWrite(led,LOW);
    digitalWrite(buzzer,LOW);
    delay(zaman);  
  }

  else {
    digitalWrite(led,LOW);
    digitalWrite(buzzer,LOW);
    
    
  }





}

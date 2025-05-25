int kirmizi = 7;
int sari = 6;
int yesil = 5;

void setup() {

pinMode(kirmizi,OUTPUT);
pinMode(sari,OUTPUT);
pinMode(yesil,OUTPUT);
}

void loop() {

digitalWrite(kirmizi,HIGH);
delay(5000); //5000 milisaniye = 5 saniye
digitalWrite(kirmizi,LOW);

digitalWrite(sari,HIGH);
delay(2000); 
digitalWrite(sari,LOW);

digitalWrite(yesil,HIGH);
delay(5000); 
digitalWrite(yesil,LOW);

digitalWrite(sari,HIGH);
delay(2000);  
digitalWrite(sari,LOW);
}  

int esikDegeri = 1023;

int sensorPin = A0;
int buzzerPin = 8;

int veri;


void setup() {
  pinMode(buzzerPin,OUTPUT);
}

void loop() {

  veri = analogRead(sensorPin);

  if(veri > esikDegeri){
    
    digitalWrite(buzzerPin,HIGH);
    delay(100);
    digitalWrite(buzzerPin,HIGH);
    delay(100);
  }

  else{
    digitalWrite(sensorPin,LOW);
  }

}

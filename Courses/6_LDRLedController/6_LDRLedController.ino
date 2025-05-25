#define led 11

void setup() {

  pinMode(led,OUTPUT);
  Serial.begin(9600);
}

void loop() {

int isik = analogRead(A0);
Serial.println(isik);
delay(50);

if(isik>300){
  digitalWrite(led,LOW);
}

if(isik<220){
  digitalWrite(led,HIGH);
}
}

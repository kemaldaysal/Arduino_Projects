#define echoPin 6
#define trigPin 7
#define buzzerPin 8

int maximumRange = 50;
int minimumRange = 0;

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);

}

void loop() {

  int olcum = mesafe(maximumRange, minimumRange);
  melodi(olcum*10);

}

int mesafe(int maxrange, int minrange)
{
  long duration,distance;

   digitalWrite(trigPin, LOW);
   delayMicroseconds(2);
   digitalWrite(trigPin, HIGH);
   delayMicroseconds(10);
   digitalWrite(trigPin, LOW);


   duration = pulseIn(echoPin,HIGH); // Echo pini 5V(High olduğunda) geçen süreyi ölç ve bana ver.
   distance = duration / 58.2;
   delay(50);

   if(distance >= maxrange || distance <= minrange)
   return 0; // iften sonra {} açmazsan sadece alt satırındaki komutu uygular, komut istediği gibi değilse else için 2 satır alttaki komutu uygular.
   return distance; // else oldu
}

int melodi(int dly)
{
  tone(buzzerPin, 440);
  delay(dly);
  noTone(buzzerPin);
  delay(dly);
}

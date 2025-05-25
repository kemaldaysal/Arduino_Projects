#include <stdint.h>

#define pin_relay ((uint8_t) 6 )
#define pin_pot A0

const uint16_t time_on = (40 * 1000); // seconds to milliseconds
uint32_t time_off = (10 * 60000); // minutes to milliseconds
#define startup_delay ((uint16_t) 3000)


bool debugModeEnabled = false; 
bool potEnabled = false;

unsigned long previousMillis = 0;
unsigned long currentMillis = 0;

char buf_serial[75];

void setup()
{

  pinMode(pin_relay, OUTPUT);

  delay(startup_delay);

  digitalWrite(pin_relay, HIGH); // added later
  previousMillis = millis(); // added later

  if (debugModeEnabled)
  {
    Serial.begin(9600);
    Serial.println("Starting");
  }
  
}

void loop()
{
  if (debugModeEnabled)
  {
    sprintf(buf_serial, "time passed: %lu sec, off time: %lu minutes", currentMillis/1000, time_off/60000);
    Serial.println(buf_serial);
  }
  
  if (potEnabled)
  {
    time_off = ((map(analogRead(pin_pot), 0, 1023, 8, 12)) * 60000);
  }

  currentMillis = millis();

  if (digitalRead(pin_relay) == HIGH) 
  {
    if ((currentMillis - previousMillis) >= time_on)
    {
      digitalWrite(pin_relay, LOW);
      previousMillis = currentMillis;
    }
  }
  else
  {
    if ((currentMillis - previousMillis) >= time_off)
    {
      digitalWrite(pin_relay, HIGH);
      previousMillis = currentMillis;
    } 
  }
}

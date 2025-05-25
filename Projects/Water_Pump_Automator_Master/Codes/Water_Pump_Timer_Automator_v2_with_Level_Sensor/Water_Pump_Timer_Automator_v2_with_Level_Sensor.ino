#include <stdint.h>

#define pin_relay ((uint8_t) 6 )
#define pin_pot A0
#define pin_levelsensor A1


const uint16_t time_on = (10 * 1000); // seconds to milliseconds
uint32_t time_off = (0.5 * 60000); // minutes to milliseconds
#define startup_delay ((uint16_t) 3000)
#define water_level_threshold ((uint16_t) 430)

bool debugModeEnabled = true;
bool potEnabled = false;
bool levelSensorEnabled = true;

unsigned long previousMillis = 0;
unsigned long currentMillis = 0;

char buf_serial[100];

void setup()
{

  pinMode(pin_relay, OUTPUT);
  pinMode(pin_pot, INPUT);
  pinMode(pin_levelsensor, INPUT);

  delay(startup_delay);

  digitalWrite(pin_relay, HIGH); // added later for startup activation
  previousMillis = millis(); // added later for startup activation

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
      sprintf(buf_serial, "time passed: %lu sec, off time: %lu minutes, level sensor data: %d, relayState: %d", currentMillis/1000, time_off/60000, analogRead(pin_levelsensor), digitalRead(pin_relay));
      Serial.println(buf_serial);
    }

    if ((levelSensorEnabled) && (analogRead(pin_levelsensor) <= water_level_threshold))
    {
      
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
    else if ((levelSensorEnabled) && (analogRead(pin_levelsensor) > water_level_threshold))
    {
      digitalWrite(pin_relay, LOW);
      previousMillis = 0;
      currentMillis = 0;
    }
}

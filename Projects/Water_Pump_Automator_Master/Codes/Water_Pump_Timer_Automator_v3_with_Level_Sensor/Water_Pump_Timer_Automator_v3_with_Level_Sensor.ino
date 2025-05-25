#include <stdint.h>

#define pin_relay ((uint8_t) 6 )
#define pin_pot A0
#define pin_levelsensor A1

const uint16_t time_on = (40 * 1000); // seconds to milliseconds // REAL TIMER 
uint32_t time_off = (10 * 60000); // minutes to milliseconds // REAL TIMER

/*
const uint16_t time_on = (3 * 1000); // seconds to milliseconds // FOR QUICK TESTS
uint32_t time_off = (0.2 * 60000); // minutes to milliseconds // FOR QUICK TESTS
*/
#define startup_delay ((uint16_t) 3000) // wait 3 seconds after powering-up
#define water_level_threshold ((uint16_t) 430) // first 430, then tried 280

bool debugModeEnabled = true;
bool potEnabled = false;
bool levelSensorEnabled = true;

uint64_t previousMillis = 0;
uint64_t currentMillis = 0;

char buf_serial[100];

void setup()
{

  pinMode(pin_relay, OUTPUT);
  pinMode(pin_pot, INPUT);
  pinMode(pin_levelsensor, INPUT);
  
  if (debugModeEnabled)
  {
    Serial.begin(9600);
    Serial.println("Starting");
  }

  //digitalWrite(pin_relay, LOW); // ensuring the motor isn't powered at startup before the startup delay but isn't necessary in this condition

  delay(startup_delay);

  digitalWrite(pin_relay, HIGH); // added later for startup activation
  previousMillis = millis(); // added later for startup activation
  
}

void loop()
{

    if (potEnabled)
    {
        time_off = ((map(analogRead(pin_pot), 0, 1023, 8, 12)) * 60000);
    }

    if (debugModeEnabled)
    {
      sprintf(buf_serial, "Seconds passed: %lu, relayState: %d, level sensor data: %d, ", millis()/1000, digitalRead(pin_relay), analogRead(pin_levelsensor));
      Serial.println(buf_serial);
    }

    if ((levelSensorEnabled) && (analogRead(pin_levelsensor) <= water_level_threshold)) // if tanker is not full, GO ON
    {

      currentMillis = millis();

      if (digitalRead(pin_relay) == 1) 
      {
        if ((currentMillis - previousMillis) >= time_on)
        {
          digitalWrite(pin_relay, 0);
          previousMillis = currentMillis;
        }
      }
      else
      {
        if ((currentMillis - previousMillis) >= time_off)
        {
          digitalWrite(pin_relay, 1);
          previousMillis = currentMillis;
        } 
      }
    }
    else if ((levelSensorEnabled) && (analogRead(pin_levelsensor) > water_level_threshold)) // if tanker is full, STOP
    {
      digitalWrite(pin_relay, 0);
      previousMillis = millis();
      // previousMillis = 0;
      // currentMillis = 0;
    }
}
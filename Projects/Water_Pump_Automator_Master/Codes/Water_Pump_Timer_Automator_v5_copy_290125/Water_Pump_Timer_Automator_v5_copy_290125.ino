#include <stdint.h>

// --- setting timers

#define time_on_in_seconds ((uint8_t)40)             // 40 seconds time_on
#define time_off_in_minutes ((uint8_t)10)            // 10 minutes time_on
#define fast_test_time_on_in_seconds ((uint8_t)10)   // 10 seconds time_on ONLY IN TEST MODE
#define fast_test_time_off_in_seconds ((uint8_t)10)  // 10 seconds time_off ONLY IN TEST MODE
#define startup_delay ((uint16_t)3000)               // wait 3 seconds after powering-up the Arduino

// --- important settings

bool potEnabled = false;                       // enable potentiometer for changing off-time
bool levelSensorEnabled = true;               // enable water level sensor
#define water_level_threshold ((uint16_t)430)  // first 430, then tried 280

// --- setting for testing & troubleshooting

bool debugModeEnabled = true;                  // display debug logs on serial monitor // DON'T FORGET TO DISABLE THIS BEFORE FINAL UPLOAD TO THE CARD
bool fastTestModeEnabled = true;               // 10 seconds on / 10 seconds off test mode // DON'T FORGET TO DISABLE THIS BEFORE FINAL UPLOAD TO THE CARD

// --- pin connections

#define pin_relay ((uint8_t)6)
#define pin_pot A0
#define pin_levelsensor A1

// --- initializations and buffer declarations

char buf_serial[100];
uint64_t previousMillis = 0;
uint64_t currentMillis = 0;
uint16_t time_on = ((uint16_t)time_on_in_seconds * 1000);     // seconds to milliseconds
uint32_t time_off = ((uint32_t)time_off_in_minutes * 60000);  // minutes to milliseconds

void setup() {
  pinMode(pin_relay, OUTPUT);
  pinMode(pin_pot, INPUT);
  pinMode(pin_levelsensor, INPUT);

  if (fastTestModeEnabled) {
    time_on = ((uint16_t)10 * 1000);   // 10 seconds to milliseconds // REAL TIMER
    time_off = ((uint32_t)10 * 1000);  // 10 seconds to milliseconds // REAL TIMER
  }

  if (debugModeEnabled) {
    Serial.begin(9600);
    Serial.println("Starting");
  }

  //digitalWrite(pin_relay, LOW); // ensuring the motor isn't powered at startup before the startup delay but isn't necessary in this condition

  delay(startup_delay);

  digitalWrite(pin_relay, HIGH);  // added later for startup activation
  previousMillis = millis();      // added later for startup activation
}

void loop() {
  if (potEnabled) {
    time_off = ((map(analogRead(pin_pot), 0, 1023, 8, 12)) * 60000);  // work on it, think about not doing mapping but handling values in if statements for precise regions
  }

  if (debugModeEnabled) {
    sprintf(buf_serial, "Seconds passed: %lu, relayState: %d, level sensor data: %d", (((millis()-startup_delay) / 1000)), digitalRead(pin_relay), analogRead(pin_levelsensor));
    Serial.println(buf_serial);
  }

  if (levelSensorEnabled) // if level sensor is used
  {                                    
    if (analogRead(pin_levelsensor) <= water_level_threshold)  // if tanker is not full, GO ON
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
    else if (analogRead(pin_levelsensor) > water_level_threshold)  // if tanker is full, STOP
    {
      digitalWrite(pin_relay, 0);
      previousMillis = millis();
    }
  } 
  else  // if level sensor isn't used
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
}
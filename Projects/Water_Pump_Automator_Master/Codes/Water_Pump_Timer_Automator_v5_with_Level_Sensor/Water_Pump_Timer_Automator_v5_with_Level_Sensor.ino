#include <stdint.h>

// --- setting timers

const uint8_t time_on_in_seconds = 40;             // 40 seconds time_on
const uint8_t time_off_in_minutes = 10;             // 40 seconds time_on

const uint8_t fast_test_time_on_in_seconds = 10;   // 10 seconds time_on ONLY IN TEST MODE
const uint8_t fast_test_time_off_in_seconds = 10;  // 10 seconds time_off ONLY IN TEST MODE
const uint16_t startup_delay = 3000;               // wait 3 seconds after powering-up the Arduino

// --- important settings

bool potEnabled = false;                       // enable potentiometer for changing off-time
bool levelSensorEnabled = true;               // enable water level sensor
const uint16_t water_level_threshold = 430;  // first 430, then tried 280

// --- setting for testing & troubleshooting

bool debugModeEnabled = true;                  // display debug logs on serial monitor // DON'T FORGET TO DISABLE THIS BEFORE FINAL UPLOAD TO THE CARD
bool fastTestModeEnabled = true;               // 10 seconds on / 10 seconds off test mode // DON'T FORGET TO DISABLE THIS BEFORE FINAL UPLOAD TO THE CARD

// --- pin connections

#define pin_relay 6
#define pin_pot A0
#define pin_levelsensor A1

// --- shortcuts and macros for better readability
#define RELAY_ON HIGH
#define RELAY_OFF LOW
#define SECONDS_TO_MILLISECONDS 1000
#define MINUTES_TO_MILLISECONDS 60000

// --- Global variables

char buf_serial[100];
uint64_t previousMillis = 0;
uint64_t currentMillis = 0;
uint16_t time_on = ((uint16_t)time_on_in_seconds * SECONDS_TO_MILLISECONDS);     // seconds to milliseconds
uint32_t time_off = ((uint32_t)time_off_in_minutes * MINUTES_TO_MILLISECONDS);  // minutes to milliseconds
uint16_t potValue = 0;

// --- function prototypes
void ControlPump(void);

// -------- Start of void setup()

void setup() {
  pinMode(pin_relay, OUTPUT);
  pinMode(pin_pot, INPUT);
  pinMode(pin_levelsensor, INPUT);

  digitalWrite(pin_relay, RELAY_OFF);

  if (fastTestModeEnabled) {
    time_on = fast_test_time_on_in_seconds * SECONDS_TO_MILLISECONDS;   // 10 seconds to milliseconds // REAL TIMER
    time_off = fast_test_time_off_in_seconds * SECONDS_TO_MILLISECONDS;  // 10 seconds to milliseconds // REAL TIMER
  }

  if (debugModeEnabled) {
    Serial.begin(9600);
    Serial.println("System starting...");
  }

  //digitalWrite(pin_relay, LOW); // ensuring the motor isn't powered at startup before the startup delay but isn't necessary in this condition

  delay(startup_delay);

  digitalWrite(pin_relay, RELAY_ON);  // added later for startup activation
  previousMillis = millis();      // added later for startup activation
}

// -------- End of void setup()

// -------- Start of void loop()

void loop() {
  if (potEnabled) // setting time_off by pot values
  {
    potValue = analogRead(pin_pot);

    if (potValue < 256) 
    {
      time_off = 8 * MINUTES_TO_MILLISECONDS;
    }
    else if (potValue < 512)
    {
      time_off = 10 * MINUTES_TO_MILLISECONDS;
    }
    else if (potValue < 768)
    {
      time_off = 12 * MINUTES_TO_MILLISECONDS;
    }
    else 
    {
      time_off = 30 * MINUTES_TO_MILLISECONDS; 
    }
  }

  if (debugModeEnabled) {
    sprintf(buf_serial, "Time: %lu s, Relay: %d, Level Sensor: %d-%d, Pot: %d-%d", (((millis()-startup_delay) / 1000)), digitalRead(pin_relay), levelSensorEnabled, analogRead(pin_levelsensor), potEnabled, potValue);
    Serial.println(buf_serial);
  }

  if (levelSensorEnabled) // if level sensor is used
  {    
    uint16_t levelSensorValue = analogRead(pin_levelsensor);

    if (levelSensorValue <= water_level_threshold)  // if tanker is not full, GO ON
    {
      ControlPump();
    } 
    else // if tanker is full, STOP and wait till it's not full and then wait for time_off time
    {
      digitalWrite(pin_relay, RELAY_OFF);
      previousMillis = millis();
    }
  } 
  else  // if level sensor isn't used
  {
    ControlPump();
  }
}
// -------- End of void loop()

// ------- Start of custom function implementations 
void ControlPump(void)
{
    currentMillis = millis();
    if (digitalRead(pin_relay) == RELAY_ON) 
    {
      if ((currentMillis - previousMillis) >= time_on) 
      {
        digitalWrite(pin_relay, RELAY_OFF);
        previousMillis = currentMillis;
      }
    } 
    else 
    {
      if ((currentMillis - previousMillis) >= time_off) 
      {
        digitalWrite(pin_relay, RELAY_ON);
        previousMillis = currentMillis;
      }
    }
}

// ------- End of custom function implementations 
#include <stdint.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <LowPower.h>

// TO DO
/*

  1) Debugmode macro by defines
  2) time_on gibi global değişkenleri minimize etmenin yolunu bul


*/

// --- important settings

namespace S_FEATURES {
constexpr bool POT_ENABLED = true;           // enable potentiometer for changing off-time
constexpr bool LEVEL_SENSOR_ENABLED = true;  // enable water level sensor
constexpr bool SLEEP_OPTION_ENABLED = false;
// const uint16_t water_level_threshold = 400; // only needed with analog sensor  // 430 -> 280 -> 265 -> 400 // don't go below 200 !!
}


// --- setting for testing & troubleshooting
namespace S_DEBUG {
constexpr bool DEBUG_MODE_ENABLED = false;      // display debug logs on serial monitor // DON'T FORGET TO DISABLE THIS BEFORE FINAL UPLOAD TO THE CARD
constexpr bool FAST_TEST_MODE_ENABLED = false;  // 10 seconds on / 10 seconds off test mode // DON'T FORGET TO DISABLE THIS BEFORE FINAL UPLOAD TO THE CARD
}

#if DEBUG_MODE_ENABLED
#define LOG(x) Serial.print(x)
#define LOG_LN(x) Serial.println(x)
#define LOG_BEGIN(baudrate) Serial.begin(baudrate)
#else
#define LOG(x)
#define LOG_LN(x)
#define LOG_BEGIN(baudrate)
#endif

// --- setting timers
namespace S_TIMERS_DELAYS {
constexpr uint8_t TIME_ON_IN_S = 40;   // 40 seconds time_on as default
constexpr uint8_t TIME_OFF_IN_M = 10;  // 10 minutes time_off as default

constexpr uint8_t FT_TIME_ON_IN_S = 4;   // 10 seconds time_on ONLY IN TEST MODE
constexpr uint8_t FT_TIME_OFF_IN_S = 4;  // 10 seconds time_off ONLY IN TEST MODE

constexpr uint16_t STARTUP = 3000;  // wait 3 seconds after powering-up the Arduino
constexpr uint16_t DEBUG_PERIOD_IN_S = 1000;

// -- time-off options
constexpr uint32_t POT_CHECK_INTERVAL = 0.5 * 60000;  // 1 minutes interval for reading potentiometer (300,000 ms)
constexpr uint8_t TIME_OFF_OPT_1 = 5;                 // 5 minutes time off in pot option 1
constexpr uint8_t TIME_OFF_OPT_2 = 10;                // 10 minutes time off in pot option 2
constexpr uint8_t TIME_OFF_OPT_ANTIFREEZE = 30;       // 30 minutes time off in anti-freeze mode

constexpr uint16_t TIME_OFF_OPT_1_SCALE = (1023 / 3) * 1;
constexpr uint16_t TIME_OFF_OPT_2_SCALE = (1023 / 3) * 2;
}

namespace S_COMMS {
constexpr uint16_t S_BAUDRATE = 9600;
}

namespace PIN {
constexpr uint8_t LEVEL_SENSOR = 2;
constexpr uint8_t RELAY = 3;  // CHANGED FROM 4 TO 3 !!
constexpr uint8_t LED_LEVEL_FULL = LED_BUILTIN;
constexpr uint8_t POT = A7;
}

// --- shortcuts and macros for better readability

namespace RELAY {
constexpr bool ON = 0;
constexpr bool OFF = 1;
}

namespace CONVERSIONS {

constexpr uint16_t SECONDS_TO_MILLISECONDS = 1000;
constexpr uint16_t MINUTES_TO_MILLISECONDS = 60000;
}

constexpr uint8_t BUF_SIZE = 100;

// --- Global variables

volatile bool wakeUpFlag = false;  // Shared variable between ISR and main loop
char buf_serial[BUF_SIZE];
uint64_t previousMillis = 0;
uint64_t currentMillis = 0;
uint16_t time_on = ((uint16_t)S_TIMERS_DELAYS::TIME_ON_IN_S * CONVERSIONS::SECONDS_TO_MILLISECONDS);    // seconds to milliseconds
uint32_t time_off = ((uint32_t)S_TIMERS_DELAYS::TIME_OFF_IN_M * CONVERSIONS::MINUTES_TO_MILLISECONDS);  // minutes to milliseconds
uint32_t lastPotReadingMillis = 0;

// --- function prototypes
void ControlPump(void);
void EnterSleepMode(void);
void WakeUp(void);

// -------- Start of void setup()

void setup() {
  pinMode(PIN::RELAY, OUTPUT);
  pinMode(PIN::POT, INPUT);
  pinMode(PIN::LEVEL_SENSOR, INPUT);  // DON'T FORGET TO PUT A 4.7 - 10 kOhm PULL-UP RESISTOR !!
  pinMode(PIN::LED_LEVEL_FULL, OUTPUT);

  digitalWrite(PIN::RELAY, RELAY::OFF);  // Ensuring relay is off at startup
  digitalWrite(PIN::LED_LEVEL_FULL, 0);

  LOG_BEGIN(S_COMMS::S_BAUDRATE);
  LOG_LN("\nSystem starting...")

  uint32_t startTime = millis();
  while ((millis() - startTime) < S_TIMERS_DELAYS::STARTUP) {}

  uint16_t potValue = analogRead(PIN::POT);

  // Adjust time_off based on potentiometer reading at startup
  if (potValue < S_TIMERS_DELAYS::TIME_OFF_OPT_1_SCALE) {
    time_off = S_TIMERS_DELAYS::TIME_OFF_OPT_1 * CONVERSIONS::MINUTES_TO_MILLISECONDS;
  } else if (potValue < S_TIMERS_DELAYS::TIME_OFF_OPT_2_SCALE) {
    time_off = S_TIMERS_DELAYS::TIME_OFF_OPT_2 * CONVERSIONS::MINUTES_TO_MILLISECONDS;
  } else {
    time_off = S_TIMERS_DELAYS::TIME_OFF_OPT_ANTIFREEZE * CONVERSIONS::MINUTES_TO_MILLISECONDS;  // anti-freeze mode
  }

  if (S_DEBUG::FAST_TEST_MODE_ENABLED) {
    time_on = S_TIMERS_DELAYS::FT_TIME_ON_IN_S * CONVERSIONS::SECONDS_TO_MILLISECONDS;    // 10 seconds to milliseconds // REAL TIMER
    time_off = S_TIMERS_DELAYS::FT_TIME_OFF_IN_S * CONVERSIONS::SECONDS_TO_MILLISECONDS;  // 10 seconds to milliseconds // REAL TIMER
  }

  if (!(digitalRead(PIN::LEVEL_SENSOR))) {
    digitalWrite(PIN::RELAY, RELAY::ON);  // added later for startup activation
    previousMillis = millis();            // added later for startup activation
  }
}

// -------- End of void setup()

// -------- Start of void loop()

void loop() {

  if (S_FEATURES::SLEEP_OPTION_ENABLED && wakeUpFlag) {
    wakeUpFlag = false;
    LOG_LN("Waking up...");
  }

  if (S_DEBUG::DEBUG_MODE_ENABLED) {
    static uint32_t lastDebugTime = 0;
    if ((millis() - lastDebugTime) >= S_TIMERS_DELAYS::DEBUG_PERIOD_IN_S) {
      lastDebugTime = millis();
      snprintf(buf_serial, sizeof(buf_serial), "Time: %lu s, Relay: %d, Level Sensor: %d-%d-%d, Pot: %d-%d, Time_off: %d min",
               (((millis() - S_TIMERS_DELAYS::STARTUP) / 1000)), !(digitalRead(PIN::RELAY)),
               S_FEATURES::LEVEL_SENSOR_ENABLED, digitalRead(PIN::LEVEL_SENSOR), digitalRead(PIN::LED_LEVEL_FULL),
               S_FEATURES::POT_ENABLED, analogRead(PIN::POT), (time_off / 60000));

      LOG_LN(buf_serial);
      Serial.flush();
    }
  }

  if (S_FEATURES::POT_ENABLED && (!S_DEBUG::FAST_TEST_MODE_ENABLED) && ((millis() - lastPotReadingMillis) >= S_TIMERS_DELAYS::POT_CHECK_INTERVAL))  // setting time_off by pot values
  {
    lastPotReadingMillis = millis();
    uint16_t potValue = analogRead(PIN::POT);

    if (potValue < S_TIMERS_DELAYS::TIME_OFF_OPT_1_SCALE) {
      time_off = S_TIMERS_DELAYS::TIME_OFF_OPT_1 * CONVERSIONS::MINUTES_TO_MILLISECONDS;
    } else if (potValue < S_TIMERS_DELAYS::TIME_OFF_OPT_2_SCALE) {
      time_off = S_TIMERS_DELAYS::TIME_OFF_OPT_2 * CONVERSIONS::MINUTES_TO_MILLISECONDS;
    } else  // anti-freeze mode
    {
      time_off = S_TIMERS_DELAYS::TIME_OFF_OPT_ANTIFREEZE * CONVERSIONS::MINUTES_TO_MILLISECONDS;  // anti-freeze mode
      ControlPump();
      digitalWrite(PIN::LED_LEVEL_FULL, digitalRead(PIN::LEVEL_SENSOR));
      return;
    }
  }

  if (S_FEATURES::LEVEL_SENSOR_ENABLED)  // if level sensor is used
  {
    // uint16_t levelSensorValue = analogRead(PIN::LEVEL_SENSOR);

    if (!(digitalRead(PIN::LEVEL_SENSOR)))  // if water tank is not full, proceed normally
    {
      ControlPump();
      digitalWrite(PIN::LED_LEVEL_FULL, LOW);
    } else  // if tank is full, STOP and wait till it's not full and then wait for time_off time
    {
      digitalWrite(PIN::RELAY, RELAY::OFF);
      previousMillis = millis();
      digitalWrite(PIN::LED_LEVEL_FULL, HIGH);
      // digitalWrite(pin_led_relay_state, 0);
      if (S_FEATURES::SLEEP_OPTION_ENABLED) {
        if (S_DEBUG::DEBUG_MODE_ENABLED) {
          LOG_LN("Tank full, sleeping...");
          Serial.flush();  // Wait for transmission to complete before sleeping
        }
        EnterSleepMode();
      }
    }
  } else  // No level sensor, just use the timer
  {
    ControlPump();
  }
}
// -------- End of void loop()

// ------- Start of custom function implementations
void ControlPump() {
  currentMillis = millis();
  if (digitalRead(PIN::RELAY) == RELAY::ON) {
    if ((currentMillis - previousMillis) >= time_on) {
      digitalWrite(PIN::RELAY, RELAY::OFF);
      previousMillis = currentMillis;
    }
  } else {
    if ((currentMillis - previousMillis) >= time_off) {
      digitalWrite(PIN::RELAY, RELAY::ON);
      previousMillis = currentMillis;
    }
  }
}

void EnterSleepMode() {
  sleep_enable();

  attachInterrupt(digitalPinToInterrupt(PIN::LEVEL_SENSOR), WakeUp, LOW);  // Interrupt on falling edge (sensor goes LOW, tank becomes not full)

  // LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_ON);
}

void WakeUp() {

  static uint32_t lastWakeTime = 0;
  uint32_t now = millis();

  if (now - lastWakeTime < 200)  // 200ms debounce
  {
    return;  // Ignore rapid successive interrupts
  }

  lastWakeTime = now;
  wakeUpFlag = true;  // set flag instead of using Serial inside ISR

  sleep_disable();                                            // Disable sleep mode after waking up
  detachInterrupt(digitalPinToInterrupt(PIN::LEVEL_SENSOR));  // Detach the interrupt to prevent unnecessary triggers

  pinMode(PIN::LEVEL_SENSOR, INPUT);
}

// ------- End of custom function implementations
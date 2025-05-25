// TO DO
/*

  1) Debugmode macro by defines
  2) time_on gibi global değişkenleri minimize etmenin yolunu bul

*/

#include <stdint.h>

// --- Feature toggles

#define POT_ENABLED false           // enable potentiometer for changing off-time
#define LEVEL_SENSOR_ENABLED true  // enable water level sensor

// --- setting for testing & troubleshooting

#define DEBUG_MODE_ENABLED true       // display debug logs on serial monitor // DON'T FORGET TO DISABLE THIS BEFORE FINAL UPLOAD TO THE CARD
#define FAST_TEST_MODE_ENABLED false  // 10 seconds on / 10 seconds off test mode // DON'T FORGET TO DISABLE THIS BEFORE FINAL UPLOAD TO THE CARD

#if DEBUG_MODE_ENABLED
#define LOG(x) Serial.print(x)
#define LOG_LN(x) Serial.println(x)
#define LOG_BEGIN(baudrate) Serial.begin(baudrate)
#else
#define LOG(x)
#define LOG_LN(x)
#define LOG_BEGIN(baudrate)
#endif

namespace POT {
constexpr uint16_t MAX_VALUE = 1023;
constexpr uint8_t NUM_RANGES = 3;
constexpr uint16_t RANGE = MAX_VALUE / NUM_RANGES;
}

namespace CONVERSIONS {

constexpr uint16_t SECONDS_TO_MILLISECONDS = 1000;
constexpr uint16_t MINUTES_TO_MILLISECONDS = 60000;
}

// --- setting timers
namespace S_TIMERS_DELAYS {
constexpr uint8_t TIME_ON_IN_S = 33;   // 40 -> 33 seconds time_on as default
constexpr uint8_t TIME_OFF_IN_M = 15;  // 10 -> 15 minutes time_off as default

constexpr uint8_t FT_TIME_ON_IN_S = 4;   // 10 seconds time_on ONLY IN TEST MODE
constexpr uint8_t FT_TIME_OFF_IN_S = 4;  // 10 seconds time_off ONLY IN TEST MODE

constexpr uint16_t STARTUP = 3000;  // wait 3 seconds after powering-up the Arduino
constexpr uint16_t DEBUG_PERIOD_IN_S = 1000;

// -- time-off options
constexpr uint32_t POT_CHECK_INTERVAL = 0.5 * CONVERSIONS::MINUTES_TO_MILLISECONDS;  // minutes interval for reading potentiometer (300,000 ms)
constexpr uint8_t TIME_OFF_OPT_1 = 5;                                                // 5 minutes time off in pot option 1
constexpr uint8_t TIME_OFF_OPT_2 = 10;                                               // 10 minutes time off in pot option 2
constexpr uint8_t TIME_OFF_OPT_ANTIFREEZE = 30;                                      // 30 minutes time off in anti-freeze mode

constexpr uint16_t TIME_OFF_OPT_1_SCALE = POT::RANGE * 1;
constexpr uint16_t TIME_OFF_OPT_2_SCALE = POT::RANGE * 2;
}

namespace S_COMMS {
constexpr uint16_t S_BAUDRATE = 9600;
}

namespace PIN {
constexpr uint8_t LEVEL_SENSOR = 2;
//constexpr uint8_t RELAY = 6;  // 6 in Uno
constexpr uint8_t RELAY = 3;  // 3 in Nano
constexpr uint8_t LED_LEVEL_FULL = LED_BUILTIN;
constexpr uint8_t POT = A7;
}

// --- shortcuts and macros for better readability

namespace RELAY {
//constexpr bool ON = LOW; // FOR NEW SYSTEM IN NANO??
//constexpr bool OFF = HIGH; // FOR NEW SYSTEM IN NANO??
constexpr bool ON = HIGH;  // IN CURRENT SYSTEM
constexpr bool OFF = LOW;  // IN CURRENT SYSTEM
}

constexpr uint8_t BUF_SIZE = 100;

// --- Global variables

// - Time Tracking

uint32_t previousMillis = 0;
uint32_t timeOffStartup = 0;
bool usedStartupTimeOff = false;


// -- Buffers
char buf_serial[BUF_SIZE];


// --- function prototypes
void ControlPump(uint32_t time_on, uint32_t time_off);
uint32_t getTimeOn();
uint32_t getTimeOff(uint16_t potValue = 0);

// -------- Start of void setup()

void setup() {
  pinMode(PIN::RELAY, OUTPUT);
  pinMode(PIN::LED_LEVEL_FULL, OUTPUT);

#if (POT_ENABLED)
  pinMode(PIN::POT, INPUT);
#endif

#if (LEVEL_SENSOR_ENABLED)
  pinMode(PIN::LEVEL_SENSOR, INPUT);  // DON'T FORGET TO PUT A 4.7 - 10 kOhm PULL-UP RESISTOR !!
#endif

  digitalWrite(PIN::RELAY, RELAY::OFF);  // Ensuring relay is off at startup
  digitalWrite(PIN::LED_LEVEL_FULL, LOW);

  LOG_BEGIN(S_COMMS::S_BAUDRATE);
  LOG_LN("\nSystem starting...");

  uint32_t startTime = millis();
  while ((millis() - startTime) < S_TIMERS_DELAYS::STARTUP) {}

#if POT_ENABLED
  uint16_t potValue = analogRead(PIN::POT);
  timeOffStartup = getTimeOff(potValue);  // Now it stores and uses the initial reading
  LOG("Startup pot value: ");
  LOG(potValue);
  LOG(" -> Initial timeOff (min): ");
  LOG_LN(timeOffStartup / CONVERSIONS::MINUTES_TO_MILLISECONDS);
#endif

  /*
#if (POT_ENABLED)

  uint16_t potValue = analogRead(PIN::POT);
  // Adjust time_off based on potentiometer reading at startup
  if (potValue < S_TIMERS_DELAYS::TIME_OFF_OPT_1_SCALE) {
    time_off = S_TIMERS_DELAYS::TIME_OFF_OPT_1 * CONVERSIONS::MINUTES_TO_MILLISECONDS;
  } else if (potValue < S_TIMERS_DELAYS::TIME_OFF_OPT_2_SCALE) {
    time_off = S_TIMERS_DELAYS::TIME_OFF_OPT_2 * CONVERSIONS::MINUTES_TO_MILLISECONDS;
  } else {
    time_off = S_TIMERS_DELAYS::TIME_OFF_OPT_ANTIFREEZE * CONVERSIONS::MINUTES_TO_MILLISECONDS;  // anti-freeze mode
  }
#endif
*/

  // CHECK IF POT IS READ AT THE BEGINNING SINCE YOU'VE COMMENTED OUT IT'S CODE

#if (LEVEL_SENSOR_ENABLED)
  if (!(digitalRead(PIN::LEVEL_SENSOR))) {
    digitalWrite(PIN::RELAY, RELAY::ON);  // added later for startup activation
    previousMillis = millis();            // added later for startup activation
  }
#else
  digitalWrite(PIN::RELAY, RELAY::ON);  // added later for startup activation
  previousMillis = millis();            // added later for startup activation
#endif
}

// -------- End of void setup()

// -------- Start of void loop()

void loop() {

#if (DEBUG_MODE_ENABLED)
  static uint32_t lastDebugTime = 0;
  if ((millis() - lastDebugTime) >= S_TIMERS_DELAYS::DEBUG_PERIOD_IN_S) {
    lastDebugTime = millis();
    snprintf(buf_serial, sizeof(buf_serial),
             "Time: %lu s, Relay: %d, Level Sensor[EN:%d VAL: %d LED:%d], Pot[EN:%d VAL:%d], Time_off: %lu min",
             (((millis() - S_TIMERS_DELAYS::STARTUP) / 1000)),
             !(digitalRead(PIN::RELAY)),
             LEVEL_SENSOR_ENABLED,
             digitalRead(PIN::LEVEL_SENSOR),
             digitalRead(PIN::LED_LEVEL_FULL),
             POT_ENABLED,
             analogRead(PIN::POT),
             (getTimeOff(analogRead(PIN::POT)) / CONVERSIONS::MINUTES_TO_MILLISECONDS));

    LOG_LN(buf_serial);
    Serial.flush();
  }
#endif


  //if (POT_ENABLED && (!FAST_TEST_MODE_ENABLED) && ((millis() - lastPotReadingMillis) >= S_TIMERS_DELAYS::POT_CHECK_INTERVAL))  // setting time_off by pot values
  //{
#if ((POT_ENABLED) && (!FAST_TEST_MODE_ENABLED))
  static uint32_t lastPotReadingMillis = 0;
  if ((millis() - lastPotReadingMillis) >= S_TIMERS_DELAYS::POT_CHECK_INTERVAL) {
    lastPotReadingMillis = millis();
    uint16_t potValue = analogRead(PIN::POT);
    uint32_t time_off = getTimeOff(potValue);

    if (time_off == S_TIMERS_DELAYS::TIME_OFF_OPT_ANTIFREEZE * CONVERSIONS::MINUTES_TO_MILLISECONDS) {
      ControlPump(getTimeOn(), time_off);
      digitalWrite(PIN::LED_LEVEL_FULL, digitalRead(PIN::LEVEL_SENSOR));
      return;
    }
  }
  //}
#endif


#if (LEVEL_SENSOR_ENABLED)
  if (!(digitalRead(PIN::LEVEL_SENSOR)))  // if water tank is not full, proceed normally
  {
    if (POT_ENABLED && !FAST_TEST_MODE_ENABLED) {
      if (!usedStartupTimeOff) {
        ControlPump(getTimeOn(), timeOffStartup);
        usedStartupTimeOff = true;
      } else {
        ControlPump(getTimeOn(), getTimeOff(analogRead(PIN::POT)));
      }
    } else {
      ControlPump(getTimeOn(), getTimeOff());
    }
    digitalWrite(PIN::LED_LEVEL_FULL, LOW);
  } else  // if tank is full, STOP and wait till it's not full and then wait for time_off time
  {
    digitalWrite(PIN::RELAY, RELAY::OFF);
    previousMillis = millis();
    digitalWrite(PIN::LED_LEVEL_FULL, HIGH);
  }
#else
  if (POT_ENABLED && !FAST_TEST_MODE_ENABLED) {
    ControlPump(getTimeOn(), getTimeOff(analogRead(PIN::POT)));
  } else {
    ControlPump(getTimeOn(), getTimeOff());
  }
#endif
}
// -------- End of void loop()

// ------- Start of custom function implementations
void ControlPump(uint32_t time_on, uint32_t time_off) {
  uint64_t currentMillis = millis();

  //Serial.println(time_off);

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

uint32_t getTimeOn() {
#if FAST_TEST_MODE_ENABLED
  return S_TIMERS_DELAYS::FT_TIME_ON_IN_S * CONVERSIONS::SECONDS_TO_MILLISECONDS;
#else
  return S_TIMERS_DELAYS::TIME_ON_IN_S * CONVERSIONS::SECONDS_TO_MILLISECONDS;
#endif
}

uint32_t getTimeOff(uint16_t potValue) {
#if FAST_TEST_MODE_ENABLED
  return S_TIMERS_DELAYS::FT_TIME_OFF_IN_S * CONVERSIONS::SECONDS_TO_MILLISECONDS;
#elif POT_ENABLED
  if (potValue < S_TIMERS_DELAYS::TIME_OFF_OPT_1_SCALE)
    return S_TIMERS_DELAYS::TIME_OFF_OPT_1 * CONVERSIONS::MINUTES_TO_MILLISECONDS;
  else if (potValue < S_TIMERS_DELAYS::TIME_OFF_OPT_2_SCALE)
    return S_TIMERS_DELAYS::TIME_OFF_OPT_2 * CONVERSIONS::MINUTES_TO_MILLISECONDS;
  else
    return S_TIMERS_DELAYS::TIME_OFF_OPT_ANTIFREEZE * CONVERSIONS::MINUTES_TO_MILLISECONDS;
#else
  return S_TIMERS_DELAYS::TIME_OFF_IN_M * CONVERSIONS::MINUTES_TO_MILLISECONDS;
#endif
}

// ------- End of custom function implementations
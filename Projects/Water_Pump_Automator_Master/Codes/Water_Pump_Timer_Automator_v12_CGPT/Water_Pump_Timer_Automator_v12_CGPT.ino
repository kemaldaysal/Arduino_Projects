#include <stdint.h>

// ---------------------- Feature Config
#define LEVEL_SENSOR_ENABLED false
#define POT_ENABLED true
#define DEBUG_MODE_ENABLED true
#define FAST_TEST_MODE_ENABLED false

// ---------------------- Serial Logging
#if DEBUG_MODE_ENABLED
#define LOG(x) Serial.print(x)
#define LOG_LN(x) Serial.println(x)
#define LOG_BEGIN(baudrate) Serial.begin(baudrate)
#else
#define LOG(x)
#define LOG_LN(x)
#define LOG_BEGIN(baudrate)
#endif

// ---------------------- Namespaces
namespace CONVERSIONS {
constexpr uint32_t SEC_TO_MS = 1000;
constexpr uint32_t MIN_TO_MS = 60000;
}

namespace S_TIMERS {
constexpr uint32_t STARTUP_DELAY_MS = 3000;
constexpr uint32_t TIME_ON_S = 33;
constexpr uint32_t TIME_OFF_MIN = 15;
constexpr uint32_t FT_TIME_ON_S = 4;
constexpr uint32_t FT_TIME_OFF_S = 4;
constexpr uint32_t POT_READ_INTERVAL = 30 * CONVERSIONS::SEC_TO_MS;
constexpr uint32_t TIME_OFF_OPT1 = 5;
constexpr uint32_t TIME_OFF_OPT2 = 10;
constexpr uint32_t TIME_OFF_ANTIFREEZE = 30;
}

namespace POT {
constexpr uint16_t MAX = 1023;
constexpr uint8_t RANGES = 3;
constexpr uint16_t SCALE = MAX / RANGES;
}

namespace PIN {
constexpr uint8_t RELAY = 3;
constexpr uint8_t POT = A7;
constexpr uint8_t LEVEL_SENSOR = 2;
constexpr uint8_t LED_LEVEL = LED_BUILTIN;
}

namespace RELAY {
constexpr bool ON = HIGH;
constexpr bool OFF = LOW;
}

// ---------------------- Global State
enum SystemState { STARTUP,
                   PUMP_ON,
                   PUMP_OFF,
                   TANK_FULL_WAIT };
SystemState currentState = STARTUP;

uint32_t lastStateChange = 0;
uint32_t potLastReadTime = 0;
uint32_t currentTimeOff = S_TIMERS::TIME_OFF_MIN * CONVERSIONS::MIN_TO_MS;

char buf[100];

uint32_t getTimeOn();
uint32_t getTimeOff(uint16_t potValue);
void logDebug(unsigned long now, unsigned long currentTimeOff);

// ---------------------- Setup
void setup() {
  pinMode(PIN::RELAY, OUTPUT);
  pinMode(PIN::LED_LEVEL, OUTPUT);
#if (POT_ENABLED && (!FAST_TEST_MODE_ENABLED))
  pinMode(PIN::POT, INPUT);
#endif
#if LEVEL_SENSOR_ENABLED
  pinMode(PIN::LEVEL_SENSOR, INPUT);
#endif

  digitalWrite(PIN::RELAY, RELAY::OFF);
  digitalWrite(PIN::LED_LEVEL, LOW);

  LOG_BEGIN(9600);
  LOG_LN("System booting...");

  delay(S_TIMERS::STARTUP_DELAY_MS);
  lastStateChange = millis();

  currentTimeOff = getTimeOff(
#if (POT_ENABLED && (!FAST_TEST_MODE_ENABLED))
    analogRead(PIN::POT)
#else
    0
#endif
  );

  currentState = PUMP_ON;
  digitalWrite(PIN::RELAY, RELAY::ON);
  LOG_LN("Pump ON after startup.");
}

// ---------------------- Loop
void loop() {
  uint32_t now = millis();

#if LEVEL_SENSOR_ENABLED
  bool tankFull = digitalRead(PIN::LEVEL_SENSOR);
  digitalWrite(PIN::LED_LEVEL, tankFull ? HIGH : LOW);
#endif

#if (POT_ENABLED && (!FAST_TEST_MODE_ENABLED))
  //LOG_LN(currentTimeOff); // THIS IS TRIGGERED, results always 37856
  if ((now - potLastReadTime) >= S_TIMERS::POT_READ_INTERVAL) {  // FIX HERE
    potLastReadTime = now;
    currentTimeOff = getTimeOff(analogRead(PIN::POT));
  }
#endif

  logDebug(now, currentTimeOff);

  // LEVEL FULL check (interrupt pump operation immediately)
#if LEVEL_SENSOR_ENABLED
  if (tankFull && currentState != TANK_FULL_WAIT) {
    digitalWrite(PIN::RELAY, RELAY::OFF);
    digitalWrite(PIN::LED_LEVEL, HIGH);
    currentState = TANK_FULL_WAIT;
    LOG_LN("Tank full! Pump stopped.");
    return;
  }
#endif

  // If previously full, wait until it's not full
#if LEVEL_SENSOR_ENABLED
  if (currentState == TANK_FULL_WAIT) {
    if (!tankFull) {
      lastStateChange = now;
      currentState = PUMP_OFF;
      LOG_LN("Tank not full anymore. Entering T_off delay.");
    }
    return;
  }
#endif

  switch (currentState) {
    case PUMP_ON:
      if (now - lastStateChange >= getTimeOn()) {
        digitalWrite(PIN::RELAY, RELAY::OFF);
        lastStateChange = now;
        currentState = PUMP_OFF;
        LOG_LN("Pump OFF. Entering T_off delay.");
      }
      break;

    case PUMP_OFF:
      if (now - lastStateChange >= currentTimeOff) {
        digitalWrite(PIN::RELAY, RELAY::ON);
        lastStateChange = now;
        currentState = PUMP_ON;
        LOG_LN("Pump ON. Entering T_on duration.");
      }
      break;

    default:
      break;
  }
}

// ---------------------- Time Functions
uint32_t getTimeOn() {
#if FAST_TEST_MODE_ENABLED
  return S_TIMERS::FT_TIME_ON_S * CONVERSIONS::SEC_TO_MS;
#else
  return S_TIMERS::TIME_ON_S * CONVERSIONS::SEC_TO_MS;
#endif
}
uint32_t getTimeOff(uint16_t potValue) {
#if (FAST_TEST_MODE_ENABLED)
  return S_TIMERS::FT_TIME_OFF_S * CONVERSIONS::SEC_TO_MS;
#elif (POT_ENABLED && (!FAST_TEST_MODE_ENABLED))
  if (potValue < POT::SCALE)
    return S_TIMERS::TIME_OFF_OPT1 * CONVERSIONS::MIN_TO_MS;
  else if (potValue < 2 * POT::SCALE)
    return S_TIMERS::TIME_OFF_OPT2 * CONVERSIONS::MIN_TO_MS;
  else
    return S_TIMERS::TIME_OFF_ANTIFREEZE * CONVERSIONS::MIN_TO_MS;
#else
  return S_TIMERS::TIME_OFF_MIN * CONVERSIONS::MIN_TO_MS;
#endif
}

// ---------------------- Debug Logging
void logDebug(unsigned long now, unsigned long currentTimeOff) {
#if DEBUG_MODE_ENABLED
  static uint32_t lastLog = 0;
  if (now - lastLog >= 1000) {

#if LEVEL_SENSOR_ENABLED
    int level = digitalRead(PIN::LEVEL_SENSOR);
#else
    int level = -1;
#endif
    lastLog = now;
    snprintf(buf, sizeof(buf),
             "[%lus] State:%d | Relay:%d | Level:%d | Pot:%d | T_off:%lu min or sec",
             (now / 1000) - ((S_TIMERS::STARTUP_DELAY_MS) / 1000),
             currentState,
             digitalRead(PIN::RELAY),
             level,
             POT_ENABLED ? analogRead(PIN::POT) : -1,
#if FAST_TEST_MODE_ENABLED
             currentTimeOff / 1000);
#else
             currentTimeOff / 60000);
#endif

    LOG_LN(buf);
  }
#endif
}
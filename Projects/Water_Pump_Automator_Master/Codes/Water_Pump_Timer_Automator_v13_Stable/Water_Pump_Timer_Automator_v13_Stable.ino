#include <stdint.h>

// ---------------------- Feature Config
#define LEVEL_SENSOR_ENABLED false
#define POT_ENABLED false

#define DEBUG_MODE_ENABLED false
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
constexpr uint32_t STARTUP_DELAY_MS = 19 * CONVERSIONS::MIN_TO_MS; // changed from 3 s to 20 min
constexpr uint32_t TIME_ON_S = 33;
constexpr uint32_t TIME_OFF_MIN = 19; // 15 -> 20
constexpr uint32_t FT_TIME_ON_S = 4;
constexpr uint32_t FT_TIME_OFF_S = 4;
constexpr uint32_t POT_READ_INTERVAL = 30 * CONVERSIONS::SEC_TO_MS;
constexpr uint32_t TIME_OFF_OPT1_MIN = 5;
constexpr uint32_t TIME_OFF_OPT2_MIN = 10;
constexpr uint32_t TIME_OFF_ANTIFREEZE_MIN = 30;
constexpr uint32_t INTERVAL_DEBUG_LOG = 1000;
}

namespace POT {
constexpr uint16_t MAX = 1023;
constexpr uint8_t RANGES = 3;
constexpr uint16_t SCALE = MAX / RANGES;
}

namespace PIN {
//constexpr uint8_t RELAY = 3;  // In Nano (latest system)
constexpr uint8_t RELAY = 6; // In Uno (current system)
constexpr uint8_t POT = A7;
constexpr uint8_t LEVEL_SENSOR = 2;
constexpr uint8_t LED_LEVEL = LED_BUILTIN;
}

namespace RELAY {
constexpr bool ON = HIGH;
constexpr bool OFF = LOW;
}

// ---------------------- Global State
enum class SystemState : uint8_t { STARTUP,
                                   PUMP_ON,
                                   PUMP_OFF,
                                   TANK_FULL_WAIT };
SystemState currentState = SystemState::STARTUP;

uint32_t lastStateChange = 0;
uint32_t potLastReadTime = 0;

#if (!POT_ENABLED || FAST_TEST_MODE_ENABLED)
uint32_t currentTimeOff = S_TIMERS::TIME_OFF_MIN * CONVERSIONS::MIN_TO_MS;
#else
uint32_t currentTimeOff = 0;  // Will be set from analogRead()
#endif

constexpr uint8_t SIZE_BUF = 100;

char buf[SIZE_BUF];

static inline uint32_t getTimeOn();
static inline uint32_t getTimeOff(uint16_t potValue);
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

  currentState = SystemState::PUMP_ON;
  digitalWrite(PIN::RELAY, RELAY::ON);
  LOG_LN("Pump ON after startup.");
}

// ---------------------- Loop
void loop() {
  uint32_t now = millis();

#if (POT_ENABLED && (!FAST_TEST_MODE_ENABLED))
  if ((now - potLastReadTime) >= S_TIMERS::POT_READ_INTERVAL) {
    potLastReadTime = now;
    currentTimeOff = getTimeOff(analogRead(PIN::POT));
  }
#endif

  logDebug(now, currentTimeOff);

  // LEVEL FULL check (interrupt pump operation immediately)
#if LEVEL_SENSOR_ENABLED
  bool tankFull = digitalRead(PIN::LEVEL_SENSOR);
  digitalWrite(PIN::LED_LEVEL, tankFull ? HIGH : LOW);

  if (tankFull && (currentState != SystemState::TANK_FULL_WAIT)) {
    digitalWrite(PIN::RELAY, RELAY::OFF);
    digitalWrite(PIN::LED_LEVEL, HIGH);
    currentState = SystemState::TANK_FULL_WAIT;
    LOG_LN("Tank full! Pump stopped.");
    return;
  }

  // If previously full, wait until it's not full
  if ((currentState == SystemState::TANK_FULL_WAIT) && !tankFull) {
    lastStateChange = now;
    currentState = SystemState::PUMP_OFF;
    LOG_LN("Tank not full anymore. Entering T_off delay.");
    return;
  }
#endif

  switch (currentState) {
    case SystemState::PUMP_ON:
      if (now - lastStateChange >= getTimeOn()) {
        digitalWrite(PIN::RELAY, RELAY::OFF);
        lastStateChange = now;
        currentState = SystemState::PUMP_OFF;
        LOG_LN("Pump OFF. Entering T_off delay.");
      }
      break;

    case SystemState::PUMP_OFF:
      if (now - lastStateChange >= currentTimeOff) {
        digitalWrite(PIN::RELAY, RELAY::ON);
        lastStateChange = now;
        currentState = SystemState::PUMP_ON;
        LOG_LN("Pump ON. Entering T_on duration.");
      }
      break;

    default:
      break;
  }
}

// ---------------------- Time Functions
static inline uint32_t getTimeOn() {
#if FAST_TEST_MODE_ENABLED
  return S_TIMERS::FT_TIME_ON_S * CONVERSIONS::SEC_TO_MS;
#else
  return S_TIMERS::TIME_ON_S * CONVERSIONS::SEC_TO_MS;
#endif
}
static inline uint32_t getTimeOff(uint16_t potValue) {
#if (FAST_TEST_MODE_ENABLED)
  return S_TIMERS::FT_TIME_OFF_S * CONVERSIONS::SEC_TO_MS;
#elif (POT_ENABLED && (!FAST_TEST_MODE_ENABLED))
  if (potValue < POT::SCALE)
    return S_TIMERS::TIME_OFF_OPT1_MIN * CONVERSIONS::MIN_TO_MS;
  else if (potValue < 2 * POT::SCALE)
    return S_TIMERS::TIME_OFF_OPT2_MIN * CONVERSIONS::MIN_TO_MS;
  else
    return S_TIMERS::TIME_OFF_ANTIFREEZE_MIN * CONVERSIONS::MIN_TO_MS;
#else
  return S_TIMERS::TIME_OFF_MIN * CONVERSIONS::MIN_TO_MS;
#endif
}

// ---------------------- Debug Logging
void logDebug(unsigned long now, unsigned long currentTimeOff) {
#if DEBUG_MODE_ENABLED
  static uint32_t lastLog = 0;
  if (now - lastLog >= S_TIMERS::INTERVAL_DEBUG_LOG) {
    lastLog = now;

#if LEVEL_SENSOR_ENABLED
    int level = digitalRead(PIN::LEVEL_SENSOR);
#else
    int level = -1;
#endif

#if FAST_TEST_MODE_ENABLED
uint32_t toffDisplay = currentTimeOff / CONVERSIONS::SEC_TO_MS;
const char* unit = "sec";
#else
uint32_t toffDisplay = currentTimeOff / CONVERSIONS::MIN_TO_MS;
const char* unit = "min";
#endif

    snprintf(buf, sizeof(buf),
             "[%lus] State:%d | Relay:%d | Level:%d | Pot:%d | T_off:%lu %s",
             (now - S_TIMERS::STARTUP_DELAY_MS) / CONVERSIONS::SEC_TO_MS,
             static_cast<int>(currentState),
             digitalRead(PIN::RELAY),
             level,
             POT_ENABLED ? analogRead(PIN::POT) : -1,
             toffDisplay,
             unit);

    LOG_LN(buf);
  }
#endif
}
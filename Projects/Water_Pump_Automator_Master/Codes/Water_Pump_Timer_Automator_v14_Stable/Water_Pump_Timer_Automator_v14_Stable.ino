#include <stdint.h>

// ---------------------- Feature Config
#define LEVEL_SENSOR_ENABLED false
#define POT_ENABLED false

#define DEBUG_MODE_ENABLED false
#define FAST_TEST_MODE_ENABLED false
#define WATCHDOG_TIMER_ENABLED false

#if WATCHDOG_TIMER_ENABLED
#include <avr/wdt.h>
#endif

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
constexpr uint32_t STARTUP_DELAY_MS = 16 * CONVERSIONS::MIN_TO_MS;  // 19 -> 16
//constexpr uint32_t STARTUP_DELAY_MS = 5 * CONVERSIONS::SEC_TO_MS;  // for fast trials
constexpr uint32_t TIME_ON_S = 30;
constexpr uint32_t TIME_OFF_MIN = 16;  // 15 -> 19 -> 16

#if FAST_TEST_MODE_ENABLED
constexpr uint32_t STARTUP_DELAY_FAST_TEST = 3 * CONVERSIONS::SEC_TO_MS;
constexpr uint32_t FT_TIME_ON_S = 4;
constexpr uint32_t FT_TIME_OFF_S = 4;
#endif

#if POT_ENABLED
constexpr uint32_t POT_READ_INTERVAL = 30 * CONVERSIONS::SEC_TO_MS;
constexpr uint32_t TIME_OFF_OPT1_MIN = 5;
constexpr uint32_t TIME_OFF_OPT2_MIN = 10;
constexpr uint32_t TIME_OFF_ANTIFREEZE_MIN = 30;
#endif

#if DEBUG_MODE_ENABLED
constexpr uint32_t INTERVAL_DEBUG_LOG = 1000;
#endif

#if WATCHDOG_TIMER_ENABLED
constexpr uint32_t WATCHDOG_STARTUP_DELAY = 3 * CONVERSIONS::SEC_TO_MS;  // 3 -> 2 -> 3
#endif
}

#if POT_ENABLED
namespace POT {
constexpr uint16_t MAX = 1023;
constexpr uint8_t RANGES = 3;
constexpr uint16_t SCALE = MAX / RANGES;
}
#endif

namespace PIN {
constexpr uint8_t RELAY = 3;  // In Nano (latest system)
// constexpr uint8_t RELAY = 6;  // In Uno (current system)
#if POT_ENABLED
constexpr uint8_t POT = A7;
#endif
#if LEVEL_SENSOR_ENABLED
constexpr uint8_t LEVEL_SENSOR = 2;
constexpr uint8_t LED_LEVEL = LED_BUILTIN;
#endif
}

namespace RELAY {
constexpr bool ON = LOW;    // For Nano with new relay
constexpr bool OFF = HIGH;  // For Nano with new relay
// constexpr bool ON = HIGH; // For Uno with older relay
// constexpr bool OFF = LOW; // For Uno with older relay
}

// ---------------------- State class
enum class SystemState : uint8_t { STARTUP,
                                   PUMP_ON,
                                   PUMP_OFF,
                                   TANK_FULL_WAIT };
SystemState currentState = SystemState::STARTUP;

// ---------------------- Global Variables

#if (POT_ENABLED)
uint32_t potLastReadTime = 0;
#endif

uint32_t lastStateChange = 0;
uint32_t startupBeginTime = 0;
bool startupDelayDone = false;

#if ((!POT_ENABLED) && (!FAST_TEST_MODE_ENABLED))
uint32_t timeOff = S_TIMERS::TIME_OFF_MIN * CONVERSIONS::MIN_TO_MS;
#elif ((!POT_ENABLED) && (FAST_TEST_MODE_ENABLED))
uint32_t timeOff = S_TIMERS::FT_TIME_OFF_S * CONVERSIONS::SEC_TO_MS;
#elif ((POT_ENABLED) && (!FAST_TEST_MODE_ENABLED))
uint32_t timeOff = 0;  // will be set from analogRead later
#elif ((POT_ENABLED) && (FAST_TEST_MODE_ENABLED))
uint32_t timeOff = 0;  // will be set from analogRead later

#endif

#if FAST_TEST_MODE_ENABLED
uint32_t timeOn = S_TIMERS::FT_TIME_ON_S * CONVERSIONS::SEC_TO_MS;
#else
uint32_t timeOn = S_TIMERS::TIME_ON_S * CONVERSIONS::SEC_TO_MS;
#endif

#if DEBUG_MODE_ENABLED
constexpr uint8_t SIZE_BUF = 100;
char buf[SIZE_BUF];
#endif

// ---------------- Function Prototypes

#if POT_ENABLED
uint32_t getTimeOffPot(uint16_t potValue = 0);
#endif

#if DEBUG_MODE_ENABLED
void logDebug(unsigned long now, unsigned long timeOff);
#endif

// ---------------------- Setup Function
void setup() {

#if (WATCHDOG_TIMER_ENABLED)
  wdt_disable();
#endif

  pinMode(LED_BUILTIN, OUTPUT);

  for (int i = 0; i < 6; i++) {  // total 3 seconds of delay for wdt
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
  }

#if (WATCHDOG_TIMER_ENABLED)
  wdt_enable(WDTO_8S);  // Watchdog: reset if loop stalls for 8 seconds
#endif

  pinMode(PIN::RELAY, OUTPUT);
  digitalWrite(PIN::RELAY, RELAY::OFF);

#if (POT_ENABLED && (!FAST_TEST_MODE_ENABLED))
  pinMode(PIN::POT, INPUT);
  timeOff = getTimeOffPot(analogRead(PIN::POT));
#endif

#if LEVEL_SENSOR_ENABLED
  pinMode(PIN::LEVEL_SENSOR, INPUT);
  pinMode(PIN::LED_LEVEL, OUTPUT);
  digitalWrite(PIN::LED_LEVEL, LOW);
#endif

  currentState = SystemState::STARTUP;

  LOG_BEGIN(9600);
  LOG_LN("System booted. Non-blocking startup delay initialized.");

  startupBeginTime = millis();
}

// ---------------------- Loop
void loop() {
#if (WATCHDOG_TIMER_ENABLED)
  wdt_reset();  // reset watchdog
#endif

  uint32_t now = millis();

  if (!startupDelayDone) {
#if FAST_TEST_MODE_ENABLED
    if (now - startupBeginTime >= S_TIMERS::STARTUP_DELAY_FAST_TEST) {
#else
    if (now - startupBeginTime >= S_TIMERS::STARTUP_DELAY_MS) {
#endif
      startupDelayDone = true;
      LOG_LN("Startup delay finished.");

#if LEVEL_SENSOR_ENABLED
      bool tankFull_at_startup = digitalRead(PIN::LEVEL_SENSOR);
      digitalWrite(PIN::LED_LEVEL, tankFull_at_startup);
      if (tankFull_at_startup) {
        digitalWrite(PIN::RELAY, RELAY::OFF);
        currentState = SystemState::TANK_FULL_WAIT;
        lastStateChange = now;
        LOG_LN("Tank full after startup delay, i will watch the sensor.");
      }
#else
      digitalWrite(PIN::RELAY, RELAY::ON);
      currentState = SystemState::PUMP_ON;
      lastStateChange = now;
      LOG_LN("PUMP ON after startup delay");
#endif
    }
    return;  // Still waiting for startup delay to finish
  }

#if (POT_ENABLED && (!FAST_TEST_MODE_ENABLED))
  if ((now - potLastReadTime) >= S_TIMERS::POT_READ_INTERVAL) {
    potLastReadTime = now;
    timeOff = getTimeOffPot(analogRead(PIN::POT));
  }
#endif

#if DEBUG_MODE_ENABLED
  logDebug(now, timeOff);
#endif

  // LEVEL FULL check (interrupt pump operation immediately)
#if LEVEL_SENSOR_ENABLED
  bool tankFull = digitalRead(PIN::LEVEL_SENSOR);
  digitalWrite(PIN::LED_LEVEL, tankFull ? HIGH : LOW);

  if (tankFull && (currentState != SystemState::TANK_FULL_WAIT)) {
    digitalWrite(PIN::RELAY, RELAY::OFF);
    currentState = SystemState::TANK_FULL_WAIT;
    lastStateChange = now;
    LOG_LN("Tank full! Pump stopped.");
    return;
  }

  else if ((currentState == SystemState::TANK_FULL_WAIT) && !tankFull) {
    currentState = SystemState::PUMP_OFF;
    lastStateChange = now;
    LOG_LN("Tank not full anymore. Entering T_off delay.");
    return;
  }
#endif

  switch (currentState) {
    case SystemState::PUMP_ON:
      if (now - lastStateChange >= timeOn) {
        digitalWrite(PIN::RELAY, RELAY::OFF);
        currentState = SystemState::PUMP_OFF;
        lastStateChange = now;
        LOG_LN("Pump OFF. Entering T_off delay.");
      }
      break;

    case SystemState::PUMP_OFF:
      if (now - lastStateChange >= timeOff) {
        digitalWrite(PIN::RELAY, RELAY::ON);
        currentState = SystemState::PUMP_ON;
        lastStateChange = now;
        LOG_LN("Pump ON. Entering T_on duration.");
      }
      break;

    default:
      digitalWrite(PIN::RELAY, RELAY::OFF);
      currentState = SystemState::PUMP_OFF;
      lastStateChange = now;
      LOG_LN("Unknown state!! PUMP OFF.");
      break;
  }
}

// ---------------------- Function Implementations

// ------------- Potentiometer based Off-Time

#if (POT_ENABLED)
uint32_t getTimeOffPot(uint16_t potValue) {
  if (potValue < POT::SCALE) {
    return S_TIMERS::TIME_OFF_OPT1_MIN * CONVERSIONS::MIN_TO_MS;
  } else if (potValue < 2 * POT::SCALE) {
    return S_TIMERS::TIME_OFF_OPT2_MIN * CONVERSIONS::MIN_TO_MS;
  } else {
    return S_TIMERS::TIME_OFF_ANTIFREEZE_MIN * CONVERSIONS::MIN_TO_MS;
  }
}
#endif

// ------------- Debug Logging
#if DEBUG_MODE_ENABLED
void logDebug(unsigned long now, unsigned long timeOff) {
  static uint32_t lastLog = 0;
  if (now - lastLog >= S_TIMERS::INTERVAL_DEBUG_LOG) {
    lastLog = now;

#if FAST_TEST_MODE_ENABLED
    uint32_t toffDisplay = timeOff / CONVERSIONS::SEC_TO_MS;
    const char* unit = "sec";
    uint32_t current_time_final_to_display = (now - S_TIMERS::STARTUP_DELAY_FAST_TEST) / CONVERSIONS::SEC_TO_MS;
#else
    uint32_t toffDisplay = timeOff / CONVERSIONS::MIN_TO_MS;
    const char* unit = "min";
    uint32_t current_time_final_to_display = (now - S_TIMERS::STARTUP_DELAY_MS) / CONVERSIONS::SEC_TO_MS;
#endif

#if LEVEL_SENSOR_ENABLED
    int level_log = digitalRead(PIN::LEVEL_SENSOR);
#else
    int level_log = -1;
#endif

#if POT_ENABLED
    int pot_log = analogRead(PIN::POT);
#else
    int pot_log = -1;
#endif

    snprintf(buf, sizeof(buf),
             "[%lus] State:%d | Relay:%d | Level:%d | Pot:%d | T_off:%lu %s",
             current_time_final_to_display,
             static_cast<int>(currentState),
             !(digitalRead(PIN::RELAY)),
             level_log,
             pot_log,
             toffDisplay,
             unit);

    LOG_LN(buf);
  }
}
#endif
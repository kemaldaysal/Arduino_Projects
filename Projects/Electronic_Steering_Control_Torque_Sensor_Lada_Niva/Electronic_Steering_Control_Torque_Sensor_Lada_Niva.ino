/*
  TO DO (IMPORTANT):
  
  1) Implement a filter that'll track transitions from Stationary -> to directions, then only apply assist if that conditions met.
    If direction -> stationary happens (self-alignment of steer), then it won't provide assist. 
     !! Otherwise, it'll create critical driving safety problems after turns !!  
  
  2) Get vehicle speed info from another sensor to only allow e-steering assist when below a speed threshold, for city manuevers 

*/

#include <stdint.h>

// ---------------------- Defines, Enumerations and Constants

constexpr bool LOGGING_ENABLED = true;

#define LOG(x) \
  if (LOGGING_ENABLED) { Serial.print(x); }
#define LOG_LN(x) \
  if (LOGGING_ENABLED) { Serial.println(x); }

namespace PIN {
constexpr uint8_t SENSOR_TORQUE = A0;
constexpr uint8_t RELAY_LEFT = 5;
constexpr uint8_t RELAY_RIGHT = 6;
}

namespace RELAY_STATE {
constexpr bool ON = 0;
constexpr bool OFF = 1;
}

enum class TORQUE_DIRECTION : uint8_t {
  LEFT,
  RIGHT,
  STATIONARY
};

constexpr float V_THR_LEFT = 2.3f;
constexpr float V_THR_RIGHT = 3.5f;
constexpr float DEADBAND = 0.05f;  // use it if necessary

constexpr uint16_t SERIAL_BAUDRATE = 9600;
constexpr uint8_t ADC_VOLTAGE_MAX = 5;
constexpr uint8_t ADC_VOLTAGE_MIN = 0;
constexpr float ADC_RESOLUTION = 1023.0f;

// ---------------------- Function Prototypes

TORQUE_DIRECTION Decide_Direction(float sensor_voltage_output);
void Assist_Left();
void Assist_Right();
void No_Assist();

// ---------------------- setup() function

void setup() {
  Serial.begin(SERIAL_BAUDRATE);
  pinMode(PIN::RELAY_LEFT, OUTPUT);
  pinMode(PIN::RELAY_RIGHT, OUTPUT);
}

// ---------------------- loop() function

void loop() {

  float sensor_voltage_output = (analogRead(PIN::SENSOR_TORQUE) * ADC_VOLTAGE_MAX) / ADC_RESOLUTION;
  Serial.println(sensor_voltage_output);

  switch (Decide_Direction(sensor_voltage_output)) {
    case TORQUE_DIRECTION::LEFT: Assist_Left(); break;
    case TORQUE_DIRECTION::RIGHT: Assist_Right(); break;
    case TORQUE_DIRECTION::STATIONARY: No_Assist(); break;
    default:
      LOG_LN("ERROR: Faulty sensor output");
      break;
  }
}

// ---------------------- Function Implementations

TORQUE_DIRECTION Decide_Direction(float sensor_voltage_output) {

  if ((sensor_voltage_output < ADC_VOLTAGE_MIN) || (sensor_voltage_output > ADC_VOLTAGE_MAX)) {
    LOG("Invalid sensor voltage: ");
    LOG_LN(sensor_voltage_output);
    return TORQUE_DIRECTION::STATIONARY;
  }

  if (sensor_voltage_output < V_THR_LEFT)  // Steering to left detected
  {
    return TORQUE_DIRECTION::LEFT;
  }

  else if (V_THR_RIGHT < sensor_voltage_output)  // Steering to right detected
  {
    return TORQUE_DIRECTION::RIGHT;
  }

  return TORQUE_DIRECTION::STATIONARY;  // No assist when not turning the steering wheel
}

void Assist_Left() {
  digitalWrite(PIN::RELAY_RIGHT, RELAY_STATE::ON);
  digitalWrite(PIN::RELAY_LEFT, RELAY_STATE::OFF);
}

void Assist_Right() {
  digitalWrite(PIN::RELAY_RIGHT, RELAY_STATE::OFF);
  digitalWrite(PIN::RELAY_LEFT, RELAY_STATE::ON);
}

void No_Assist() {
  digitalWrite(PIN::RELAY_RIGHT, RELAY_STATE::OFF);
  digitalWrite(PIN::RELAY_LEFT, RELAY_STATE::OFF);
}

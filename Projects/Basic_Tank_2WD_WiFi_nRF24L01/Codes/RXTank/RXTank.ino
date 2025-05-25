// RX Codes on Tank

// ------ Includes and Libraries

#include <stdint.h>

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// ------ Toggles of settings and features
#define DEBUG_MODE_ENABLED true

// ------ Macros

#if DEBUG_MODE_ENABLED
#define LOG(x) Serial.print(x)
#define LOG_LN(x) Serial.println(x)
#define LOG_BEGIN(baudrate) Serial.begin(baudrate)
#else
#define LOG(x)
#define LOG_LN(x)
#define LOG_BEGIN(baudrate)
#endif

// ------ Namespaces, Defines, Enumerations

namespace PINS {
constexpr uint8_t RF_CE = 9;
constexpr uint8_t RF_CSN = 10;

constexpr uint8_t MOTOR_R_FORWARD = 5;   // IN2
constexpr uint8_t MOTOR_R_BACKWARD = 4;  // IN1
constexpr uint8_t MOTOR_R_PWM = 3;       // ENA

constexpr uint8_t MOTOR_L_FORWARD = 7;   // IN3
constexpr uint8_t MOTOR_L_BACKWARD = 8;  // IN4
constexpr uint8_t MOTOR_L_PWM = 6;       // ENB

constexpr uint8_t LED = 2;
}

namespace COMMS {

constexpr uint16_t BAUDRATE_SERIAL = 9600;
constexpr uint64_t CHANNEL_ADDRESS = 0xE8E8F0F0E1LL;  // address must be same between RX/TX

}

namespace CONTROL {
constexpr uint8_t JOY_CENTER = 512;
constexpr uint8_t JOY_DEADZONE = 20;
constexpr uint8_t PWM_MAX = 255;
}

constexpr uint16_t RF_DELAY = 20;

// ------ Global Variables

struct ControlData {
  uint16_t x;  // 0 - 1023
  uint16_t y;  // 0 - 1023
  bool led_state;
};

ControlData received_data = { CONTROL::JOY_CENTER, CONTROL::JOY_CENTER, false };

// ------ State Machine

enum class RobotState {
  WAITING_FOR_INPUT_DATA,
  APPLY_INPUT_DATA
};

RobotState current_state = RobotState::WAITING_FOR_INPUT_DATA;

RF24 radio(PINS::RF_CE, PINS::RF_CSN);

// ------ Function prototypes

void Setup_Radio();
void Handle_State_Machine();
void Apply_Movement(int x_raw, int y_raw);
int Map_Joystick(int raw_val, bool invert = false);
int Apply_Deadzone(int value);
void Data_Logger();
void Setup_Pins();

// ------ setup() Function

void setup() {
  LOG_BEGIN(COMMS::BAUDRATE_SERIAL);
  Setup_Pins();
  Setup_Radio();
  LOG_LN("System initialized");
}

// ------ loop() Function

void loop() {
  Handle_State_Machine();
  delay(RF_DELAY);
}

// ------ Function Implementations

void Setup_Radio() {
  radio.begin();
  radio.openReadingPipe(1, COMMS::CHANNEL_ADDRESS);  // // Start as receiver/reader in specified channel address
  //radio.setPALevel(RF24_PA_MIN);   // Optional range setting(?). 4 choices: MIN, LOW, HIGH, MAX. MAX means high range, Min yaparsan menzil en az olur.
  //radio.setDataRate(RF24_250KBPS); // Optional transmission speed setting(?). 3 choices: 250KBPS, 1MBPS, 2 MBPS. Slow speed = high range
  radio.startListening();  // Start listening
  LOG_LN("nRF24 is started listening in RX mode.");
}

void Handle_State_Machine() {

  switch (current_state) {
    case (RobotState::WAITING_FOR_INPUT_DATA):

      if (radio.available()) {
        radio.read(&received_data, sizeof(received_data));
        current_state = RobotState::APPLY_INPUT_DATA;
      }
      break;

    case (RobotState::APPLY_INPUT_DATA):
      Apply_Movement(received_data.x, received_data.y);
      digitalWrite(PINS::LED, received_data.led_state ? HIGH : LOW);
      current_state = RobotState::WAITING_FOR_INPUT_DATA;
      break;

    default: break;
  }
}

void Apply_Movement(int x_raw, int y_raw) {

  int x = Apply_Deadzone(Map_Joystick(x_raw));
  int y = Apply_Deadzone(Map_Joystick(y_raw, true));

  int motor_l_speed = y + x;
  int motor_r_speed = y - x;

  motor_l_speed = constrain(motor_l_speed, -255, 255);
  motor_r_speed = constrain(motor_r_speed, -255, 255);

  LOG("joyX: ");
  LOG(x);
  LOG(" joyY: ");
  LOG(y);
  LOG(" m_L_speed: ");
  LOG(motor_l_speed);
  LOG(" m_R_speed: ");
  LOG(motor_r_speed);

  // Left Motor
  if (motor_l_speed > 0) {
    digitalWrite(PINS::MOTOR_L_FORWARD, HIGH);
    digitalWrite(PINS::MOTOR_L_BACKWARD, LOW);
    analogWrite(PINS::MOTOR_L_PWM, motor_l_speed);
  } else if (motor_l_speed < 0) {
    digitalWrite(PINS::MOTOR_L_FORWARD, LOW);
    digitalWrite(PINS::MOTOR_L_BACKWARD, HIGH);
    analogWrite(PINS::MOTOR_L_PWM, -motor_l_speed);
  } else {
    digitalWrite(PINS::MOTOR_L_FORWARD, LOW);
    digitalWrite(PINS::MOTOR_L_BACKWARD, LOW);
    analogWrite(PINS::MOTOR_L_PWM, 0);
  }

  // Right Motor
  if (motor_r_speed > 0) {
    digitalWrite(PINS::MOTOR_R_FORWARD, HIGH);
    digitalWrite(PINS::MOTOR_R_BACKWARD, LOW);
    analogWrite(PINS::MOTOR_R_PWM, motor_r_speed);
  } else if (motor_l_speed < 0) {
    digitalWrite(PINS::MOTOR_R_FORWARD, LOW);
    digitalWrite(PINS::MOTOR_R_BACKWARD, HIGH);
    analogWrite(PINS::MOTOR_R_PWM, -motor_r_speed);
  } else {
    digitalWrite(PINS::MOTOR_R_FORWARD, LOW);
    digitalWrite(PINS::MOTOR_R_BACKWARD, LOW);
    analogWrite(PINS::MOTOR_R_PWM, 0);
  }
}

int Map_Joystick(int raw_val, bool invert) {
  int val = raw_val - CONTROL::JOY_CENTER;
  return invert ? -val : val;
}

int Apply_Deadzone(int value) {
  return (abs(value) < CONTROL::JOY_DEADZONE) ? 0 : value;
}

void Data_Logger()  // Vericiden gelen joystick değerlerini serial monitöre yazdırarak değerlerin iletimini ve gecikmesini kontrol edebilmeyi sağlayan fonksiyonu oluşturduk.
{
  LOG("x (j1): ");
  LOG(received_data.x);
  LOG(", y (j2): ");
  LOG(received_data.y);
  LOG(", ledState: ");
  LOG_LN(received_data.led_state);
}

void Setup_Pins() {
  pinMode(PINS::MOTOR_R_FORWARD, OUTPUT);
  pinMode(PINS::MOTOR_R_BACKWARD, OUTPUT);
  pinMode(PINS::MOTOR_R_PWM, OUTPUT);

  pinMode(PINS::MOTOR_L_FORWARD, OUTPUT);
  pinMode(PINS::MOTOR_L_BACKWARD, OUTPUT);
  pinMode(PINS::MOTOR_L_PWM, OUTPUT);

  pinMode(PINS::LED, OUTPUT);
}

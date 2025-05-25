// ----------------- Libraries ------------------

#include <stdint.h>
#include <SoftwareSerial.h>

// ----------------- Global Variables ------------------

SoftwareSerial bt_iot(10, 11);

// ----------------- Settings with Namespaces, Enums and Defines ------------------

constexpr bool LOGGING_ENABLED = true;

#define LOG(x) \
  if (LOGGING_ENABLED) { Serial.print(x); }
#define LOG_LN(x) \
  if (LOGGING_ENABLED) { Serial.println(x); }

namespace PIN {
// RIGHT MOTOR PINS
constexpr uint8_t MOTOR_R_FORWARD = 5;
constexpr uint8_t MOTOR_R_BACKWARD = 4;
constexpr uint8_t MOTOR_R_PWM = 3;

// LEFT MOTOR PINS
constexpr uint8_t MOTOR_L_FORWARD = 7;
constexpr uint8_t MOTOR_L_BACKWARD = 8;
constexpr uint8_t MOTOR_L_PWM = 6;
}

// Baudrate speed settings
namespace BAUDRATE {
constexpr uint32_t SERIAL_MONITOR = 9600;
constexpr uint32_t BLUETOOTH = 9600;
}

// Motor speed setting stages (PWM between 0-255)
namespace M_SPEED {
constexpr int16_t INVALID_SPEED = -1;
constexpr int16_t STAGE_0 = 0;
constexpr int16_t STAGE_1 = 26;
constexpr int16_t STAGE_2 = 52;
constexpr int16_t STAGE_3 = 78;
constexpr int16_t STAGE_4 = 104;
constexpr int16_t STAGE_5 = 130;
constexpr int16_t STAGE_6 = 156;
constexpr int16_t STAGE_7 = 182;
constexpr int16_t STAGE_8 = 208;
constexpr int16_t STAGE_9 = 234;
constexpr int16_t STAGE_10 = 255;
}

enum class DIRECTION { STOP,
                       FORWARD,
                       BACKWARD };
enum class Turn { NONE,
                  LEFT,
                  RIGHT };

// ----------------- Globals ------------------
int16_t current_speed = M_SPEED::STAGE_5;  // Default mid-speed
int16_t prev_l_speed = 0, prev_r_speed = 0;
unsigned long last_command_time = 0;
constexpr unsigned long COMMAND_TIMEOUT_MS = 5000;

// ----------------- Function Prototypes ------------------

void Set_Pin_Modes();
int16_t Get_Speed_Stage(char bt_data);
void Apply_Motor_Movement(int l_speed, int r_speed);
void Set_Motor(DIRECTION dir, uint8_t pwm, uint8_t pin_fwd, uint8_t pin_bwd, uint8_t pin_pwm);
void Stop_All_Motors();
void Check_Inactivity();

// ----------------- Setup Function ------------------

void setup() {

  if (LOGGING_ENABLED) {
    Serial.begin(BAUDRATE::SERIAL_MONITOR);
  }

  bt_iot.begin(BAUDRATE::BLUETOOTH);
  Set_Pin_Modes();

  Stop_All_Motors();
}

// ----------------- Loop Function ------------------

void loop() {
  if (bt_iot.available()) {
    char bt_char = bt_iot.read();
    last_command_time = millis();

    int16_t pwm = Get_Speed_Stage(bt_char);
    if (pwm != M_SPEED::INVALID_SPEED) {
      current_speed = pwm;
      LOG("Speed Set: ");
      LOG_LN(current_speed);
      return;
    }

    int16_t l_speed = 0, r_speed = 0;
    switch (bt_char) {
      case 'F':  // Forward
        l_speed = r_speed = current_speed;
        break;
      case 'B':  // Backward
        l_speed = r_speed = -current_speed;
        break;
      case 'L':  // Turn left
        l_speed = -current_speed;
        r_speed = current_speed;
        break;
      case 'R':  // Turn right
        l_speed = current_speed;
        r_speed = -current_speed;
        break;
      default:  // Stop
        l_speed = 0;
        r_speed = 0;
        break;
    }

    Apply_Motor_Movement(l_speed, r_speed);
  }

  Check_Inactivity();
}

// ----------------- Function Implementations ------------------

// ----------------- Core Movement Logic ------------------
void Apply_Motor_Movement(int l_speed, int r_speed) {
  l_speed = constrain(l_speed, -255, 255);
  r_speed = constrain(r_speed, -255, 255);

  if (l_speed == prev_l_speed && r_speed == prev_r_speed) {  // avoid repeated commands
    return;
  }

  prev_l_speed = l_speed;
  prev_r_speed = r_speed;

  LOG("Motor_L: ");
  LOG(l_speed);
  LOG(" Motor_R: ");
  LOG_LN(r_speed);

  Set_Motor(l_speed > 0 ? DIRECTION::FORWARD : l_speed < 0 ? DIRECTION::BACKWARD
                                                           : DIRECTION::STOP,
            abs(l_speed), PIN::MOTOR_L_FORWARD, PIN::MOTOR_L_BACKWARD, PIN::MOTOR_L_PWM);

  Set_Motor(r_speed > 0 ? DIRECTION::FORWARD : r_speed < 0 ? DIRECTION::BACKWARD
                                                           : DIRECTION::STOP,
            abs(r_speed), PIN::MOTOR_R_FORWARD, PIN::MOTOR_R_BACKWARD, PIN::MOTOR_R_PWM);
}

void Set_Motor(DIRECTION dir, uint8_t pwm, uint8_t pin_fwd, uint8_t pin_bwd, uint8_t pin_pwm) {
  switch (dir) {
    case DIRECTION::FORWARD:
      digitalWrite(pin_fwd, HIGH);
      digitalWrite(pin_bwd, LOW);
      break;
    case DIRECTION::BACKWARD:
      digitalWrite(pin_fwd, LOW);
      digitalWrite(pin_bwd, HIGH);
      break;
    case DIRECTION::STOP:
      digitalWrite(pin_fwd, LOW);
      digitalWrite(pin_bwd, LOW);
      pwm = 0;
      break;
  }
  analogWrite(pin_pwm, pwm);
}


void Stop_All_Motors() {
  Set_Motor(DIRECTION::STOP, 0, PIN::MOTOR_L_FORWARD, PIN::MOTOR_L_BACKWARD, PIN::MOTOR_L_PWM);
  Set_Motor(DIRECTION::STOP, 0, PIN::MOTOR_R_FORWARD, PIN::MOTOR_R_BACKWARD, PIN::MOTOR_R_PWM);
}


void Set_Pin_Modes() {
  pinMode(PIN::MOTOR_R_FORWARD, OUTPUT);
  pinMode(PIN::MOTOR_R_BACKWARD, OUTPUT);
  pinMode(PIN::MOTOR_R_PWM, OUTPUT);

  pinMode(PIN::MOTOR_L_FORWARD, OUTPUT);
  pinMode(PIN::MOTOR_L_BACKWARD, OUTPUT);
  pinMode(PIN::MOTOR_L_PWM, OUTPUT);
}

// ----------------- Speed Mapping ------------------
int16_t Get_Speed_Stage(char bt_char) {
  switch (bt_char) {
    case '0': return M_SPEED::STAGE_0;
    case '1': return M_SPEED::STAGE_1;
    case '2': return M_SPEED::STAGE_2;
    case '3': return M_SPEED::STAGE_3;
    case '4': return M_SPEED::STAGE_4;
    case '5': return M_SPEED::STAGE_5;
    case '6': return M_SPEED::STAGE_6;
    case '7': return M_SPEED::STAGE_7;
    case '8': return M_SPEED::STAGE_8;
    case '9': return M_SPEED::STAGE_9;
    case 'q': return M_SPEED::STAGE_10;
    default: return M_SPEED::INVALID_SPEED;
  }
}

void Check_Inactivity() {
  if (millis() - last_command_time > COMMAND_TIMEOUT_MS) {
    Stop_All_Motors();
    prev_l_speed = 0;
    prev_r_speed = 0;
  }
}
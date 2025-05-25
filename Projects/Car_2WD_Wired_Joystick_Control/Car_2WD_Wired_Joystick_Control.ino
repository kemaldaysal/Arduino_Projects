
#include <stdint.h>

namespace DELAY {
constexpr uint16_t STARTUP = 500;
constexpr uint8_t TRANSMISSION = 50;

}

constexpr bool LOGGING_ENABLED = true;
constexpr uint32_t BAUDRATE_SERIAL = 9600;

#define LOG(x) \
  if (LOGGING_ENABLED) { Serial.print(x); }

#define LOG_LN(x) \
  if (LOGGING_ENABLED) { Serial.println(x); }

namespace PIN {

// Pins of the right motor
constexpr uint8_t MOTOR_R_FORWARD = 5;   // IN3
constexpr uint8_t MOTOR_R_BACKWARD = 4;  // IN4
constexpr uint8_t MOTOR_R_PWM = 3;       // ENB

// Pins of the left motor
constexpr uint8_t MOTOR_L_FORWARD = 7;   // IN2
constexpr uint8_t MOTOR_L_BACKWARD = 8;  // IN1
constexpr uint8_t MOTOR_L_PWM = 6;       // ENA

// Pins of the wired joysticks
constexpr uint8_t JOY_X = A0;
constexpr uint8_t JOY_Y = A1;
}

namespace JOY_SETTINGS {
constexpr uint16_t CENTER = 512;  // Raw center value of joystick
constexpr uint8_t DEADZONE = 20;
}

// Function declarations
void Set_Pin_Modes();

int Calculate_Joy_Val_Without_Deadzone(int value);
void Apply_Combined_Movement(int joy_x_raw, int joy_y_raw);
void Set_Motor_Speed(int motor_l_speed, int motor_r_speed);
void Motor_L_Go_Forward(uint8_t pwm_value);
void Motor_L_Go_Backward(uint8_t pwm_value);
void Motor_L_Cut_Power();
void Motor_R_Go_Forward(uint8_t pwm_value);
void Motor_R_Go_Backward(uint8_t pwm_value);
void Motor_R_Cut_Power();

void setup() {
  if (LOGGING_ENABLED) {
    Serial.begin(BAUDRATE_SERIAL);
  }

  Set_Pin_Modes();

  Motor_L_Cut_Power();
  Motor_R_Cut_Power();
  delay(DELAY::STARTUP);
}

void loop() {
  // 0,0 is top-left on joystick
  uint16_t joy_x_raw = analogRead(PIN::JOY_X);
  uint16_t joy_y_raw = analogRead(PIN::JOY_Y);

  LOG("joy_x_raw: ");
  LOG(joy_x_raw);
  LOG(", joy_y_raw: ");
  LOG(joy_y_raw);
  LOG(" ");

  Apply_Combined_Movement(joy_x_raw, joy_y_raw);

  delay(DELAY::TRANSMISSION);
}

void Set_Pin_Modes() {

  pinMode(PIN::JOY_X, INPUT);
  pinMode(PIN::JOY_Y, INPUT);

  pinMode(PIN::MOTOR_R_FORWARD, OUTPUT);
  pinMode(PIN::MOTOR_R_BACKWARD, OUTPUT);
  pinMode(PIN::MOTOR_R_PWM, OUTPUT);

  pinMode(PIN::MOTOR_L_FORWARD, OUTPUT);
  pinMode(PIN::MOTOR_L_BACKWARD, OUTPUT);
  pinMode(PIN::MOTOR_L_PWM, OUTPUT);
}

int Calculate_Joy_Val_Without_Deadzone(int value) {
  return ((abs(value) < JOY_SETTINGS::DEADZONE) ? 0 : value);
}

void Apply_Combined_Movement(int joy_x_raw, int joy_y_raw) {
  int joy_x = Calculate_Joy_Val_Without_Deadzone(joy_x_raw - JOY_SETTINGS::CENTER);  // Left/Right
  int joy_y = Calculate_Joy_Val_Without_Deadzone(JOY_SETTINGS::CENTER - joy_y_raw);  // Forward/Backward (invert Y)

  int motor_l_speed = joy_y + joy_x;
  int motor_r_speed = joy_y - joy_x;

  motor_l_speed = constrain(motor_l_speed, -255, 255);
  motor_r_speed = constrain(motor_r_speed, -255, 255);

  LOG("joy_x: ");
  LOG(joy_x);
  LOG(" joy_y: ");
  LOG(joy_y);
  LOG(" | motor_l_speed: ");
  LOG(motor_l_speed);
  LOG(" motor_r_speed: ");
  LOG_LN(motor_r_speed);


  Set_Motor_Speed(motor_l_speed, motor_r_speed);
}


void Set_Motor_Speed(int motor_l_speed, int motor_r_speed) {
  // Left Motor
  if (motor_l_speed > 0) {
    Motor_L_Go_Forward(motor_l_speed);
  } else if (motor_l_speed < 0) {
    Motor_L_Go_Backward(-motor_l_speed);
  } else {
    analogWrite(PIN::MOTOR_L_PWM, 0);
    digitalWrite(PIN::MOTOR_L_FORWARD, LOW);
    digitalWrite(PIN::MOTOR_L_BACKWARD, LOW);
  }

  // Right Motor
  if (motor_r_speed > 0) {
    Motor_R_Go_Forward(motor_r_speed);
  } else if (motor_r_speed < 0) {
    Motor_R_Go_Backward(-motor_r_speed);
  } else {
    analogWrite(PIN::MOTOR_R_PWM, 0);
    digitalWrite(PIN::MOTOR_R_FORWARD, LOW);
    digitalWrite(PIN::MOTOR_R_BACKWARD, LOW);
  }
}

void Motor_L_Go_Forward(uint8_t pwm_value) {
  digitalWrite(PIN::MOTOR_L_FORWARD, HIGH);
  digitalWrite(PIN::MOTOR_L_BACKWARD, LOW);
  analogWrite(PIN::MOTOR_L_PWM, pwm_value);
}

void Motor_L_Go_Backward(uint8_t pwm_value) {
  digitalWrite(PIN::MOTOR_L_FORWARD, LOW);
  digitalWrite(PIN::MOTOR_L_BACKWARD, HIGH);
  analogWrite(PIN::MOTOR_L_PWM, pwm_value);
}

void Motor_L_Cut_Power() {
  digitalWrite(PIN::MOTOR_L_FORWARD, LOW);
  digitalWrite(PIN::MOTOR_L_BACKWARD, LOW);
  analogWrite(PIN::MOTOR_L_PWM, 0);
}

void Motor_R_Go_Forward(uint8_t pwm_value) {
  digitalWrite(PIN::MOTOR_R_FORWARD, HIGH);
  digitalWrite(PIN::MOTOR_R_BACKWARD, LOW);
  analogWrite(PIN::MOTOR_R_PWM, pwm_value);
}

void Motor_R_Go_Backward(uint8_t pwm_value) {
  digitalWrite(PIN::MOTOR_R_FORWARD, LOW);
  digitalWrite(PIN::MOTOR_R_BACKWARD, HIGH);
  analogWrite(PIN::MOTOR_R_PWM, pwm_value);
}

void Motor_R_Cut_Power() {
  digitalWrite(PIN::MOTOR_R_FORWARD, LOW);
  digitalWrite(PIN::MOTOR_R_BACKWARD, LOW);
  analogWrite(PIN::MOTOR_R_PWM, 0);
}

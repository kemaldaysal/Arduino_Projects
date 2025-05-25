// TX Codes on Controller

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

constexpr uint8_t JOYSTICK_X = A0;
constexpr uint8_t JOYSTICK_Y = A1;
constexpr uint8_t BUTTON = 3;
}

namespace COMMS {
constexpr uint16_t BAUDRATE_SERIAL = 9600;
constexpr uint64_t CHANNEL_ADDRESS = 0xE8E8F0F0E1LL;
}

namespace CONTROL {
constexpr uint8_t BUTTON_DEBOUNCE_DELAY = 50;
}

constexpr uint8_t TX_DELAY = 20;

// ------ Global Variables

struct ControlData {
  uint16_t x;
  uint16_t y;
  bool led_state;
};

ControlData send_data = { 512, 512, false };

RF24 radio(PINS::RF_CE, PINS::RF_CSN);

uint8_t previous_button_state = LOW;
unsigned long last_debounce_time = 0;

// ------ Function Prototypes

void Setup_Pins();
void Setup_Radio();
void Read_Input();
void Transmit_Data();
void Data_Logger();

// ------ setup() Function

void setup() {
  LOG_BEGIN(COMMS::BAUDRATE_SERIAL);
  Setup_Pins();
  Setup_Radio();
  LOG_LN("Transmitter system initialized.");
}

// ------ loop() Function

void loop() {
  Read_Input();
  Transmit_Data();
  Data_Logger();
  delay(TX_DELAY);
}

// ------ Function Implementations

void Setup_Pins() {
  pinMode(PINS::JOYSTICK_X, INPUT);
  pinMode(PINS::JOYSTICK_Y, INPUT);
  pinMode(PINS::BUTTON, INPUT);
}

void Setup_Radio() {
  radio.begin();
  radio.openWritingPipe(COMMS::CHANNEL_ADDRESS);
  //radio.setPALevel(RF24_PA_MAX);
  //radio.setDataRate(RF24_250KBPS);
  radio.stopListening();
  LOG_LN("nRF24 is started in TX mode.");
}

void Read_Input() {
  send_data.x = analogRead(PINS::JOYSTICK_X);  // raw 0–1023
  send_data.y = analogRead(PINS::JOYSTICK_Y);  // raw 0–1023

  uint8_t current_button_state = digitalRead(PINS::BUTTON);

  if (current_button_state != previous_button_state) {
    if (millis() - last_debounce_time > CONTROL::BUTTON_DEBOUNCE_DELAY) {
      if (current_button_state == HIGH) {
        send_data.led_state = !send_data.led_state;
      }
      last_debounce_time = millis();
    }
    previous_button_state = current_button_state;
  }
}

void Transmit_Data() {
  radio.write(&send_data, sizeof(send_data));
}

void Data_Logger() {
  LOG("TX | x: ");
  LOG(send_data.x);
  LOG(" y: ");
  LOG(send_data.y);
  LOG(" led: ");
  LOG_LN(send_data.led_state);
}

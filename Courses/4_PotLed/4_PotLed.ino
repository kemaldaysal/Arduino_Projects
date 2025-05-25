#include <stdint.h>

namespace pin {
constexpr uint8_t POT = A0;
constexpr uint8_t LED = 6;
}

namespace adc_settings {
constexpr float ADC_VOLTAGE_MAX = 5.00f;
constexpr float ADC_VOLTAGE_MIN = 0.00f;

constexpr uint16_t ADC_RESOLUTION_DEF_MAX = 1023;
constexpr uint8_t ADC_RESOLUTION_DEF_MIN = 0;
}

constexpr uint16_t VAL_RANGE_MAX = 256;
constexpr uint8_t VAL_RANGE_MIN = 0;


void setup() {
  Serial.begin(9600);
  Serial.println("Pot Deger Okuma");
}

void loop() {
  int value = map(analogRead(pin::POT), adc_settings::ADC_RESOLUTION_DEF_MIN, adc_settings::ADC_RESOLUTION_DEF_MAX, VAL_RANGE_MIN, VAL_RANGE_MAX);
  float voltage = (adc_settings::ADC_VOLTAGE_MAX / VAL_RANGE_MAX) * value;
  analogWrite(pin::LED, value);
  Serial.println(voltage);
}

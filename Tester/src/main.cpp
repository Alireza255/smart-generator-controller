#include <Arduino.h>

//#define DEBUG_MODE

#define TRIGGER_OUT_PIN 23
#define TRIGGER_ADJUST_PIN 4

#define TRIGGER_WHEEL_TOTAL_TEETH 60
#define TRIGGER_WHEEL_MISSING_TEETH 2


bool trigger_pattern[TRIGGER_WHEEL_TOTAL_TEETH] = {0};
uint16_t trigger_wheel_rpm = 1000;
uint32_t trigger_tooth_time_gap = 0;

uint32_t map_uint(uint32_t x, uint32_t x_min, uint32_t x_max, uint32_t result_min, uint32_t result_max)
{
  return ((((x - x_min)*(result_max - result_min))/(x_max - x_min)) + result_min);
}

uint32_t round_uint(uint32_t x, uint8_t digits)
{
  uint32_t factor = 1;
  for (uint8_t i = 0; i < digits; i++)
  {
    factor *= 10;
  }
  return ((x + factor / 2) / factor) * factor;
}
uint32_t filter_smooth_uint(uint32_t x)
{
  static uint32_t previous_value = 0;
  const float alpha = 0.9999; // Extreme smoothing factor (very close to 1)

  uint32_t smoothed_value = (alpha * x) + ((1 - alpha) * previous_value);
  previous_value = smoothed_value;

  return smoothed_value;
}

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);

  pinMode(TRIGGER_OUT_PIN, OUTPUT);
  pinMode(TRIGGER_ADJUST_PIN, INPUT);
  analogSetAttenuation(ADC_2_5db);

  for (uint8_t i = 0; i < TRIGGER_WHEEL_TOTAL_TEETH; i++)
  {
    if (i < (TRIGGER_WHEEL_TOTAL_TEETH - TRIGGER_WHEEL_MISSING_TEETH))
    {
      trigger_pattern[i] = 1;
    }
    else
    {
      trigger_pattern[i] = 0;
    }
  }

  trigger_tooth_time_gap = 1000000 / trigger_wheel_rpm;
  

}

void loop() {
  uint16_t trigger_adjust_value_raw = analogRead(TRIGGER_ADJUST_PIN);
  trigger_adjust_value_raw = filter_smooth_uint(trigger_adjust_value_raw);
  trigger_wheel_rpm = map_uint(trigger_adjust_value_raw, 0, 1600, 50, 10000);
  trigger_wheel_rpm = round_uint(trigger_wheel_rpm, 1);


  trigger_wheel_rpm = 5000;

  trigger_tooth_time_gap = 1000000 / trigger_wheel_rpm;
  

  #ifdef DEBUG_MODE
  Serial.print("raw value: ");
  Serial.print(trigger_adjust_value_raw);
  Serial.print("  Mapped RPM: ");
  Serial.println(trigger_wheel_rpm);
  #endif



    
//Rest of code here that does some boring stuff and is known to 100% work every time.

  for (uint8_t i = 0; i < TRIGGER_WHEEL_TOTAL_TEETH; i++)
  {
    digitalWrite(TRIGGER_OUT_PIN, trigger_pattern[i]);
    delayMicroseconds(trigger_tooth_time_gap / 2);
    digitalWrite(TRIGGER_OUT_PIN, 0);
    delayMicroseconds(trigger_tooth_time_gap / 2);

  }

  
}

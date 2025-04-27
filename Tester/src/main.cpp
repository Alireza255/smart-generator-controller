#include <Arduino.h>

#define TRIGGER_OUT_PIN 23

#define TRIGGER_WHEEL_TOTAL_TEETH 60
#define TRIGGER_WHEEL_MISSING_TEETH 2


bool trigger_pattern[TRIGGER_WHEEL_TOTAL_TEETH] = {0};
uint16_t trigger_wheel_rpm = 1000;
uint32_t trigger_tooth_time_gap = 0;

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);

  pinMode(TRIGGER_OUT_PIN, OUTPUT);
  
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
  // put your main code here, to run repeatedly:
  if (Serial.available() > 0) {
    byte incomingByte = 0;
    incomingByte = Serial.read();
    uint16_t previous_rpm = trigger_wheel_rpm;

    switch (incomingByte)
    {
    case '+':
        if (trigger_wheel_rpm < 2000)
        {
          trigger_wheel_rpm += 100;
        }
      break;
    case '-':
        if (trigger_wheel_rpm > 5)
        {
          trigger_wheel_rpm -= 100;
        }
    default:
      break;
    }
    if (previous_rpm != trigger_wheel_rpm)
    {
      trigger_tooth_time_gap = 1000000 / trigger_wheel_rpm;
    }
    
  }
//Rest of code here that does some boring stuff and is known to 100% work every time.

  for (uint8_t i = 0; i < TRIGGER_WHEEL_TOTAL_TEETH; i++)
  {
    digitalWrite(TRIGGER_OUT_PIN, trigger_pattern[i]);
    delayMicroseconds(trigger_tooth_time_gap / 2);
    digitalWrite(TRIGGER_OUT_PIN, 0);
    delayMicroseconds(trigger_tooth_time_gap / 2);
  }

  
}

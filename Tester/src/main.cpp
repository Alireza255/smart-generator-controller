#include <Arduino.h>

//#define DEBUG_MODE

/**
 * Important notes i found out the hard way :(
 * Strapping Pins: GPIO0, GPIO2, GPIO12, and GPIO15 are used during the boot process.
 * Avoid using them for analog input to prevent unexpected behavior.
 */


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

// ADC pins (excluding GPIO25 and GPIO26 which are used for DAC)
const int analogPins[] = {32, 33, 34, 35, 36, 37, 38, 39, 27};
const int numPins = sizeof(analogPins) / sizeof(analogPins[0]);

// DAC pins
const int dacPins[] = {25, 26}; // GPIO25 = DAC1, GPIO26 = DAC2
const int numDacPins = sizeof(dacPins) / sizeof(dacPins[0]);

int dacValue = 0;
int step = 5;

void setup() {
  Serial.begin(115200);
  delay(1000); // Allow time for serial monitor to initialize
  Serial.println("ESP32 Analog Input and DAC Output Test");
}

void loop() {
  Serial.println("Analog Input Readings:");
  for (int i = 0; i < numPins; i++) {
    int pin = analogPins[i];
    int value = analogRead(pin);
    Serial.print("GPIO");
    Serial.print(pin);
    Serial.print(": ");
    Serial.println(value);
  }

  Serial.println("Writing to DAC Pins:");
  for (int i = 0; i < numDacPins; i++) {
    int pin = dacPins[i];
    dacWrite(pin, dacValue);
    Serial.print("GPIO");
    Serial.print(pin);
    Serial.print(" (DAC): ");
    Serial.println(dacValue);
  }

  dacValue += step;
  if (dacValue >= 255 || dacValue <= 0) {
    step = -step;
  }

  Serial.println("-----------------------------");
  delay(100);
}

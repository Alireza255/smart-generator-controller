#include <Arduino.h>
#include <LiquidCrystal.h>

// ----------------- CONFIG -----------------
const int HALL_OUT_PIN = 25;      // simulated Hall sensor output pin
const int POT_PIN = 34;           // ADC pin for RPM pot
const int BUTTON_PIN = 35;        // wheel-select button (uses pull-up)
const int SCOPE_SYNC_OUT = 26;    // output pin for oscilloscope sync (goes high on each rotation)
// LCD pin mapping
const int LCD_RS = 4;
const int LCD_EN = 5;
const int LCD_D4 = 18;
const int LCD_D5 = 19;
const int LCD_D6 = 22;
const int LCD_D7 = 23;

LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

const uint32_t RPM_MIN = 0;
const uint32_t RPM_MAX = 5000;

const uint32_t DISPLAY_REFRESH_RATE_MS = 200;

const bool wheel_pattern[] = {
  0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
};

bool is_inverted = false;


// ----------------- Setup -----------------
void setup() {
  Serial.begin(115200);
  pinMode(HALL_OUT_PIN, OUTPUT);
  pinMode(SCOPE_SYNC_OUT, OUTPUT);
  digitalWrite(SCOPE_SYNC_OUT, LOW);
  digitalWrite(HALL_OUT_PIN, LOW);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  analogReadResolution(12);
#ifdef ARDUINO_ARCH_ESP32
  analogSetPinAttenuation(POT_PIN, ADC_11db);
#endif

  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Trigger Sim v2");
  delay(800);

}

static uint8_t tooth_index = 0;

void loop() {


  digitalWrite(HALL_OUT_PIN, wheel_pattern[tooth_index] ^ is_inverted);
  delay(1);
  digitalWrite(HALL_OUT_PIN, is_inverted);
  delay(1);
  if (tooth_index < sizeof(wheel_pattern)/sizeof(wheel_pattern[0]) - 1) {
    tooth_index++;
    digitalWrite(SCOPE_SYNC_OUT, LOW);
  } else {
    tooth_index = 0;
    digitalWrite(SCOPE_SYNC_OUT, HIGH);
  }


}

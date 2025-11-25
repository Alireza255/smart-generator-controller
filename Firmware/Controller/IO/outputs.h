#ifndef OUTPUTS_H
#define OUTPUTS_H

#include "main.h"
#include "types.h"
#include "constants.h"
#include "cmsis_os.h"

extern controller_output_pin_t main_relay_output;
extern controller_output_pin_t gas_solenoid_output;
extern controller_output_pin_t fuel_pump_output;
extern controller_output_pin_t fan1_output;
extern controller_output_pin_t fan2_output;
extern controller_output_pin_t injector_output[FIRMWARE_INJECTOR_OUTPUT_COUNT];
extern controller_output_pin_t ignition_output[FIRMWARE_IGNITION_OUTPUT_COUNT];

void output_main_relay_set(bool state);

void output_gas_solenoid_set(bool state);

void output_fuel_pump_set(bool state);

void output_fan1_set(bool state);

void output_fan2_set(bool state);

void output_set(controller_output_pin_t *pin, bool state);
void output_override(controller_output_pin_t *pin, bool forced_state);
void output_override_clear(controller_output_pin_t *pin);

void output_fire_ignition(controller_output_pin_t *pin);
void output_fire_injection(controller_output_pin_t *pin);

#endif //OUTPUTS_H



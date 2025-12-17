#ifndef TIMING_LIGHT_H
#define TIMING_LIGHT_H

#include "types.h"
#include "timing.h"
#include "outputs.h"

#define TIMING_LIGHT_PULSE_LENGTH_MICROSECONDS 100

void timing_light_pulse_a_pin(controller_output_pin_t *pin);

#endif // TIMING_LIGHT_H
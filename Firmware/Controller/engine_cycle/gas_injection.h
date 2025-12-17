#ifndef GAS_INJECTION_H
#define GAS_INJECTION_H

#include "types.h"
#include "math.h"
#include "fuel_math.h"
#include "electronic_throttle.h"
#include "outputs.h"

#define FIRMWARE_GAS_INJECTION_MIN_MASS (mass_t)0.1

void gas_injection_init(electronic_throttle_t *gas_control_etb);

void gas_injection_update(time_us_t timestamp);

#endif //GAS_INJECTION_H
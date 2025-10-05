#ifndef GAS_INJECTION_H
#define GAS_INJECTION_H

#include "types.h"
#include "math.h"
#include "fuel_math.h"
#include "electronic_throttle.h"

void gas_injection_init(electronic_throttle_t *gas_control_etb);

void gas_injection_update(void *arg);

#endif //GAS_INJECTION_H
#ifndef TRIGGER_SIMULATOR_H
#define TRIGGER_SIMULATOR_H

#include "types.h"
#include "timing.h"
#include "utils.h"
#include "trigger.h"

void trigger_simulator_init(uint8_t full_teeth, uint8_t missing_teeth, void (*cb)(trigger_t *arg), trigger_t *arg);

void trigger_simulator_update(rpm_t rpm);


#endif // TRIGGER_SIMULATOR_H
#ifndef TRIGGER_SIMULATOR_H
#define TRIGGER_SIMULATOR_H

#include "types.h"
#include "timing.h"
#include "utils.h"
#include "trigger.h"

void trigger_simulator_init(uint8_t full_teeth, uint8_t missing_teeth, void (*cb_cam)(bool edge), void (*cb_crank)(void));

void trigger_simulator_update(rpm_t rpm);

void trigger_simulator_start();
void trigger_simulator_stop();

#endif // TRIGGER_SIMULATOR_H
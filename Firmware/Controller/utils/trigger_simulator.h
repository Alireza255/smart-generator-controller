#ifndef TRIGGER_SIMULATOR_H
#define TRIGGER_SIMULATOR_H

#include "types.h"
#include "timing.h"
#include "utils.h"
#include "trigger.h"

typedef enum
{
    TRIGGER_SIMULATOR_WHEEL_TYPE_60_2,
} trigger_simulator_wheel_type_t;


void trigger_simulator_crank_init(trigger_simulator_wheel_type_t wheel_type, osMessageQueueId_t trigger_mq_id);

void trigger_simulator_set_rpm_and_start(rpm_t rpm);

void trigger_simulator_update();

void trigger_simulator_stop();

#endif // TRIGGER_SIMULATOR_H
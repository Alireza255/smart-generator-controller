#ifndef FAN_CONTROL_H
#define FAN_CONTROL_H

#include "types.h"
#include "error_handling.h"
#include "sensors.h"
#include "controller.h"

typedef struct
{
    controller_output_pin_t pin;
    temperature_t temp_off;
    temperature_t temp_on;
    uint32_t on_flag;
} fan_control_t;

void fan_control_update(fan_control_t *fan);

#endif // FAN_CONTROL_H
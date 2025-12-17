#ifndef INJECTION_H
#define INJECTION_H

#include "controller.h"
#include "fuel_math.h"
#include "outputs.h"


#define INJECTION_DENSITY_OF_PETROL_GRAM_PER_CC 0.675f
#define INJECTION_END_OF_INJECTION_ADVANCE_MIN (angle_t)0
#define INJECTION_END_OF_INJECTION_ADVANCE_MAX (angle_t)180

typedef enum
{
    INJECTION_EVENT_INACTIVE = 0,
    INJECTION_EVENT_PENDING = 1,
    INJECTION_EVENT_INJECTING = 2,
    INJECTION_EVENT_FINISHED = 3,
} injection_event_status_t;

typedef struct
{
    time_us_t injection_start_time;
    time_us_t injection_stop_time;
    controller_output_pin_t *primary_output;
    controller_output_pin_t *secondary_output;
    angle_t crank_angle_at_end_of_injection;
    injection_event_status_t status;
} injection_event_t;


void injection_trigger_event_handle(angle_t crankshaft_angle, rpm_t rpm, time_us_t current_time_us);

bool injection_init();

void injection_disable();

#endif
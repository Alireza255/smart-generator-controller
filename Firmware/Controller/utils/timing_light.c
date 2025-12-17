#include "timing_light.h"

static void turn_off_pin(void *arg);

void timing_light_pulse_a_pin(controller_output_pin_t *pin)
{
    if (pin == NULL)
    {
        return;
    }
    
    time_us_t current_time = get_time_us();
    time_us_t light_turn_off_time = current_time + TIMING_LIGHT_PULSE_LENGTH_MICROSECONDS;
    scheduler_schedule_event_with_arg(light_turn_off_time, turn_off_pin, (void*)pin);
    output_override(pin, true);
}


static void turn_off_pin(void *arg)
{
    if (arg == NULL)
    {
        return;
    }
    controller_output_pin_t *pin = (controller_output_pin_t*)arg;
    output_override_clear(pin);
}
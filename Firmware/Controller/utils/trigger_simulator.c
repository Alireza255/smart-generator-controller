#include "trigger_simulator.h"
#include <stdint.h> // Ensure uint8_t, int32_t are defined

uint8_t wheel_full_teeth = 0;
uint8_t wheel_missing_teeth = 0;

static void (*trigger_callback)(trigger_t *arg);  //passing an argument
static trigger_t *trigger_callback_arg;           // data to pass to callback_arg

void trigger_simulator_init(uint8_t full_teeth, uint8_t missing_teeth, void (*cb)(trigger_t *arg), trigger_t *arg)
{
    wheel_full_teeth = full_teeth;
    wheel_missing_teeth = missing_teeth;
    trigger_callback = cb;
    trigger_callback_arg = arg;
}

void trigger_simulator_update(rpm_t rpm)
{
    if (trigger_callback == NULL)
    {
        return;
    }
    if (wheel_full_teeth == 0) {
        return; // Prevent division by zero
    }
    time_us_t tooth_interval = microseconds_per_degree(rpm) * 360 / wheel_full_teeth;

    time_us_t current_time = get_time_us();
    static time_us_t prev_time = 0;


    if ((int32_t)(current_time - prev_time) < tooth_interval)
    {
        return;
    }
    prev_time = current_time;
    
    static uint8_t current_tooth_index = 0;

    if (current_tooth_index < wheel_missing_teeth)
    {
        // welll do nothing!
    }
    else
    {
        trigger_callback(trigger_callback_arg);
    }
    

    if (current_tooth_index < (wheel_full_teeth - 1))
    {
        current_tooth_index++;
    }
    else
    {
        current_tooth_index = 0;
    }


}


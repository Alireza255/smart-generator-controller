#include "trigger_simulator.h"

uint8_t wheel_full_teeth = 0;
uint8_t wheel_missing_teeth = 0;
void (*wheel_callback)(void) = NULL;

void trigger_simulator_init(uint8_t full_teeth, uint8_t missing_teeth, void (*callback)(void))
{
    wheel_full_teeth = full_teeth;
    wheel_missing_teeth = missing_teeth;
    wheel_callback = callback;
}

void trigger_simulator_update(rpm_t rpm)
{
    if (wheel_callback == NULL)
    {
        return;
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
        wheel_callback();
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


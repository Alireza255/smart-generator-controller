#include "trigger_simulator.h"
#include <stdint.h> // Ensure uint8_t, int32_t are defined

uint8_t wheel_full_teeth = 0;
uint8_t wheel_missing_teeth = 0;

static bool cam_state = false;
static bool trigger_simulator_running = false;
static void (*trigger_callback_cam)(bool edge) = NULL;  // pointer to function taking no args, returns void
static void (*trigger_callback_crank)(void) = NULL;  // pointer to function taking no args, returns void

void trigger_simulator_init(uint8_t full_teeth, uint8_t missing_teeth, void (*cb_cam)(bool edge), void (*cb_crank)(void))
{
    wheel_full_teeth = full_teeth;
    wheel_missing_teeth = missing_teeth;
    trigger_callback_cam = cb_cam;
    trigger_callback_crank = cb_crank;
}

void trigger_simulator_start()
{
    trigger_simulator_running = true;
}
void trigger_simulator_stop()
{
    trigger_simulator_running = false;
}


void trigger_simulator_update(rpm_t rpm)
{
    if (trigger_callback_cam == NULL || trigger_callback_crank == NULL)
    {
        return;
    }
    if (wheel_full_teeth == 0) {
        return; // Prevent division by zero
    }
    if (rpm == 0 || !trigger_simulator_running)
    {
        osDelay(1);
        return;
    }
    
    time_us_t tooth_interval = (time_us_t)roundf((float)microseconds_per_degree(rpm) * (float)360 / (float)wheel_full_teeth / (float)1000);

    osDelay(tooth_interval);
    
    static uint8_t current_tooth_index = 0;

    if (current_tooth_index < wheel_missing_teeth)
    {
        // welll do nothing!
    }
    else
    {
        trigger_callback_crank();
    }
    

    if (current_tooth_index < (wheel_full_teeth - 1))
    {
        current_tooth_index++;
    }
    else
    {
        current_tooth_index = 0;
    }

    if (current_tooth_index == 0)
    {
        cam_state = !cam_state;
        trigger_callback_cam(cam_state);
    }

    
}


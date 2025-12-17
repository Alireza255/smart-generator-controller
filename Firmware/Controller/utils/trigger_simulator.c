#include "trigger_simulator.h"


static osMessageQueueId_t trigger_event_queue_id = NULL;

rpm_t simulated_rpm = (rpm_t)0;

static bool *selected_wheel_pattern = NULL;
static uint8_t selected_wheel_pattern_length = 0;
const bool wheel_pattern_60_2[] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0};

void trigger_simulator_crank_init(trigger_simulator_wheel_type_t wheel_type, osMessageQueueId_t trigger_mq_id)
{
    if (trigger_mq_id == NULL)
    {
        return;
    }
    
    trigger_event_queue_id = trigger_mq_id;
    switch (wheel_type)
    {
    case TRIGGER_SIMULATOR_WHEEL_TYPE_60_2:
        selected_wheel_pattern_length = SIZE_OF_ARRAY(wheel_pattern_60_2);
        selected_wheel_pattern = &wheel_pattern_60_2[0];
        break;
    
    default:
        break;
    }

}

void trigger_simulator_update()
{
    if (trigger_event_queue_id == NULL || simulated_rpm == (rpm_t)0 || selected_wheel_pattern == NULL || selected_wheel_pattern_length == 0)
    {
        osDelay(100);
        return;
    }
    time_ms_t tooth_gap = (time_ms_t)(60000.0f / (simulated_rpm * selected_wheel_pattern_length));
    if (tooth_gap <= 0)
    {
        tooth_gap = 1;
    }
    
    static uint_fast8_t tooth_pointer = 0;
    
    if (selected_wheel_pattern[tooth_pointer] == 1)
    {
        // Send a trigger event to the queue
        trigger_event_t event = {0}; // Initialize event as needed
        event.type = TRIGGER_EVENT_TYPE_CRANKSHAFT;
        event.edge = TRIGGER_EVENT_EDGE_FALLING;
        event.timestamp = get_time_us();
        osMessageQueuePut(trigger_event_queue_id, &event, 0, 0);
    }
    
    if (tooth_pointer < (selected_wheel_pattern_length - 1))
    {
        tooth_pointer++;
    }
    else
    {
        tooth_pointer = 0;
    }
    
    osDelay(tooth_gap);
}

void trigger_simulator_set_rpm_and_start(rpm_t rpm)
{
    simulated_rpm = rpm;
}
void trigger_simulator_stop()
{
    simulated_rpm = (rpm_t)0;
}

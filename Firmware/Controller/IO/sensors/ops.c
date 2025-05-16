#include "ops.h"


bool sensor_ops_get()
{
    static bool state = false;
    static const uint16_t threshold_high = 2500;
    static const uint16_t threshold_low = 1800;

    uint16_t raw_adc_value = 0;
    raw_adc_value = analog_inputs_get_data(ANALOG_INPUT_SENSOR_OIL_PIN);

    if (raw_adc_value > threshold_high)
    {
        state = true;
    }
    else if (raw_adc_value < threshold_low)
    {
        state = false;
    }

    return state;
}
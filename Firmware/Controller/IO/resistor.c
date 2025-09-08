#include "resistor.h"
#include "float.h"
void resistor_init(resistor_t* resistor, float pull_resistor, resistor_pull_type_t pull_type, analog_input_channel_t analog_input)
{
    if (resistor == NULL)
    {
        log_error("Resistor init failed!");
        return;
    }
    resistor->pull_resistor = pull_resistor;
    resistor->pull_type = pull_type;
    resistor->analog_input = analog_input;
}

/**
 * @todo div by zero will break this!
 */
float resistor_get_resistance(const resistor_t *resistor)
{
    volatile float voltage = analog_inputs_get_voltage(resistor->analog_input);
    // simple check to see if the sensor is open circuit or not
    if (!IS_IN_RANGE(voltage, 0.05f * CONTROLLER_REF_VOLTAGE, 0.95f * CONTROLLER_REF_VOLTAGE))
    {
        return NAN;
    }
    if (resistor->pull_type == RESISTOR_PULL_UP) {
        return (voltage * (float)resistor->pull_resistor) / ((float)CONTROLLER_REF_VOLTAGE - voltage);
    } else { // PULL_DOWN
        if(voltage == 0) return INFINITY;
        return ((CONTROLLER_REF_VOLTAGE - voltage) * (float)resistor->pull_resistor) / voltage;
    }
}

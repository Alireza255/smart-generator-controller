#include "resistor.h"

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
    uint16_t adc_value = analog_inputs_get_data(resistor->analog_input);
    float voltage = ((float)adc_value / (float)ADC_MAX_VALUE) * (float)ADC_REF_VOLTAGE;
    // simple check to see if the sensor is open circuit or not
    if (!IS_IN_RANGE(voltage, 0.05f * ADC_REF_VOLTAGE, 0.95f * ADC_REF_VOLTAGE))
    {
        log_error("Resistor open circuit.");
        return NAN;
    }
    if (resistor->pull_type == RESISTOR_PULL_UP) {
        return (voltage * (float)resistor->pull_resistor) / ((float)ADC_REF_VOLTAGE - voltage);
    } else { // PULL_DOWN
        return ((ADC_REF_VOLTAGE - voltage) * (float)resistor->pull_resistor) / voltage;
    }
}

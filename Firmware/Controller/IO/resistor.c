#include "resistor.h"

void resistor_init(resistor_s* resistor, float pull_resistor, resistor_pull_type_e pull_type, analog_input_adc_channel_mapping_e analog_input)
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


float resistor_get_resistance(const resistor_s *resistor)
{
    uint16_t adc_value = analog_inputs_get_data(resistor->analog_input);
    float voltage = (adc_value / ADC_MAX_VALUE) * ADC_REF_VOLTAGE;
    // simple check to see if the sensor is open circuit or not
    if (!IS_IN_RANGE(voltage, 0.05f * ADC_REF_VOLTAGE, 0.95f * ADC_REF_VOLTAGE))
    {
        log_error("Resistor open circuit.");
        return NAN;
    }
    if (resistor->pull_type == RESISTOR_PULL_UP) {
        return (voltage * resistor->pull_resistor) / (ADC_REF_VOLTAGE - voltage);
    } else { // PULL_DOWN
        return ((ADC_REF_VOLTAGE - voltage) * resistor->pull_resistor) / voltage;
    }
}

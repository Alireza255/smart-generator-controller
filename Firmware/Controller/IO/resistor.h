#ifndef RESISTOR_H
#define RESISTOR_H

#include <math.h>
#include "analog_inputs.h"
#include "error_handling.h"
#include "utils.h"

typedef enum {
    RESISTOR_PULL_UP,
    RESISTOR_PULL_DOWN
} resistor_pull_type_e;

/**
 * @struct resistor_s
 * @brief Represents a resistor configuration.
 * 
 * @var resistor_s::pull_type
 * The type of pull configuration for the resistor (e.g., pull-up, pull-down).
 * 
 * @var resistor_s::pull_resistor
 * The resistance value of the pull resistor in ohms.
 * 
 * @var resistor_s::analog_input
 * The analog input channel associated with the resistor.
 */
typedef struct {
    resistor_pull_type_e pull_type;
    float pull_resistor;
    analog_input_adc_channel_mapping_e analog_input;
} resistor_s;

/**
 * @brief Initializes a resistor configuration.
 * 
 * @param resistor Pointer to the resistor structure to initialize.
 * @param pull_resistor Value of the pull resistor in ohms.
 * @param pull_type Type of pull configuration (e.g., pull-up or pull-down).
 * @param analog_input Analog input channel mapping associated with the resistor.
 */
void resistor_init(resistor_s* resistor, float pull_resistor, resistor_pull_type_e pull_type, analog_input_adc_channel_mapping_e analog_input);

/**
 * @brief Calculates the resistance of a resistor based on ADC input and configuration.
 * 
 * This function reads the ADC value corresponding to the resistor's analog input,
 * calculates the voltage, and determines the resistance using the pull-up or pull-down
 * configuration of the resistor.
 * 
 * @param resistor Pointer to a resistor_s structure containing resistor configuration.
 * @return The calculated resistance in ohms, or NAN if the sensor is open circuit.
 */
float resistor_get_resistance(const resistor_s *resistor);

#endif // RESISTOR_H
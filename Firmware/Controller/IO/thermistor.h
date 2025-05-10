#ifndef THERMISTOR_H
#define THERMISTOR_H

// this will include the hal functions we will be using
#include "resistor.h"
#include "math.h"
#include "error_handling.h"

/**
 * @brief Thermistor known values

*/
// start of thermistor_conf_s
typedef struct
{
	/**
	 * these values are in Celcius
	 * units: *C
	 */
	float tempC_1;
	/**
	 * units: *C
	 */
	float tempC_2;
	/**
	 * units: *C
	 */
	float tempC_3;
	/**
	 * units: Ohm
	 */
	float resistance_1;
	/**
	 * units: Ohm
	 */
	float resistance_2;
	/**
	 * units: Ohm
	 */
	float resistance_3;
} thermistor_conf_s;

typedef struct
{
    resistor_s *resistor;
    float m_a;
    float m_b;
    float m_c;
} thermistor_s;

/**
 * @brief Initializes a thermistor instance with the specified configuration.
 * 
 * @param thermistor Pointer to the thermistor instance to initialize.
 * @param config Pointer to the configuration structure for the thermistor.
 */
void thermistor_init(thermistor_s *thermistor, thermistor_conf_s cfg);

/**
 * @brief Retrieves the temperature reading from the specified thermistor.
 * 
 * @param thermistor Pointer to the thermistor structure.
 * @return The temperature value in C.
 * @note Returns NAN if invalid temp is measured.
 */
temperature_t thermistor_get_temp(thermistor_s *thermistor);

#endif // THERMISTOR_H
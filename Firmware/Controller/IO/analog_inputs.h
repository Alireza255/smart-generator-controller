#ifndef ANALOG_INPUTS_H
#define ANALOG_INPUTS_H

#include "stm32f4xx_hal.h"
#include "types.h"
#include "utils.h"
#include "cmsis_os2.h"

/**
 * @brief Analog inputs measurement and (optional) filtering
 * Here i have decided to use an intresting tactic to deal with noise issues.
 * It works by sampling the analog inputs not just once every 1mS but many times more.
 * This is only possible due to the amazing power of stm32 microcontrollers, specifically,
 * their fast ADC and DMA which can do the conversion and transfer the results to the memory
 * without the involvment of the CPU.
 */
/**
 * @note it is very important that this matches the stm32cubemx configuration
 */
#define ANALOG_INPUTS_MAX 13
#define ADC_MAX_VALUE 4095
#define ADC_REF_VOLTAGE 3.3f
#define ANALOG_INPUTS_NUMBER_OF_SAMPLES 8

/**
 * @enum analog_input_adc_channel_mapping_e
 * @brief Enumeration for mapping ADC conversion ranks to analog input pins.
 *
 * This enumeration defines the mapping between ADC conversion ranks and the corresponding
 * analog input pins used in the system. Each entry represents a specific analog input
 * and its associated ADC conversion rank.
 *
 * Mappings:
 * - ANALOG_INPUT_ETB1_SENSE1: ADC Conversion Rank 0, used for ETB1 Sense 1.
 * - ANALOG_INPUT_ETB1_SENSE2: ADC Conversion Rank 1, used for ETB1 Sense 2.
 * - ANALOG_INPUT_ETB2_SENSE1: ADC Conversion Rank 2, used for ETB2 Sense 1.
 * - ANALOG_INPUT_ETB2_SENSE2: ADC Conversion Rank 3, used for ETB2 Sense 2.
 * - ANALOG_INPUT_SENSOR_MAP_PIN: ADC Conversion Rank 4, used for MAP Sensor.
 * - ANALOG_INPUT_SENSOR_MAT_PIN: ADC Conversion Rank 5, used for MAT Sensor.
 * - ANALOG_INPUT_SENSOR_CLT_PIN: ADC Conversion Rank 6, used for CLT Sensor.
 * - ANALOG_INPUT_SENSOR_OIL_PIN: ADC Conversion Rank 7, used for Oil Sensor.
 * - ANALOG_INPUT_VBAT_SENSE_PIN: ADC Conversion Rank 8, used for VBAT Sense.
 * - ANALOG_INPUT_AD_INPUT1_PIN: ADC Conversion Rank 9, used for AD Input 1.
 * - ANALOG_INPUT_AD_INPUT2_PIN: ADC Conversion Rank 10, used for AD Input 2.
 * - ANALOG_INPUT_AD_INPUT3_PIN: ADC Conversion Rank 11, used for AD Input 3.
 * - ANALOG_INPUT_AD_INPUT4_PIN: ADC Conversion Rank 12, used for AD Input 4.
 */
typedef enum
{
    ANALOG_INPUT_ETB1_SENSE1 = 1,
    ANALOG_INPUT_ETB1_SENSE2 = 2,
    ANALOG_INPUT_ETB2_SENSE1 = 3,
    ANALOG_INPUT_ETB2_SENSE2 = 4,
    ANALOG_INPUT_SENSOR_MAP_PIN = 5,
    ANALOG_INPUT_SENSOR_IAT_PIN = 6,
    ANALOG_INPUT_SENSOR_CLT_PIN = 7,
    ANALOG_INPUT_SENSOR_OIL_PIN = 8,
    ANALOG_INPUT_VBAT_SENSE_PIN = 9,
    ANALOG_INPUT_AD_INPUT1_PIN = 10,
    ANALOG_INPUT_AD_INPUT2_PIN = 11,
    ANALOG_INPUT_AD_INPUT3_PIN = 12,
} analog_input_channel_t;


/**
 * @brief Initializes the analog inputs module.
 * 
 * This function sets up the ADC peripheral and prepares the analog inputs
 * module for operation.
 * 
 * @param hadc Pointer to the ADC_HandleTypeDef structure that contains
 *             the configuration information for the specified ADC.
 * @param instance Pointer to the analog_inputs_s structure instance
 *                 that holds the state and configuration of the analog inputs.
 */
void analog_inputs_init(ADC_HandleTypeDef *hadc);

/**
 * @brief Starts the ADC conversion for analog inputs.
 * 
 * This function triggers the ADC to begin converting the analog signals
 * to digital values.
 */
void analog_inputs_start_conversion();

/**
 * @brief Retrieves the converted data for a specific analog input.
 * 
 * This function returns the digital value corresponding to the specified
 * analog input index.
 * 
 * @param input_index The index of the analog input to retrieve data from.
 *                    This index corresponds to the specific channel of the ADC.
 * @return The digital value of the specified analog input.
 */
uint16_t analog_inputs_get_data(analog_input_channel_t input_index);


voltage_t analog_inputs_get_voltage(analog_input_channel_t input_index);

#endif // ANALOG_INPUTS_H
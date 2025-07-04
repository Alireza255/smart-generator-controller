#include "analog_inputs.h"
#include "error_handling.h"

ADC_HandleTypeDef *hadc = NULL;
static volatile uint16_t analog_data[ANALOG_INPUTS_MAX * ANALOG_INPUTS_NUMBER_OF_SAMPLES];

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{

    
}

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
void analog_inputs_init(ADC_HandleTypeDef *adc_handle)
{
    if (adc_handle == NULL)
    {
        log_error("Analog input init failed! adc handle is null.");
        return;
    }
    
    hadc = adc_handle;

    /**
    * @todo this is just a temporary fix make this a proper thing
    */
    osTimerId_t timer_id = osTimerNew(
        analog_inputs_start_conversion,
        osTimerPeriodic,
        NULL,
        NULL
    );
    osTimerStart(timer_id, 1);

}

/**
 * @brief Starts the ADC conversion for analog inputs.
 * 
 * This function triggers the ADC to begin converting the analog signals
 * to digital values.
 */
void analog_inputs_start_conversion()
{
    if (hadc == NULL)
    {
        return;
    }
    
    HAL_ADC_Start_DMA(hadc, (uint32_t*)&analog_data, ANALOG_INPUTS_MAX * ANALOG_INPUTS_NUMBER_OF_SAMPLES);
}

void analog_inputs_start_conversion_injected()
{

}

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
uint16_t analog_inputs_get_data(analog_input_channel_t input_index)
{
    if (input_index >= ANALOG_INPUTS_MAX)
    {
        log_error("Invalid index in analog input.");
        return 0;
    }
    uint_fast32_t sum = 0;
    for (uint_fast8_t i = 0; i < ANALOG_INPUTS_NUMBER_OF_SAMPLES; i++)
    {
        sum += analog_data[i * ANALOG_INPUTS_MAX + input_index];
    }
    
    return sum / ANALOG_INPUTS_NUMBER_OF_SAMPLES;
}

voltage_t analog_inputs_get_voltage(analog_input_channel_t input_index)
{
    voltage_t v = 0;
    uint16_t raw_data = 0;
    raw_data = analog_inputs_get_data(input_index);
    v = mapf((float)raw_data, (float)0, (float)4095, 0, ADC_REF_VOLTAGE);
    return v;
}
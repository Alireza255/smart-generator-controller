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
void analog_inputs_init(ADC_HandleTypeDef *adc_handle, TIM_HandleTypeDef *timer_handle)
{
    if (adc_handle == NULL || timer_handle == NULL)
    {
        
        log_error("Analog input init failed! adc or timer handle is null.");
        return;
    }
    
    hadc = adc_handle;

    const uint16_t frequency = 1000;
	
	uint32_t timer_clock = HAL_RCC_GetPCLK1Freq(); // Get the timer clock frequency
	uint16_t prescaler = 0;
	uint32_t auto_reload = 0;

	// Calculate prescaler and auto-reload values
	for (prescaler = 0; prescaler <= 0xFFFF; prescaler++)
	{
		auto_reload = (timer_clock / ((prescaler + 1) * frequency)) - 1;
		
		if (auto_reload <= 0xFFFF)
		{
			break;

		}
	}

	if (prescaler > 0xFFFF || auto_reload > 0xFFFF)
	{
		// Frequency is too low, cannot configure timer
		return;
	}
	
	// Set the prescaler and auto-reload values
	__HAL_TIM_SET_PRESCALER((TIM_HandleTypeDef *)timer_handle, prescaler);
	__HAL_TIM_SET_AUTORELOAD((TIM_HandleTypeDef *)timer_handle, auto_reload);
    __HAL_TIM_SET_COMPARE(timer_handle, TIM_CHANNEL_1, 0);
    
    HAL_ADC_Start_DMA(hadc, (uint32_t*)&analog_data, ANALOG_INPUTS_MAX * ANALOG_INPUTS_NUMBER_OF_SAMPLES);

    HAL_TIM_Base_Start(timer_handle);
    HAL_TIM_OC_Start(timer_handle, TIM_CHANNEL_1);


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
    if (input_index != ANALOG_INPUT_VBAT_SENSE_PIN && analog_inputs_get_data(ANALOG_INPUT_VBAT_SENSE_PIN) < (uint16_t)(ANALOG_INPUTS_MIN_VBAT / VBAT_DIVIDER_RATIO / ADC_REF_VOLTAGE * (float)4095))
    {
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
    v = mapf((float)raw_data, (float)0, (float)4095, 0, CONTROLLER_REF_VOLTAGE);
    return v;
}
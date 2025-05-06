#include "ignition.h"


/**
 * @brief Initializes the ignition system with the specified parameters.
 * 
 * @param htim Pointer to a TIM_HandleTypeDef structure that contains
 *             the configuration information for the TIM peripheral.
 * @param ignition_mode The mode of ignition to be used.
 * @param pin Pointer to a structure of type ignition_output_pins_s
 *            that defines the output pins for the ignition system.
 */
void ignition_init(TIM_HandleTypeDef *htim, ignition_mode_e ignition_mode, ignition_output_pins_s *pin)
{
    if (htim == NULL || pin == NULL)
    {
        /**
         * @todo thorow an error
         */
        return;
    }
    if (engine.cylinder_count == 0)
    {
        return;
        /**
         * @todo throw an error
         */
    }
    
    if (configuration.ignition_is_multi_spark && !IS_IN_RANGE(configuration.ignition_multi_spark_number_of_sparks, 0, IGNITION_MULTI_SPARK_MAX_SPARKS))
    {
        /**
         * @todo throw a warning and tell the user that multi spark is enabled but the number of additional sparks is 0
         */
    }
    
    // Configure the timer to have a tick duration of 1 microsecond
    uint32_t timer_clock = HAL_RCC_GetSysClockFreq(); // Assuming the timer clock is the same is system clock
    uint32_t prescaler = (timer_clock / 1000000UL) - 1;

    if (prescaler > 0xFFFF)
    {
        // @todo throw an error if the prescaler value is out of range
        return;
    }

    htim->Instance->PSC = prescaler; // Set the prescaler
    htim->Instance->ARR = 0xFFFFFFFF; // Set auto-reload register to max for 32-bit timer
    htim->Instance->CNT = 0; // Reset the counter

    // Enable the output compare no output mode for the first channel using HAL
    TIM_OC_InitTypeDef sConfigOC = {0};
    sConfigOC.OCMode = TIM_OCMODE_TIMING; // No output mode
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

    if (HAL_TIM_OC_ConfigChannel(htim, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
    {
        // @todo handle the error
        return;
    }

    HAL_TIM_Base_Start_IT(htim); // Start the timer in interrupt mode

    __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, 0);
    HAL_TIM_OC_Start_IT(htim , TIM_CHANNEL_1); // start the output compare mode and enable interrupts
    
    
    switch (configuration.firing_order)
    {
        case FO_1342:
            engine.cylinder_count = 4;
            
        break;
        
        default:
        break;
    }
    
    
    engine.firing_interval = (angle_t)720 / engine.cylinder_count;
}

/**
 * @brief Handles an ignition trigger event based on the crankshaft angle, RPM, and current time.
 * 
 * @param crankshaft_angle The current angle of the crankshaft.
 * @param rpm The current revolutions per minute of the engine.
 * @param current_time_us The current time in microseconds.
 */
void ignition_trigger_event_handle(angle_t crankshaft_angle, rpm_t rpm, time_us_t current_time_us)
{
    /**
     * @todo add the necessary checks and bounds
     */
    if (engine.ignition_mode == IM_NO_IGNITION)
    {
        // obviously, there is no need to do any furthure processing
        return;
    }
    
    if (!IS_IN_RANGE(configuration.ignition_dwell, IGNITION_MIN_DWELL_TIME_MS, IGNITION_MAX_DWELL_TIME_MS))
    {
        /**
         * @todo throw an error
         */
        return;
    }

    uint8_t sparks_per_coil = ignition_get_number_of_sparks_per_coil(engine.ignition_mode);
    uint8_t aditional_sparks_per_coil = configuration.ignition_multi_spark_number_of_sparks;

    

}

/**
 * @brief Schedules the next spark event for the ignition system.
 */
void ignition_schedule_spark();

/**
 * @brief Gets the number of sparks per coil based on the ignition mode.
 * 
 * @param ignition_mode The mode of ignition, specified as an enumeration of type ignition_mode_e.
 * @return The number of sparks per coil.
 */
uint8_t ignition_get_number_of_sparks_per_coil(ignition_mode_e ignition_mode)
{
    switch (ignition_mode) {
        case IM_ONE_COIL:
            return engine.cylinder_count;
        case IM_INDIVIDUAL_COILS:
            return 1;
        case IM_WASTED_SPARK:
            return 2;
        default:
            //firmwareError(ObdCode::CUSTOM_ERR_IGNITION_MODE, "Unexpected ignition_mode_e %d", mode);
            return 1;
        }
}

/**
 * @brief Charges the ignition coil at the specified index.
 * 
 * @param coil_index The index of the coil to be charged.
 */
void ignition_coil_charge(uint8_t coil_index);

/**
 * @brief Fires a spark from the ignition coil at the specified index.
 * 
 * @param coil_index The index of the coil to fire the spark from.
 */
void ignition_coil_fire_spark(uint8_t coil_index);

/**
 * @brief Gets the duty cycle of the ignition coil as a percentage.
 * 
 * @return The duty cycle of the ignition coil.
 */
percent_t ignition_get_coil_duty_cycle();

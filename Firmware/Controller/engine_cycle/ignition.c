#include "ignition.h"

ignition_output_pin_s ignition_outputs[IGNITION_MAX_OUTPUTS] = {0};

uint8_t ignition_order[IGNITION_MAX_OUTPUTS] = {0};

static TIM_HandleTypeDef *timer = 0;

static uint8_t current_firing_cylinder = 0;


void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance != timer->Instance)
    {
        return;
    }
    
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
    {
        ignition_coil_begin_charge(current_firing_cylinder - 1);
    }
    else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
    {
        ignition_coil_fire_spark(current_firing_cylinder- 1);
    }
}

void ignition_init(TIM_HandleTypeDef *htim,  ignition_output_conf_s *output_conf)
{
    if (htim == NULL || output_conf == NULL)
    {
        log_error("ignition init failed. No timer or ouput config");
        return;
    }

    timer = htim;

    switch (configuration.firing_order)
    {
        case FO_1342:
            engine.cylinder_count = 4;
            ignition_order[0] = 1;
            ignition_order[1] = 3;
            ignition_order[2] = 4;
            ignition_order[3] = 2;
        break;
        
        default:
            log_error("ignition init failed. unkown firing order.");
            return;
        break;
    }
    
    
    engine.firing_interval = (angle_t)720 / engine.cylinder_count;


    for (size_t i = 0; i < IGNITION_MAX_OUTPUTS; i++)
    {
        ignition_outputs[i].gpio = output_conf->output[i].gpio;
        ignition_outputs[i].pin = output_conf->output[i].pin;
    }
    
    if (configuration.ignition_is_multi_spark && !IS_IN_RANGE(configuration.ignition_multi_spark_number_of_sparks, 0, IGNITION_MULTI_SPARK_MAX_SPARKS))
    {
        log_warning("Multi spark is enabled but number of sparks are 0.");
    }
    

    // Configure the timer to have a tick duration of 1 microsecond
    uint32_t timer_clock = HAL_RCC_GetSysClockFreq(); // Assuming the timer clock is the same is system clock
    uint32_t prescaler = (timer_clock / 1000000UL) - 1;
    
    if (prescaler > 0xFFFF)
    {
        // @todo throw an error if the prescaler value is out of range
        return;
    }
    
    timer->Instance->PSC = prescaler; // Set the prescaler
    timer->Instance->ARR = 0xFFFFFFFF; // Set auto-reload register to max for 32-bit timer
    timer->Instance->CNT = 0; // Reset the counter

    // Enable the output compare no output mode for the first channel using HAL
    TIM_OC_InitTypeDef sConfigOC = {0};
    sConfigOC.OCMode = TIM_OCMODE_TIMING; // No output mode
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    
    if (HAL_TIM_OC_ConfigChannel(timer, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
    {
        // @todo handle the error
        return;
    }
    // Enable the output compare no output mode for the second channel using HAL
    if (HAL_TIM_OC_ConfigChannel(timer, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
    {
        // @todo handle the error
        return;
    }
    HAL_TIM_Base_Start(timer);
    HAL_TIM_PWM_Start_IT(timer, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start_IT(timer, TIM_CHANNEL_2);
    
    

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
    if (configuration.ignition_mode == IM_NO_IGNITION)
    {
        // obviously, there is no need to do any furthure processing
        return;
    }
    
    /**
     * @todo add the necessary checks and bounds
     */
    if (timer == NULL)
    {
        log_error("ignition no init.");
        return;
    }
    
    if (!IS_IN_RANGE(configuration.ignition_dwell, IGNITION_MIN_DWELL_TIME_MS, IGNITION_MAX_DWELL_TIME_MS))
    {
        /**
         * @todo throw an error
         */
        return;
    }
    
    #ifdef TEST_MODE
    angle_t spark_advance = (angle_t)30;
    #else
    angle_t spark_advance = spark_logic_get_advance();
    #endif

    spark_advance = CLAMP(spark_advance, IGNITION_MIN_ADVANCE, IGNITION_MAX_ADVANCE);

    //uint8_t sparks_per_coil = ignition_get_number_of_sparks_per_coil(configuration.ignition_mode);
    //uint8_t aditional_sparks_per_coil = configuration.ignition_multi_spark_number_of_sparks;
    
    /**
     * Here we detect which cylinder has to fire next
     */
    // Determine the current phase
    uint8_t phase = (uint8_t)(ceilf(crankshaft_angle / engine.firing_interval));

    
    /**
     * for a simple 4 cylinder engine this returns:
     * 1 when 0 < crankshaft angle < 180
     * 3 when 180 < crankshaft angle < 360
     */
    current_firing_cylinder = ignition_order[phase-1];

    /**
     * Now we need to start the timer and calculate when the coil has to start charging and when it has to fire the spark
     */
    angle_t spark_angle = (angle_t)phase * engine.firing_interval - spark_advance;
    angle_t dwell_angle = degrees_per_microsecond(rpm) * configuration.ignition_dwell * (float)1000;
    if ((spark_angle - dwell_angle - crankshaft_angle) < 0)
    {
        /* This means that the spark is eather happened or is happening or maybe just about to happen(we are in dwell time)*/
        return;
    }
    time_us_t start_charge_time = 0;
    time_us_t fire_spark_time = 0;
    time_us_t microseconds_per_deg = microseconds_per_degree(rpm);
    start_charge_time = (time_us_t)((spark_angle - dwell_angle - crankshaft_angle) * (float)microseconds_per_deg);
    fire_spark_time = (time_us_t)((spark_angle - crankshaft_angle) * (float)microseconds_per_deg);
    __HAL_TIM_SET_COUNTER(timer, 0);
    __HAL_TIM_SET_COMPARE(timer, TIM_CHANNEL_1, start_charge_time);
    __HAL_TIM_SET_COMPARE(timer, TIM_CHANNEL_2, fire_spark_time);

}


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
            log_error("Unkown ignition mode");
            return 1;
        }
}

/**
 * @brief Charges the ignition coil at the specified index.
 * 
 * @param coil_index The index of the coil to be charged.
 */
void ignition_coil_begin_charge(uint8_t coil_index)
{
    if (coil_index > IGNITION_MAX_OUTPUTS - 1)
    {
        log_error("Unkown ignition output");
        return;
    }
    HAL_GPIO_WritePin(ignition_outputs[coil_index].gpio, ignition_outputs[coil_index].pin, GPIO_PIN_SET);
}

/**
 * @brief Fires a spark from the ignition coil at the specified index.
 * 
 * @param coil_index The index of the coil to fire the spark from.
 */
void ignition_coil_fire_spark(uint8_t coil_index)
{
    if (coil_index > IGNITION_MAX_OUTPUTS - 1)
    {
        log_error("Unkown ignition output");
        return;
    }
    HAL_GPIO_WritePin(ignition_outputs[coil_index].gpio, ignition_outputs[coil_index].pin, GPIO_PIN_RESET);
}

/**
 * @brief Gets the duty cycle of the ignition coil as a percentage.
 * 
 * @return The duty cycle of the ignition coil.
 */
percent_t ignition_get_coil_duty_cycle();

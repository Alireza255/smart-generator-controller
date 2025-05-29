#include "ignition.h"

ignition_output_pin_s ignition_outputs[IGNITION_MAX_OUTPUTS] = {0};

volatile ignition_coil_state_e ignition_coil_state[IGNITION_MAX_OUTPUTS] = {0};

uint8_t ignition_order[IGNITION_MAX_OUTPUTS] = {0};

static volatile bool spark_is_in_progress = false;


void ignition_init(ignition_output_conf_s *output_conf)
{
    if (output_conf == NULL)
    {
        log_error("ignition init failed. No output config");
        return;
    }


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
    /* Read the pin states from real hardware */
    for (size_t i = 0; i < IGNITION_MAX_OUTPUTS; i++)
    {
        ignition_coil_state[i] = HAL_GPIO_ReadPin(ignition_outputs[i].gpio, ignition_outputs->pin);
    }
    /* Set all outputs to reset just in case */
    for (size_t i = 0; i < IGNITION_MAX_OUTPUTS; i++)
    {
        HAL_GPIO_WritePin(ignition_outputs[i].gpio, ignition_outputs->pin, GPIO_PIN_RESET);
    }
    
    if (configuration.ignition_is_multi_spark && !IS_IN_RANGE(configuration.ignition_multi_spark_number_of_sparks, 0, IGNITION_MULTI_SPARK_MAX_SPARKS))
    {
        log_warning("Multi spark is enabled but number of sparks are 0.");
    }
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

    if (configuration.ignition_mode == IM_NO_IGNITION)
    {
        // obviously, there is no need to do any furthure processing
        return;
    }
    
    if (!IS_IN_RANGE(configuration.ignition_dwell, IGNITION_MIN_DWELL_TIME_MS, IGNITION_MAX_DWELL_TIME_MS))
    {
        /**
         * @todo throw an error
         */
        log_error("ignition dwell out of bounds.");
        return;
    }
    /* Wether or not cam phase is known */
    bool is_cam_phase_available = false;

    angle_t spark_advance = 0;
    #ifdef TEST_MODE
    spark_advance = (angle_t)30;
    #else
    spark_advance = spark_logic_get_advance();
    #endif

    spark_advance = CLAMP(spark_advance, IGNITION_MIN_ADVANCE, IGNITION_MAX_ADVANCE);
    
    uint8_t multi_spark_count = 0;
    if (configuration.ignition_is_multi_spark && rpm < configuration.ignition_multi_spark_rpm_threshold)
    {
        multi_spark_count = configuration.ignition_multi_spark_number_of_sparks;
    }
    

    /* First we have to detect the engine phase */
    uint8_t phase = (uint8_t)(crankshaft_angle / engine.firing_interval);
    uint8_t phase_count = (uint8_t)(angle_t)360 / (angle_t)engine.firing_interval;

    volatile angle_t next_spark_angle = phase * engine.firing_interval - spark_advance + engine.firing_interval;
    volatile angle_t next_dwell_angle = next_spark_angle - (float)configuration.ignition_dwell * (float)1000 * degrees_per_microsecond(rpm);
    
    static uint8_t next_firing_cylinders[2] = {0};

    next_firing_cylinders[0] = ignition_order[phase] - 1;
    next_firing_cylinders[1] = ignition_order[phase] - 1;

    if (!is_cam_phase_available)
    {
        next_firing_cylinders[1] = ignition_order[phase + 2] - 1;
        
    }
    

    bool is_synced = engine.trigger.sync_status == TS_FULLY_SYNCED;
    if (is_synced && (next_dwell_angle - crankshaft_angle) < 10 && !spark_is_in_progress && (next_dwell_angle - crankshaft_angle) > 0)
    {
        /* Now we are close enough to the dwell angle that we can schedule the coil charge start */
        spark_is_in_progress = true;
        time_us_t dwell_start_time_us = current_time_us + (time_us_t)((next_dwell_angle - crankshaft_angle) * microseconds_per_degree(rpm));
        time_us_t spark_start_time_us = current_time_us + (time_us_t)((next_spark_angle - crankshaft_angle) * microseconds_per_degree(rpm));
        scheduler_schedule_event_with_arg(dwell_start_time_us, ignition_coil_begin_charge, (void*)next_firing_cylinders);
        scheduler_schedule_event_with_arg(spark_start_time_us, ignition_coil_fire_spark, (void*)next_firing_cylinders);

    }
    
}


/**
 * @brief Gets the number of sparks per coil based on the ignition mode.
 * 
 * @param ignition_mode The mode of ignition, specified as an enumeration of type ignition_mode_e.
 * @return The number of sparks per coil.
 */
uint8_t ignition_get_number_of_sparks_per_coil(ignition_mode_e ignition_mode)
{
    switch (ignition_mode)
    {
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
void ignition_coil_begin_charge(void *arg)
{
    if (arg == NULL)
    {
        return;
    }
    uint8_t *coil_index = (uint8_t*)arg;

    if (coil_index[1] > IGNITION_MAX_OUTPUTS - 1)
    {
        log_error("Unkown ignition output");
        return;
    }
    ignition_coil_state[coil_index[0]] = IGNITION_COIL_STATE_CHARGING;
    ignition_coil_state[coil_index[1]] = IGNITION_COIL_STATE_CHARGING;
    HAL_GPIO_WritePin(ignition_outputs[coil_index[0]].gpio, ignition_outputs[coil_index[0]].pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(ignition_outputs[coil_index[1]].gpio, ignition_outputs[coil_index[1]].pin, GPIO_PIN_SET);
}

/**
 * @brief Fires a spark from the ignition coil at the specified index.
 * 
 * @param coil_index The index of the coil to fire the spark from.
 */
void ignition_coil_fire_spark(void *arg)
{
    if (arg == NULL)
    {
        return;
    }
    
    uint8_t *coil_index = (uint8_t*)arg;
    if (coil_index[1] > IGNITION_MAX_OUTPUTS - 1)
    {
        log_error("Unkown ignition output");
        return;
    }
    
    ignition_coil_state[coil_index[0]] = IGNITION_COIL_STATE_NOT_CHARGING;
    ignition_coil_state[coil_index[1]] = IGNITION_COIL_STATE_NOT_CHARGING;
    HAL_GPIO_WritePin(ignition_outputs[coil_index[0]].gpio, ignition_outputs[coil_index[0]].pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(ignition_outputs[coil_index[1]].gpio, ignition_outputs[coil_index[1]].pin, GPIO_PIN_RESET);
    spark_is_in_progress = false;
}

/**
 * @brief Gets the duty cycle of the ignition coil as a percentage.
 * 
 * @return The duty cycle of the ignition coil.
 */
percent_t ignition_get_coil_duty_cycle()
{

}

angle_t ignition_get_advance()
{
    angle_t final_advance = IGNITION_ADVANCE_FAIL_SAFE;
    if (engine.spinning_state != SS_RUNNING)
    {
        return configuration.cranking_advance;
    }
    
    temperature_t intake_air_temp = sensor_iat_get();
    rpm_t rpm = crankshaft_get_rpm();
    pressure_t map = sensor_map_get();

    final_advance += table_2d_get_value(&configuration.ignition_table, rpm, map);
    
    /* Here we can apply all kinds of correction to the table */

    return final_advance;
}
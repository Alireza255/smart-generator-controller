#include "ignition.h"

controller_output_pin_t *ignition_outputs = NULL;

volatile ignition_coil_state_t ignition_coil_state[FIRMWARE_IGNITION_OUTPUT_COUNT] = {0};

uint8_t ignition_order[FIRMWARE_LIMIT_NUMBER_OF_CYLINDERS_MAX] = {0};

static volatile bool spark_is_in_progress = false;

static uint8_t number_of_cylinders = 0;

bool ignition_init(controller_output_pin_t *outputs)
{
    if (outputs == NULL)
    {
        change_bit(&runtime.status, STATUS_IGNITION_ERROR, true);
        log_error("ignition init failed. No output config");
        return false;
    }
    ignition_outputs = outputs;

    switch (config.firing_order)
    {
    case FO_1342: // Common inline-4
        number_of_cylinders = 4;
        switch (config.ignition_mode)
        {
        case IM_ONE_COIL:
            ignition_order[0] = 1;
            ignition_order[1] = 1;
            ignition_order[2] = 1;
            ignition_order[3] = 1;
            break;
        case IM_WASTED_SPARK:
            ignition_order[0] = 1;
            ignition_order[1] = 2;
            ignition_order[2] = 2;
            ignition_order[3] = 1;
            break;
        case IM_INDIVIDUAL_COILS:
            ignition_order[0] = 1;
            ignition_order[1] = 3;
            ignition_order[2] = 4;
            ignition_order[3] = 2;
            break;

        default:
            change_bit(&runtime.status, STATUS_IGNITION_ERROR, true);
            log_error("ignition init failed. unkown mode.");
            return false;
        }

        break;
    case FO_153624: // Common inline-6
        number_of_cylinders = 6;
        switch (config.ignition_mode)
        {
        case IM_ONE_COIL:
            ignition_order[0] = 1;
            ignition_order[1] = 1;
            ignition_order[2] = 1;
            ignition_order[3] = 1;
            ignition_order[4] = 1;
            ignition_order[5] = 1;
            break;
        case IM_WASTED_SPARK:
            ignition_order[0] = 1;
            ignition_order[1] = 2;
            ignition_order[2] = 3;
            ignition_order[3] = 3;
            ignition_order[4] = 2;
            ignition_order[5] = 1;
            break;
        case IM_INDIVIDUAL_COILS:
            config.ignition_mode = IM_WASTED_SPARK;
            ignition_order[0] = 1;
            ignition_order[1] = 2;
            ignition_order[2] = 3;
            ignition_order[3] = 3;
            ignition_order[4] = 2;
            ignition_order[5] = 1;
            log_warning("Coil on plug igntion not possible with 6 cylinders. Defaulted to wasted spark");
            break;

        default:
        change_bit(&runtime.status, STATUS_IGNITION_ERROR, true);
            log_error("ignition init failed. unkown mode.");
            return false;
        }
        break;
    default:
        change_bit(&runtime.status, STATUS_IGNITION_ERROR, true);
        log_error("ignition init failed. unkown firing order.");
        return false;
    }

    runtime.firing_interval_deg = (angle_t)720 / (angle_t)number_of_cylinders;

    /* Read the pin states from real hardware */
    for (size_t i = 0; i < FIRMWARE_IGNITION_OUTPUT_COUNT; i++)
    {
        ignition_coil_state[i] = HAL_GPIO_ReadPin(ignition_outputs[i].gpio, ignition_outputs[i].pin);
    }
    /* Set all outputs to reset just in case */
    for (size_t i = 0; i < FIRMWARE_IGNITION_OUTPUT_COUNT; i++)
    {
        HAL_GPIO_WritePin(ignition_outputs[i].gpio, ignition_outputs[i].pin, GPIO_PIN_RESET);
    }

    if (config.multi_spark_enabled && !IS_IN_RANGE(config.multi_spark_number_of_sparks, 0, IGNITION_MULTI_SPARK_MAX_SPARKS))
    {
        log_warning("Multi spark is enabled but number of sparks are 0.");
    }
    return true;
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
    if (config.ignition_mode == IM_NO_IGNITION)
    {
        // obviously, there is no need to do any furthure processing
        //ignition_coil_fire_spark()????
        return;
    }
    if (runtime.firing_interval_deg == 0)
    {
        change_bit(&runtime.status, STATUS_IGNITION_ERROR, true);
        log_error("ignition not initialized.");
        return;
    }

    if (!IS_IN_RANGE(config.ignition_dwell, IGNITION_MIN_DWELL_TIME_MS, IGNITION_MAX_DWELL_TIME_MS))
    {
        config.ignition_dwell = CLAMP(config.ignition_dwell, IGNITION_MIN_DWELL_TIME_MS, IGNITION_MAX_DWELL_TIME_MS);
        /**
         * @todo throw an error
         */
        change_bit(&runtime.status, STATUS_IGNITION_ERROR, true);
         log_error("ignition dwell out of bounds.");

        return;
    }
    /* Wether or not cam phase is known */
    bool is_cam_phase_available = false;

    angle_t spark_advance = 0;

#ifdef TEST_MODE
    spark_advance = (angle_t)30;
#else
    spark_advance = ignition_get_advance();
    runtime.ignition_advance_deg = spark_advance;
#endif

    spark_advance = CLAMP(spark_advance, IGNITION_MIN_ADVANCE, IGNITION_MAX_ADVANCE);
    runtime.dwell_actual = config.ignition_dwell;

    /* First we have to detect the engine phase */
    uint8_t phase = (uint8_t)(crankshaft_angle / runtime.firing_interval_deg);
    uint8_t phase_count = (uint8_t)(angle_t)360 / (angle_t)runtime.firing_interval_deg;

    volatile angle_t next_spark_angle = phase * runtime.firing_interval_deg - spark_advance + runtime.firing_interval_deg;
    volatile angle_t next_dwell_angle = next_spark_angle - (float)config.ignition_dwell * (float)1000 * degrees_per_microsecond(rpm);

    static uint8_t next_firing_cylinders[2] = {0};

    next_firing_cylinders[0] = ignition_order[phase] - 1;
    next_firing_cylinders[1] = ignition_order[phase] - 1;

    if (!is_cam_phase_available)
    {
        next_firing_cylinders[1] = ignition_order[phase + 2] - 1;
    }

    bool is_synced = get_bit(runtime.status, STATUS_TRIGGER1_SYNCED);

    if (is_synced && (next_dwell_angle - crankshaft_angle) < 10 && !spark_is_in_progress && (next_dwell_angle - crankshaft_angle) > 0)
    {
        /* Now we are close enough to the dwell angle that we can schedule the coil charge start */

        time_us_t dwell_start_time_us = current_time_us + (time_us_t)((next_dwell_angle - crankshaft_angle) * microseconds_per_degree(rpm));
        time_us_t spark_start_time_us = current_time_us + (time_us_t)((next_spark_angle - crankshaft_angle) * microseconds_per_degree(rpm));
        scheduler_schedule_event_with_arg(dwell_start_time_us, ignition_coil_begin_charge, (void *)next_firing_cylinders);
        scheduler_schedule_event_with_arg(spark_start_time_us, ignition_coil_fire_spark, (void *)next_firing_cylinders);

        uint8_t number_of_scheduled_sparks = 1;
        if (!config.multi_spark_enabled)
        {
            runtime.multi_spark_actual_spark_count = number_of_scheduled_sparks;
            return;
        }

        while (number_of_scheduled_sparks < config.multi_spark_number_of_sparks)
        {
            if (rpm > config.multi_spark_rpm_threshold)
            {
                break;
            }
            time_us_t duration_of_spark_and_dwell = (uint32_t)((float)number_of_scheduled_sparks * (config.multi_spark_rest_time_ms + config.ignition_dwell) * (float)1000);
            if (duration_of_spark_and_dwell >= config.multi_spark_max_trailing_angle * microseconds_per_degree(rpm))
            {
                break;
            }

            dwell_start_time_us += duration_of_spark_and_dwell;
            spark_start_time_us += duration_of_spark_and_dwell;

            // don't schedule events if there are no empty slots in the scheduler
            if (!scheduler_schedule_event_with_arg(spark_start_time_us, ignition_coil_fire_spark, (void *)next_firing_cylinders))
            {
                // fire the spark right away to prevent ign coil damage
                ignition_coil_fire_spark((void *)next_firing_cylinders);
                break;
            }
            if (!scheduler_schedule_event_with_arg(dwell_start_time_us, ignition_coil_begin_charge, (void *)next_firing_cylinders))
            {

                break;
            }
            number_of_scheduled_sparks++;
        }
        spark_is_in_progress = true;
        runtime.multi_spark_actual_spark_count = number_of_scheduled_sparks;
    }
    change_bit(&runtime.status, STATUS_IGNITION_ERROR, false);
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
    uint8_t *coil_index = (uint8_t *)arg;

    if (coil_index[1] > FIRMWARE_IGNITION_OUTPUT_COUNT - 1 || coil_index[0] > FIRMWARE_IGNITION_OUTPUT_COUNT - 1)
    {
        change_bit(&runtime.status, STATUS_IGNITION_ERROR, true);
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

    uint8_t *coil_index = (uint8_t *)arg;
    if (coil_index[1] > FIRMWARE_IGNITION_OUTPUT_COUNT - 1 || coil_index[0] > FIRMWARE_IGNITION_OUTPUT_COUNT - 1)
    {
        change_bit(&runtime.status, STATUS_IGNITION_ERROR, true);
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
    if (runtime.spinning_state != SS_RUNNING)
    {
        return config.cranking_advance;
    }

    rpm_t rpm = crankshaft_get_rpm();
    pressure_t map = sensor_map_get();

    final_advance = table_2d_get_value(&config.ign_table_1, rpm, map); // expand and allow the use of table2 in the future

    /* Here we can apply all kinds of correction to the table */

    return final_advance;
}
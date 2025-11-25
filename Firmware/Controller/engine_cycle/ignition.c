#include "ignition.h"

void schedule_ignition_event(ignition_event_t *event);
void ignition_switch_to_coil_on_plug_wasted_spark();
void ignition_switch_to_coil_on_plug();
void ignition_watchdog_check();

static bool ignition_initialized = false;
static uint8_t number_of_cylinders = 0;
static uint8_t number_of_events = 0;

static bool operating_in_forced_wasted_spark = false;

// We are going to have at most the same number of events as the number of cylinders
static ignition_event_t ignition_events[FIRMWARE_LIMIT_NUMBER_OF_CYLINDERS_MAX] = {0};

bool ignition_init()
{

    controller_output_pin_t *outputs = &ignition_output[0];

    switch (config.firing_order)
    {
    case FO_1342: // Common inline-4
        number_of_cylinders = 4;
        switch (config.ignition_mode)
        {
        case IGNITION_MODE_ONE_COIL:
            ignition_events[0].primary_output = &outputs[0];
            ignition_events[1].primary_output = &outputs[0];

            ignition_events[0].crank_angle_at_tdc = (angle_t)0;
            ignition_events[1].crank_angle_at_tdc = (angle_t)180;

            number_of_events = 2;
            break;
        case IGNITION_MODE_WASTED_SPARK:
            ignition_events[0].crank_angle_at_tdc = (angle_t)0;
            ignition_events[1].crank_angle_at_tdc = (angle_t)180;

            ignition_events[0].primary_output = &outputs[0];
            ignition_events[1].primary_output = &outputs[1];

            number_of_events = 2;
            break;
        case IGNITION_MODE_COIL_ON_PLUG_WASTED_SPARK:
            ignition_events[0].crank_angle_at_tdc = (angle_t)0;
            ignition_events[1].crank_angle_at_tdc = (angle_t)180;

            ignition_events[0].primary_output = &outputs[0];
            ignition_events[0].secondary_output = &outputs[3];

            ignition_events[1].primary_output = &outputs[1];
            ignition_events[1].secondary_output = &outputs[2];

            number_of_events = 2;
            break;
        case IGNITION_MODE_COIL_ON_PLUG:
            ignition_events[0].crank_angle_at_tdc = (angle_t)0;
            ignition_events[2].crank_angle_at_tdc = (angle_t)180;
            ignition_events[3].crank_angle_at_tdc = (angle_t)360;
            ignition_events[1].crank_angle_at_tdc = (angle_t)540;

            ignition_events[0].primary_output = &outputs[0];
            ignition_events[1].primary_output = &outputs[1];
            ignition_events[2].primary_output = &outputs[2];
            ignition_events[3].primary_output = &outputs[3];

            number_of_events = 4;
            break;
        default:
            break;
        }
        break;
    case FO_153624: // Common inline-6
        number_of_cylinders = 6;
        switch (config.ignition_mode)
        {
        case IGNITION_MODE_ONE_COIL:
            ignition_events[0].crank_angle_at_tdc = (angle_t)0;   // cyl 1 & 6
            ignition_events[1].crank_angle_at_tdc = (angle_t)120; // cyl 5 & 2
            ignition_events[2].crank_angle_at_tdc = (angle_t)240; // cyl 3 & 4

            ignition_events[0].primary_output = &outputs[0];
            ignition_events[1].primary_output = &outputs[0];
            ignition_events[2].primary_output = &outputs[0];
            number_of_events = 3;
            break;

        case IGNITION_MODE_WASTED_SPARK:
            ignition_events[0].crank_angle_at_tdc = (angle_t)0;   // cyl 1 & 6
            ignition_events[1].crank_angle_at_tdc = (angle_t)120; // cyl 5 & 2
            ignition_events[2].crank_angle_at_tdc = (angle_t)240; // cyl 3 & 4

            ignition_events[0].primary_output = &outputs[0];
            ignition_events[1].primary_output = &outputs[1];
            ignition_events[2].primary_output = &outputs[2];
            number_of_events = 3;
            break;

        case IGNITION_MODE_COIL_ON_PLUG:
            config.ignition_mode = IGNITION_MODE_WASTED_SPARK;
            change_bit(&runtime.status, STATUS_IGNITION_ERROR, true);
            ignition_events[0].crank_angle_at_tdc = (angle_t)0;   // cyl 1 & 6
            ignition_events[1].crank_angle_at_tdc = (angle_t)120; // cyl 5 & 2
            ignition_events[2].crank_angle_at_tdc = (angle_t)240; // cyl 3 & 4

            ignition_events[0].primary_output = &outputs[0];
            ignition_events[1].primary_output = &outputs[1];
            ignition_events[2].primary_output = &outputs[2];
            number_of_events = 3;
            log_error("Coil on plug not possible for 6 cylinders, defaulting to watesd spark.");
            break;

        case IGNITION_MODE_COIL_ON_PLUG_WASTED_SPARK:
            config.ignition_mode = IGNITION_MODE_WASTED_SPARK;
            change_bit(&runtime.status, STATUS_IGNITION_ERROR, true);
            ignition_events[0].crank_angle_at_tdc = (angle_t)0;   // cyl 1 & 6
            ignition_events[1].crank_angle_at_tdc = (angle_t)120; // cyl 5 & 2
            ignition_events[2].crank_angle_at_tdc = (angle_t)240; // cyl 3 & 4

            ignition_events[0].primary_output = &outputs[0];
            ignition_events[1].primary_output = &outputs[1];
            ignition_events[2].primary_output = &outputs[2];
            number_of_events = 3;
            log_error("Coil on plug not possible for 6 cylinders, defaulting to watesd spark.");
            break;

        default:
            break;
        }
        break;

    default:
        log_error("ignition init failed. unkown firing order.");
        break;
    }
    
    if (number_of_cylinders == 0)
    {
        change_bit(&runtime.status, STATUS_IGNITION_ERROR, true);
        change_bit(&runtime.status, STATUS_CRITICAL_ERROR, true);
        return false;
    }

    runtime.firing_interval_deg = (angle_t)720 / (angle_t)number_of_cylinders;

    osTimerId_t ignition_watchdog_timer = osTimerNew(ignition_watchdog_check, osTimerPeriodic, NULL, NULL);
    if (ignition_watchdog_timer != NULL)
    {
        osTimerStart(ignition_watchdog_timer, 1); // check every 1 ms
    }
    ignition_initialized = true;

    return true;
}


void ignition_watchdog_check()
{
    for (size_t i = 0; i < number_of_events; i++)
    {
        ignition_event_t *event = &ignition_events[i];
        if (event->status == IGNITION_EVENT_DWELL)
        {
            time_us_t current_time = get_time_us();
            if ((int32_t)(current_time - event->fire_spark_time) > IGNITION_WATCHDOG_TIMER_EXTRA_TIME_MS)
            {
                ignition_coil_fire_spark((void *)event);
                change_bit(&runtime.status, STATUS_IGNITION_ERROR, true);
                log_error("ignition watchdog fired.");
            }
        }
    } 
}
void ignition_switch_to_coil_on_plug()
{
    controller_output_pin_t *outputs = &ignition_output[0];

    operating_in_forced_wasted_spark = false;
    switch (config.firing_order)
    {
    case FO_1342: // Common inline-4

        ignition_events[0].crank_angle_at_tdc = (angle_t)0;
        ignition_events[2].crank_angle_at_tdc = (angle_t)180;
        ignition_events[3].crank_angle_at_tdc = (angle_t)360;
        ignition_events[1].crank_angle_at_tdc = (angle_t)540;

        ignition_events[0].primary_output = &outputs[0];
        ignition_events[1].primary_output = &outputs[1];
        ignition_events[2].primary_output = &outputs[2];
        ignition_events[3].primary_output = &outputs[3];

        number_of_events = 4;
        number_of_cylinders = 4;

        break;

    case FO_153624: // Common inline-6

        break;

    default:
        log_error("ignition init failed. unkown firing order.");
        break;
    }
}

void ignition_switch_to_coil_on_plug_wasted_spark()
{
    controller_output_pin_t *outputs = &ignition_output[0];

    operating_in_forced_wasted_spark = true;
    switch (config.firing_order)
    {
    case FO_1342: // Common inline-4
        ignition_events[0].crank_angle_at_tdc = (angle_t)0;
        ignition_events[1].crank_angle_at_tdc = (angle_t)180;

        ignition_events[0].primary_output = &outputs[0];
        ignition_events[0].secondary_output = &outputs[3];

        ignition_events[1].primary_output = &outputs[1];
        ignition_events[1].secondary_output = &outputs[2];

        number_of_events = 2;
        number_of_cylinders = 4;
        break;

    case FO_153624: // Common inline-6

        break;

    default:
        log_error("ignition init failed. unkown firing order.");
        break;
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
    if (config.ignition_mode == IGNITION_MODE_NO_IGNITION || !ignition_initialized)
    {
        return;
    }
    if (!get_bit(runtime.status, STATUS_TRIGGER_CRANKSHAFT_SYNCED))
    {
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
    bool is_cam_phase_available = get_bit(runtime.status, STATUS_TRIGGER_CAMSHAFT_SYNCED);

    angle_t spark_advance = 0;

#ifdef TEST_MODE
    spark_advance = (angle_t)30;
#else
    spark_advance = ignition_get_advance();
#endif

    spark_advance = CLAMP(spark_advance, IGNITION_MIN_ADVANCE, IGNITION_MAX_ADVANCE);
    runtime.ignition_advance_deg = spark_advance;

    float dwell_correction = FIRMWARE_NOMINAL_VBAT / vbat_get();
    runtime.dwell_actual = CLAMP(config.ignition_dwell * dwell_correction, IGNITION_MIN_DWELL_TIME_MS, IGNITION_MAX_DWELL_TIME_MS);

    bool has_cam_phase = get_bit(runtime.status, STATUS_TRIGGER_CAMSHAFT_SYNCED);
    bool cam_phase = false;

    if (config.ignition_mode == IGNITION_MODE_COIL_ON_PLUG)
    {
        if (!has_cam_phase)
        {
            change_bit(&runtime.status, STATUS_WARNING, true);
            log_error("Cam phase not known. Switching to wasted spark.");
            ignition_switch_to_coil_on_plug_wasted_spark();
            cam_phase = false;
        }
        else
        {
            ignition_switch_to_coil_on_plug();
            cam_phase = camshaft_get_phase();
        }
    }

    __NOP();

    angle_t true_crank_angle = crankshaft_angle;
    bool is_coil_on_plug = (config.ignition_mode == IGNITION_MODE_COIL_ON_PLUG) && (!operating_in_forced_wasted_spark);

    if (is_coil_on_plug)
    {
        true_crank_angle = crankshaft_angle + (angle_t)cam_phase * (angle_t)360;
    }

    /* Now we will see which cylinder(s) can be sheduled next */
    for (size_t i = 0; i < number_of_events; i++)
    {
        ignition_event_t *event = &ignition_events[i];

        if (!(event->status == IGNITION_EVENT_INACTIVE || event->status == IGNITION_EVENT_FIRED))
        {
            continue;
        }
        /* firstly, we figure out if there are other teeth in the future */
        angle_t crank_angle_at_next_trigger = 0;
        angle_t spark_angle = 0;
        angle_t dwell_angle = 0;
        if (is_coil_on_plug)
        {
            // // we must use the 720deg math
            // angle_t crank_angle_at_next_trigger = crankshaft_get_next_trigger_angle();
            // angle_t spark_angle = wrap_angle_720(event->crank_angle_at_tdc - spark_advance);
            // angle_t dwell_angle = wrap_angle_720(spark_angle - degrees_per_millisecond(rpm) * runtime.dwell_actual);

            // if (is_phase_in_range(crank_angle_at_next_trigger, crankshaft_angle, dwell_angle))
            // {
            //     continue;
            // }
            // event->fire_spark_time = current_time_us + degree_to_microseconds(angular_forward_distance_720(true_crank_angle, spark_angle), rpm);
            // event->dwell_start_time = current_time_us + degree_to_microseconds(angular_forward_distance_720(true_crank_angle, dwell_angle), rpm);
            // schedule_ignition_event(event);
        }
        else
        {
            // we must use the 360deg math
            angle_t crank_angle_at_next_trigger = crankshaft_get_next_trigger_angle();
            angle_t spark_angle = wrap_angle_360(event->crank_angle_at_tdc - spark_advance);
            angle_t dwell_angle = wrap_angle_360(spark_angle - degrees_per_millisecond(rpm) * runtime.dwell_actual);

            if (is_phase_in_range(crank_angle_at_next_trigger, crankshaft_angle, dwell_angle))
            {
                continue;
            }
            event->fire_spark_time = current_time_us + degree_to_microseconds(angular_forward_distance_360(true_crank_angle, spark_angle), rpm);
            event->dwell_start_time = current_time_us + degree_to_microseconds(angular_forward_distance_360(true_crank_angle, dwell_angle), rpm);
            schedule_ignition_event(event);

            bool is_multi_spark_allowed = (config.multi_spark_enabled && rpm < config.multi_spark_rpm_threshold && config.ignition_mode != IGNITION_MODE_ONE_COIL && config.multi_spark_number_of_sparks > 1);
            uint8_t remaining_sparks = 0;
            remaining_sparks = is_multi_spark_allowed ? config.multi_spark_number_of_sparks - 1 : 0;
            remaining_sparks = CLAMP(remaining_sparks, 0, IGNITION_MULTI_SPARK_MAX_SPARKS - 1);
            
            angle_t degrees_taken_by_additional_sparks = 0;
            uint8_t total_number_of_sparks = 1;
            for (; remaining_sparks > 0; remaining_sparks--)
            {
                degrees_taken_by_additional_sparks += degrees_per_millisecond(rpm) * (config.multi_spark_rest_time_ms + runtime.dwell_actual);
                if (degrees_taken_by_additional_sparks > config.multi_spark_max_trailing_angle)
                {
                    break;
                }
                event->dwell_start_time = event->fire_spark_time + (time_us_t)MILLISECONDS_TO_MICROSECONDS(config.multi_spark_rest_time_ms);
                event->fire_spark_time = event->dwell_start_time + (time_us_t)MILLISECONDS_TO_MICROSECONDS(runtime.dwell_actual);
                total_number_of_sparks++;
                schedule_ignition_event(event);
            }
            runtime.multi_spark_actual_spark_count = total_number_of_sparks;
        }
    }
}

void ignition_coil_begin_charge(void *arg)
{
    if (arg == NULL)
    {
        return;
    }
    ignition_event_t *event = (ignition_event_t *)arg;

    if (event->primary_output)
    {
        HAL_GPIO_WritePin(event->primary_output->gpio, event->primary_output->pin, GPIO_PIN_SET);
        event->status = IGNITION_EVENT_DWELL;
    }
    if (event->secondary_output)
    {
        HAL_GPIO_WritePin(event->secondary_output->gpio, event->secondary_output->pin, GPIO_PIN_SET);
        event->status = IGNITION_EVENT_DWELL;
    }

}

void ignition_coil_fire_spark(void *arg)
{
    if (arg == NULL)
    {
        return;
    }
    ignition_event_t *event = (ignition_event_t *)arg;

    if (event->primary_output)
    {
        HAL_GPIO_WritePin(event->primary_output->gpio, event->primary_output->pin, GPIO_PIN_RESET);
        event->status = IGNITION_EVENT_FIRED;
    }
    if (event->secondary_output)
    {
        HAL_GPIO_WritePin(event->secondary_output->gpio, event->secondary_output->pin, GPIO_PIN_RESET);
        event->status = IGNITION_EVENT_FIRED;
    }
}

void schedule_ignition_event(ignition_event_t *event)
{
    if (event == NULL)
    {
        return;
    }
    /* We do not want to schedule anything in the past or very distant future */
    time_us_t current_time = get_time_us();

    if ((int32_t)(event->dwell_start_time - current_time) < 0 || (int32_t)(event->fire_spark_time - current_time) < 0)
    {
        return;
    }
    event->status = IGNITION_EVENT_PENDING;
    scheduler_schedule_event_with_arg(event->fire_spark_time, ignition_coil_fire_spark, (void *)event);
    scheduler_schedule_event_with_arg(event->dwell_start_time, ignition_coil_begin_charge, (void *)event);
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
    rpm_t rpm = crankshaft_get_rpm();
    pressure_t map = sensor_map_get();
    if (config.ignition_fixed_timing_enabled)
    {
        final_advance = config.ignition_fixed_timing_advance;
    }
    else if (runtime.spinning_state != SS_RUNNING)
    {
        final_advance = config.cranking_advance;
    }
    else
    {
        final_advance = table_2d_get_value(&config.ign_table_1, rpm, map); // expand and allow the use of table2 in the future
    }

    /* Here we can apply all kinds of correction to the table */

    /* Now we apply clt based advance */
    angle_t clt_correction = (angle_t)table_1d_get_value(&config.clt_based_advance_correction_table, sensor_clt_get());
    if (IS_IN_RANGE(clt_correction, IGNITION_CLT_CORRECTION_MIN_ADVANCE, IGNITION_CLT_CORRECTION_MAX_ADVANCE) && !isnan(clt_correction))
    {
        final_advance += clt_correction;
    }
    
    /* Rev limit Ignition retard */
    if (IS_IN_RANGE(rpm, (rpm_t)config.rev_limit, (rpm_t)config.rev_limit + (rpm_t)config.rev_limit_hystersis))
    {
        final_advance = mapf(rpm, (rpm_t)config.rev_limit, (rpm_t)config.rev_limit + (rpm_t)config.rev_limit_hystersis, (angle_t)final_advance, (angle_t)0);
    }

    return final_advance;
}
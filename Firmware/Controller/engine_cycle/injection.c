#include "injection.h"

injection_output_pin_conf_t *injector_pin_conf = NULL;

injection_event_t injection_events[FIRMWARE_MAX_NUMBER_OF_CYLINDERS] = {0};

static uint8_t number_of_events = 0;
static uint8_t number_of_injections = 0;
static bool injection_initialized = false;

static bool operating_in_forced_batch_injection = false;

float_time_ms_t get_pulse_width();
static void schedule_injection_event(injection_event_t *event);
static void injector_stop_injection(void *arg);
static void injector_start_injection(void *arg);
static void injection_switch_to_batch();
static void injection_switch_to_sequential();
static void injection_watchdog_check();

bool injection_init(injection_output_pin_conf_t *output_pin_conf)
{
    if (output_pin_conf == NULL)
    {
        change_bit(&runtime.status, STATUS_INJECTOR_ERROR, true);
        change_bit(&runtime.status, STATUS_CRITICAL_ERROR, true);
        log_error("injection init failed. No output config");
        return false;
    }
    controller_output_pin_t *outputs = &output_pin_conf->pin[0];
    injector_pin_conf = output_pin_conf;

    if (config.injection_mode == INJECTION_MODE_NO_FUEL_INJECTION)
        return true;

    switch (config.firing_order)
    {
    case FO_1342: // Common inline-4
        switch (config.injection_mode)
        {
        case INJECTION_MODE_SINGLE_POINT:
            injection_events[0].primary_output = &outputs[0];
            injection_events[1].primary_output = &outputs[0];

            injection_events[0].crank_angle_at_end_of_injection = (angle_t)0;
            injection_events[1].crank_angle_at_end_of_injection = (angle_t)180;
            number_of_injections = 4;
            number_of_events = 2;
            break;
        case INJECTION_MODE_BATCH:
            injection_events[0].crank_angle_at_end_of_injection = (angle_t)0;
            injection_events[1].crank_angle_at_end_of_injection = (angle_t)180;

            injection_events[0].primary_output = &outputs[0];
            injection_events[0].secondary_output = &outputs[3];

            injection_events[1].primary_output = &outputs[1];
            injection_events[1].secondary_output = &outputs[2];
            number_of_injections = 4;   
            number_of_events = 2;
            break;
        case INJECTION_MODE_SEQUENTIAL:
            injection_events[0].crank_angle_at_end_of_injection = (angle_t)360;
            injection_events[2].crank_angle_at_end_of_injection = (angle_t)540;
            injection_events[3].crank_angle_at_end_of_injection = (angle_t)0;
            injection_events[1].crank_angle_at_end_of_injection = (angle_t)180;

            injection_events[0].primary_output = &outputs[0];
            injection_events[1].primary_output = &outputs[1];
            injection_events[2].primary_output = &outputs[2];
            injection_events[3].primary_output = &outputs[3];
            number_of_injections = 4;
            number_of_events = 4;
            break;

        default:
            break;
        }
        break;
    case FO_153624: // Common inline-6
        switch (config.injection_mode)
        {
        case INJECTION_MODE_SINGLE_POINT:
            injection_events[0].crank_angle_at_end_of_injection = (angle_t)0;   // cyl 1 & 6
            injection_events[1].crank_angle_at_end_of_injection = (angle_t)120; // cyl 5 & 2
            injection_events[2].crank_angle_at_end_of_injection = (angle_t)240; // cyl 3 & 4

            injection_events[0].primary_output = &outputs[0];
            injection_events[1].primary_output = &outputs[0];
            injection_events[2].primary_output = &outputs[0];
            number_of_injections = 6;
            number_of_events = 3;
            break;

        case INJECTION_MODE_BATCH:
            injection_events[0].crank_angle_at_end_of_injection = (angle_t)0;   // cyl 1 & 6
            injection_events[1].crank_angle_at_end_of_injection = (angle_t)120; // cyl 5 & 2
            injection_events[2].crank_angle_at_end_of_injection = (angle_t)240; // cyl 3 & 4

            injection_events[0].primary_output = &outputs[0];
            injection_events[1].primary_output = &outputs[1];
            injection_events[2].primary_output = &outputs[2];
            number_of_injections = 6;
            number_of_events = 3;
            break;

        case INJECTION_MODE_SEQUENTIAL:
            config.injection_mode = INJECTION_MODE_BATCH;
            change_bit(&runtime.status, STATUS_INJECTOR_ERROR, true);
            injection_init(output_pin_conf);
            log_error("Sequential injection not possible for 6 cylinders, defaulting to batch fire.");
            return false;
            break;

        default:
            break;
        }
        break;

    default:
        log_error("injection init failed. unkown firing order.");
        break;
    }

    if (number_of_events == 0)
    {
        change_bit(&runtime.status, STATUS_IGNITION_ERROR, true);
        change_bit(&runtime.status, STATUS_CRITICAL_ERROR, true);
        return false;
    }

    osTimerId_t injection_watchdog_timer = osTimerNew(injection_watchdog_check, osTimerPeriodic, NULL, NULL);
    if (injection_watchdog_timer != NULL)
    {
        osTimerStart(injection_watchdog_timer, 1); // check every 1 ms
    }

    injection_initialized = true;

    return true;
}

void injection_watchdog_check()
{
    for (size_t i = 0; i < number_of_events; i++)
    {
        injection_event_t *event = &injection_events[i];
        if (event->status != INJECTION_EVENT_INJECTING) continue;
        time_us_t current_time = get_time_us();
        if ((int32_t)(current_time - (event->injection_stop_time)) > 0)
        {
            injector_stop_injection((void *)event);
            change_bit(&runtime.status, STATUS_INJECTOR_ERROR, true);
            log_error("injection watchdog fired.");
        }
    } 
}

void injection_switch_to_batch()
{
    controller_output_pin_t *outputs = &injector_pin_conf->pin[0];

    operating_in_forced_batch_injection = true;
    switch (config.firing_order)
    {
    case FO_1342: // Common inline-4
        injection_events[0].crank_angle_at_end_of_injection = (angle_t)0;
        injection_events[1].crank_angle_at_end_of_injection = (angle_t)180;

        injection_events[0].primary_output = &outputs[0];
        injection_events[0].secondary_output = &outputs[3];

        injection_events[1].primary_output = &outputs[1];
        injection_events[1].secondary_output = &outputs[2];

        number_of_events = 2;
        break;

    case FO_153624: // Common inline-6

        break;

    default:
        log_error("injection init failed. unkown firing order.");
        break;
    }
}

void injection_switch_to_sequential()
{
    controller_output_pin_t *outputs = &injector_pin_conf->pin[0];

    operating_in_forced_batch_injection = false;
    switch (config.firing_order)
    {
    case FO_1342: // Common inline-4
            injection_events[0].crank_angle_at_end_of_injection = (angle_t)360;
            injection_events[2].crank_angle_at_end_of_injection = (angle_t)540;
            injection_events[3].crank_angle_at_end_of_injection = (angle_t)0;
            injection_events[1].crank_angle_at_end_of_injection = (angle_t)180;

            injection_events[0].primary_output = &outputs[0];
            injection_events[1].primary_output = &outputs[1];
            injection_events[2].primary_output = &outputs[2];
            injection_events[3].primary_output = &outputs[3];

            number_of_events = 4;
        break;

    case FO_153624: // Common inline-6

        break;

    default:
        log_error("injection init failed. unkown firing order.");
        break;
    }
}

void injection_trigger_event_handle(angle_t crankshaft_angle, rpm_t rpm, time_us_t current_time_us)
{
    /**
     * @todo add the necessary checks and bounds
     */
    if (config.injection_mode == INJECTION_MODE_NO_FUEL_INJECTION || !injection_initialized)
    {
        return;
    }

    if (!get_bit(runtime.status, STATUS_TRIGGER_CRANKSHAFT_SYNCED))
    {
        return;
    }
    

    /* Wether or not cam phase is known */
    bool is_cam_phase_available = get_bit(runtime.status, STATUS_TRIGGER_CAMSHAFT_SYNCED);

    angle_t end_of_injection_advance = 0;

#ifdef TEST_MODE
    end_of_injection_advance = (angle_t)30;
#else
    end_of_injection_advance = config.injection_end_of_injection_angle;
#endif

    end_of_injection_advance = CLAMP(end_of_injection_advance, INJECTION_END_OF_INJECTION_ADVANCE_MIN, INJECTION_END_OF_INJECTION_ADVANCE_MAX);

    bool has_cam_phase = get_bit(runtime.status, STATUS_TRIGGER_CAMSHAFT_SYNCED);
    bool cam_phase = false;

    if (config.injection_mode == INJECTION_MODE_SEQUENTIAL)
    {
        if (!has_cam_phase)
        {
            change_bit(&runtime.status, STATUS_WARNING, true);
            log_error("Cam phase not known. Switching to batch injection.");
            injection_switch_to_batch();
            cam_phase = false;
        }
        else
        {
            injection_switch_to_sequential();
            cam_phase = camshaft_get_phase();
        }
    }

    __NOP();

    angle_t true_crank_angle = crankshaft_angle;
    bool is_sequential = (config.injection_mode == INJECTION_MODE_SEQUENTIAL) && (!operating_in_forced_batch_injection);

    if (is_sequential)
    {
        true_crank_angle = crankshaft_angle + (angle_t)cam_phase * (angle_t)360;
    }

    /* Now we will see which cylinder(s) can be sheduled next */
    for (size_t i = 0; i < number_of_events; i++)
    {
        injection_event_t *event = &injection_events[i];

        if (!(event->status == INJECTION_EVENT_FINISHED || event->status == INJECTION_EVENT_INACTIVE))
        {
            continue;
        }
        /* firstly, we figure out if there are other teeth in the future */
        angle_t crank_angle_at_next_trigger = 0;
        angle_t opening_angle = 0;
        angle_t closing_angle = 0;
        if (is_sequential)
        {
            // // we must use the 720deg math
            // crank_angle_at_next_trigger = crankshaft_get_next_trigger_angle();
            // closing_angle = wrap_angle_720(event->crank_angle_at_end_of_injection - end_of_injection_advance);
            // opening_angle = wrap_angle_720(closing_angle - degrees_per_millisecond(rpm) * get_pulse_width());

            // if (is_phase_in_range(crank_angle_at_next_trigger, crankshaft_angle, opening_angle))
            // {
            //     continue;
            // }
            // event->injection_stop_time = current_time_us + degree_to_microseconds(angular_forward_distance_720(true_crank_angle, closing_angle), rpm);
            // event->injection_start_time = current_time_us + degree_to_microseconds(angular_forward_distance_720(true_crank_angle, opening_angle), rpm);
            // schedule_injection_event(event);
        }
        else
        {
            // we must use the 360deg math
            crank_angle_at_next_trigger = crankshaft_get_next_trigger_angle();
            closing_angle = wrap_angle_360(event->crank_angle_at_end_of_injection - end_of_injection_advance);
            opening_angle = wrap_angle_360(closing_angle - degrees_per_millisecond(rpm) * get_pulse_width());

            if (is_phase_in_range(crank_angle_at_next_trigger, crankshaft_angle, opening_angle))
            {
                continue;
            }
            time_us_t injection_stop_time = current_time_us + degree_to_microseconds(angular_forward_distance_360(true_crank_angle, closing_angle), rpm);
            time_us_t injection_start_time = current_time_us + degree_to_microseconds(angular_forward_distance_360(true_crank_angle, opening_angle), rpm);
            // Don't schedule very small injections
            if (injection_stop_time - injection_start_time < 50)
            {
                continue;
            }
            event->injection_stop_time = injection_stop_time;
            event->injection_start_time = injection_start_time;
            schedule_injection_event(event);
        }
    }
}

float_time_ms_t get_pulse_width()
{
    mass_t petrol_fuel_mass = 0;
    petrol_fuel_mass = fuel_get_required_mass_petrol_per_cycle();

    float_time_ms_t injector_pulse_width = 0;
    injector_pulse_width = petrol_fuel_mass / INJECTION_DENSITY_OF_PETROL_GRAM_PER_CC / config.injector_flow_rate_cc_per_min * (float)60 * (float)1000;

    injector_pulse_width = injector_pulse_width / (float)number_of_injections;
    runtime.injector_pulse_width_ms = injector_pulse_width;
    return injector_pulse_width;
}

void injector_start_injection(void *arg)
{
    if (arg == NULL)
    {
        return;
    }
    injection_event_t *event = (injection_event_t *)arg;

    if (event->primary_output)
    {
        HAL_GPIO_WritePin(event->primary_output->gpio, event->primary_output->pin, GPIO_PIN_SET);
        event->status = INJECTION_EVENT_INJECTING;
    }
    if (event->secondary_output)
    {
        HAL_GPIO_WritePin(event->secondary_output->gpio, event->secondary_output->pin, GPIO_PIN_SET);
        event->status = INJECTION_EVENT_INJECTING;
    }
}

void injector_stop_injection(void *arg)
{
    if (arg == NULL)
    {
        return;
    }
    injection_event_t *event = (injection_event_t *)arg;

    if (event->primary_output)
    {
        HAL_GPIO_WritePin(event->primary_output->gpio, event->primary_output->pin, GPIO_PIN_RESET);
        event->status = INJECTION_EVENT_FINISHED;
    }
    if (event->secondary_output)
    {
        HAL_GPIO_WritePin(event->secondary_output->gpio, event->secondary_output->pin, GPIO_PIN_RESET);
        event->status = INJECTION_EVENT_FINISHED;
    }
}

void schedule_injection_event(injection_event_t *event)
{
    if (event == NULL)
    {
        return;
    }
    /* We do not want to schedule anything in the past or very distant future */
    time_us_t current_time = get_time_us();

    if ((int32_t)(event->injection_start_time - current_time) < 0 || (int32_t)(event->injection_stop_time - current_time) < 0)
    {
        return;
    }
    event->status = INJECTION_EVENT_PENDING;
    scheduler_schedule_event_with_arg(event->injection_stop_time, injector_stop_injection, (void *)event);
    scheduler_schedule_event_with_arg(event->injection_start_time, injector_start_injection, (void *)event);
}

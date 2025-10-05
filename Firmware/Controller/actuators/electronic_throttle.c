#include "electronic_throttle.h"

void electronic_throttle_tone_fail(electronic_throttle_t *etb);
void electronic_throttle_tone_success(electronic_throttle_t *etb);

void electronic_throttle_init(electronic_throttle_t *etb, pid_t *pid, sensor_tps_t *sensor, dc_motor_t *motor, table_1d_t *feed_forward_table)
{
    if (etb == NULL || sensor == NULL || motor == NULL || feed_forward_table == NULL)
    {
        log_error("Electronic throttle init failed");
        return;
    }

    etb->pid = pid;
    etb->sensor = sensor;
    etb->motor = motor;
    etb->target_position = ELECTRONIC_THROTTLE_FAIL_SAFE_POSITION;
    etb->feed_forward_table = feed_forward_table;

    etb->pid->limit_output_max = (float)255;
    etb->pid->limit_output_min = (float)-255;
    etb->pid->limit_integrator_max = (float)250;
    etb->pid->limit_integrator_min = (float)-250;

    // electronic_throttle_auto_tune(etb);

    // electronic_throttle_update(etb);

    etb->state = ETB_STATE_NORMAL;
}

void electronic_throttle_set(electronic_throttle_t *etb, percent_t position)
{
    // if (!IS_IN_RANGE(position, (percent_t)0, (percent_t)100))
    // {
    //     etb->target_position = ELECTRONIC_THROTTLE_FAIL_SAFE_POSITION;
    // }

    etb->target_position = CLAMP(position, (percent_t)0, (percent_t)100);
}

void electronic_throttle_update(void *arg)
{

    electronic_throttle_t *etb = (electronic_throttle_t *)arg;
    if (etb == NULL || etb->pid == NULL || etb->sensor == NULL || etb->motor == NULL)
    {
        log_error("Electronic throttle not initialized");
        return;
    }
    percent_t position = sensor_tps_get(etb->sensor);
    etb->current_position = position;
    pid_set_setpoint(etb->pid, etb->target_position);
    // feed-forward table value + pid control
    float feed_forward = table_1d_get_value(etb->feed_forward_table, etb->target_position);
    bool is_feed_forward_available = false;
    if (!IS_IN_RANGE(feed_forward, (float)-255, (float)255) || isnan(feed_forward))
    {
        feed_forward = 0;
        is_feed_forward_available = false;
    }
    else
    {
        is_feed_forward_available = true;
    }

    percent_t motor_effort = CLAMP(feed_forward + pid_compute(etb->pid, get_time_us(), position), (float)-255, (float)255);
    dc_motor_direction_t dir = motor_effort > 0 ? MOTOR_DIRECTION_FORWARD : MOTOR_DIRECTION_REVERSE;
    /* Limiting the end of travel duty cycle for protection */
    float margine = 5;

    if (!etb->is_duty_cycle_limiting_enabled)
    {
        dc_motor_set(etb->motor, dir, (uint8_t)ABS(motor_effort));
        return;
    }
    if (!is_feed_forward_available)
    {
        if (ABS(position - 100.0f) < ELECTRONIC_THROTTLE_NEAR_END_OF_TRAVEL_THRESHOLD)
        {
            motor_effort = CLAMP(motor_effort, (float)-255, etb->duty_cycle_limiting_upper);
        }
        else if (ABS(position - 0.0f) < ELECTRONIC_THROTTLE_NEAR_END_OF_TRAVEL_THRESHOLD)
        {
            motor_effort = CLAMP(motor_effort, (float)-1 * etb->duty_cycle_limiting_lower, (float)255);
        }
        dc_motor_set(etb->motor, dir, (uint8_t)ABS(motor_effort));
        return;
    }

    if (ABS(position - 100.0f) < ELECTRONIC_THROTTLE_NEAR_END_OF_TRAVEL_THRESHOLD)
    {
        motor_effort = CLAMP(motor_effort, (float)-255, etb->feed_forward_table->data[SIZE_OF_ARRAY(etb->feed_forward_table->data) - 1] + margine);
    }
    else if (ABS(position - 0.0f) < ELECTRONIC_THROTTLE_NEAR_END_OF_TRAVEL_THRESHOLD)
    {
        motor_effort = CLAMP(motor_effort, etb->feed_forward_table->data[0] - margine, (float)255);
    }
    dc_motor_set(etb->motor, dir, (uint8_t)ABS(motor_effort));
}

void electronic_throttle_auto_tune(electronic_throttle_t *etb)
{
    if (etb == NULL)
    {
        log_error("ETB Auto-Tune failed. ETB is null!");
        return;
    }
    osDelay(100);
    if (vbat_get() < ELECTRONIC_THROTTLE_AUTO_CALIB_MIN_VBAT)
    {
        log_error("ETB Auto-Tune failed. Vbat too low!");
        return;
    }

    float vbat_compensation = 1;
    voltage_t nominal_vbat = (voltage_t)12.6;
    vbat_compensation = nominal_vbat / vbat_get();
    /* First, we will try to find the end of travel for open and closed positions*/

    uint8_t opening_duty_cycle_limit = (float)160 * vbat_compensation; // 0 to 255
    uint8_t closing_duty_cycle_limit = (float)180 * vbat_compensation; // 0 to 255
    uint8_t duty_cycle_stride = 10;
    uint16_t settling_time_delay = 150; // time it takes for the etb to move and settle
    const uint8_t settled_state_adc_value_hystersis = 2;

    volatile uint16_t closed_adc_value = 0;
    volatile uint16_t opened_adc_value = 0;
    volatile uint16_t resting_adc_value = 0;
    volatile percent_t resting_tps_value = 0;

    // first we try to close the throttle plate and record the adc value
    const uint8_t number_of_adc_measurements = 25;

    uint16_t prev_adc_value = 0;
    uint16_t current_adc_value = 0;
    bool motor_not_moving = false;
    uint8_t sample_counter = 0;
    uint32_t sample_sum = 0;
    size_t prev_i = 0;
    for (size_t i = closing_duty_cycle_limit / 2; i < closing_duty_cycle_limit; i += duty_cycle_stride)
    {
        dc_motor_set(etb->motor, MOTOR_DIRECTION_REVERSE, i);
        osDelay(settling_time_delay);
        current_adc_value = etb->sensor->is_inverted ? (4095 - analog_inputs_get_data(etb->sensor->analog_channel)) : analog_inputs_get_data(etb->sensor->analog_channel);

        if (ABS(prev_adc_value - current_adc_value) < settled_state_adc_value_hystersis)
        {
            sample_counter++;
            sample_sum += current_adc_value;
            i = prev_i; // stay in the same duty cycle until we get enough samples
            if (sample_counter >= number_of_adc_measurements)
            {
                closed_adc_value = sample_sum / sample_counter;
                break;
            }
        }
        prev_i = i;
        prev_adc_value = current_adc_value;

        if (i >= (closing_duty_cycle_limit - duty_cycle_stride))
        {
            // means the motor is jammed or we are not trying hard enough.
            motor_not_moving = true;
        }
    }

    dc_motor_disable(etb->motor);
    osDelay(2 * settling_time_delay);

    // now the plate should be in the resting position
    resting_adc_value = etb->sensor->is_inverted ? (4095 - analog_inputs_get_data(etb->sensor->analog_channel)) : analog_inputs_get_data(etb->sensor->analog_channel);

    dc_motor_enable(etb->motor);

    // then we try to open the throttle plate and record the adc value
    prev_adc_value = 0;
    current_adc_value = 0;
    motor_not_moving = false;
    sample_counter = 0;
    sample_sum = 0;
    prev_i = 0;
    for (size_t i = opening_duty_cycle_limit / 2; i < opening_duty_cycle_limit; i += duty_cycle_stride)
    {
        dc_motor_set(etb->motor, MOTOR_DIRECTION_FORWARD, i);
        osDelay(settling_time_delay);
        current_adc_value = etb->sensor->is_inverted ? (4095 - analog_inputs_get_data(etb->sensor->analog_channel)) : analog_inputs_get_data(etb->sensor->analog_channel);

        if (ABS(prev_adc_value - current_adc_value) < settled_state_adc_value_hystersis)
        {
            sample_counter++;
            sample_sum += current_adc_value;
            i = prev_i; // stay in the same duty cycle until we get enough samples
            if (sample_counter >= number_of_adc_measurements)
            {
                opened_adc_value = sample_sum / sample_counter;
                break;
            }
        }
        prev_i = i;
        prev_adc_value = current_adc_value;

        if (i >= (opening_duty_cycle_limit - duty_cycle_stride))
        {
            // means the motor is jammed or we are not trying hard enough.
            motor_not_moving = true;
        }
    }

    dc_motor_set(etb->motor, MOTOR_DIRECTION_FORWARD, 0);
    osDelay(10 * settling_time_delay);
    percent_t resting_position = 0;
    if (motor_not_moving)
    {
        electronic_throttle_tone_fail(etb);
        return;
    }
    else
    {
        /**
         * @bug when the setpoint is at 100% or 0%, the motor will use max force!!!
         * we need to somehow limit the motor or better, we need to make the control loop not do stupid things
         * so we need to not feed it garbage, i am going to do that by taking multiple measurements!!!
         */
        // etb->sensor->closed_throttle_adc_value = closed_adc_value + (uint16_t)((float)0.05 * (float)(opened_adc_value - closed_adc_value)); // add a small offset to avoid hitting the mechanical stop
        // etb->sensor->wide_open_throttle_adc_value = opened_adc_value - (uint16_t)((float)0.05 * (float)(opened_adc_value - closed_adc_value)); // a small offset to avoid hitting the mechanical stop;
        etb->sensor->closed_throttle_adc_value = closed_adc_value;
        etb->sensor->wide_open_throttle_adc_value = opened_adc_value;
        electronic_throttle_tone_success(etb);
        resting_position = sensor_tps_get(etb->sensor);
    }

    // Settings Begin
    const float error_tolerance = (percent_t)0.5;
    const uint8_t loops = 50;

    // Settings End

    const uint8_t table_size = SIZE_OF_ARRAY(etb->feed_forward_table->data);

    for (size_t i = 0; i < table_size; i++)
    {
        if (i <= (table_size / 2))
        {
            etb->feed_forward_table->x_bins[i] = (float)i / (float)(table_size / 2) * resting_position;
        }
        else
        {
            etb->feed_forward_table->x_bins[i] = (float)(i - (table_size / 2)) / (float)(table_size / 2 - 1) * ((float)100 - resting_position) + resting_position;
        }
    }

    etb->feed_forward_table->x_bins[0] += (percent_t)0.5;
    etb->feed_forward_table->x_bins[table_size - 1] -= (percent_t)0.5;

    int32_t values_captured_per_table_cell_sum = 0;
    uint32_t values_captured_per_table_cell_count = 0;
    percent_t prev_position = 0;
    uint8_t loop_counter = 0;
    for (int32_t i = 0; i > -1 * closing_duty_cycle_limit * loops; i--)
    {
        if (loop_counter < loops)
        {
            loop_counter++;
        }
        else
        {
            dc_motor_direction_t dir = i > 0 ? MOTOR_DIRECTION_FORWARD : MOTOR_DIRECTION_REVERSE;
            dc_motor_set(etb->motor, dir, (uint8_t)ABS(i / loops));
            loop_counter = 0;
        }
        osDelay(1);
        percent_t current_position = sensor_tps_get(etb->sensor);
        size_t nearest_table_value_index = nearest_index_float(etb->feed_forward_table->x_bins, SIZE_OF_ARRAY(etb->feed_forward_table->x_bins), current_position);
        percent_t nearest_table_position = etb->feed_forward_table->x_bins[nearest_table_value_index];
        if (ABS(current_position - nearest_table_position) < error_tolerance)
        {
            if (prev_position != nearest_table_position)
            {
                prev_position = nearest_table_position;
                values_captured_per_table_cell_sum = 0;
                values_captured_per_table_cell_count = 0;
            }
            values_captured_per_table_cell_sum += (i / loops);
            values_captured_per_table_cell_count++;

            etb->feed_forward_table->data[nearest_table_value_index] = (float)(values_captured_per_table_cell_sum / (int32_t)values_captured_per_table_cell_count);
        }
    }
    values_captured_per_table_cell_sum = 0;
    values_captured_per_table_cell_count = 0;
    prev_position = 0;
    loop_counter = 0;
    for (int32_t i = 0; i < opening_duty_cycle_limit * loops; i++)
    {
        if (loop_counter < loops)
        {
            loop_counter++;
        }
        else
        {
            dc_motor_direction_t dir = i > 0 ? MOTOR_DIRECTION_FORWARD : MOTOR_DIRECTION_REVERSE;
            dc_motor_set(etb->motor, dir, (uint8_t)ABS(i / loops));
            loop_counter = 0;
        }
        osDelay(1);
        percent_t current_position = sensor_tps_get(etb->sensor);
        size_t nearest_table_value_index = nearest_index_float(etb->feed_forward_table->x_bins, SIZE_OF_ARRAY(etb->feed_forward_table->x_bins), current_position);
        percent_t nearest_table_position = etb->feed_forward_table->x_bins[nearest_table_value_index];

        if (ABS(current_position - nearest_table_position) < error_tolerance)
        {
            if (prev_position != nearest_table_position)
            {
                prev_position = nearest_table_position;
                values_captured_per_table_cell_sum = 0;
                values_captured_per_table_cell_count = 0;
            }
            values_captured_per_table_cell_sum += (i / loops);
            values_captured_per_table_cell_count++;

            if (nearest_table_value_index < (table_size / 2))
            {
                continue;
            }

            etb->feed_forward_table->data[nearest_table_value_index] = (float)(values_captured_per_table_cell_sum / (int32_t)values_captured_per_table_cell_count);
        }
    }

    dc_motor_set(etb->motor, MOTOR_DIRECTION_FORWARD, 0);
    osDelay(1000);
    if (motor_not_moving)
    {
        electronic_throttle_tone_fail(etb);
    }
    else
    {
        electronic_throttle_tone_success(etb);
    }

    osDelay(1000);
}

void electronic_throttle_tone_success(electronic_throttle_t *etb)
{
    for (size_t i = 0; i < 33; i++)
    {
        dc_motor_set(etb->motor, MOTOR_DIRECTION_FORWARD, 100);
        osDelay(3);
        dc_motor_set(etb->motor, MOTOR_DIRECTION_REVERSE, 0);
        osDelay(3);
    }
    for (size_t i = 0; i < 50; i++)
    {
        dc_motor_set(etb->motor, MOTOR_DIRECTION_FORWARD, 100);
        osDelay(2);
        dc_motor_set(etb->motor, MOTOR_DIRECTION_REVERSE, 50);
        osDelay(2);
    }
    for (size_t i = 0; i < 100; i++)
    {
        dc_motor_set(etb->motor, MOTOR_DIRECTION_FORWARD, 100);
        osDelay(1);
        dc_motor_set(etb->motor, MOTOR_DIRECTION_REVERSE, 50);
        osDelay(1);
    }
    dc_motor_set(etb->motor, MOTOR_DIRECTION_FORWARD, 0);
}

void electronic_throttle_tone_fail(electronic_throttle_t *etb)
{
    // a short “double buzz” for emphasis
    for (size_t j = 0; j < 2; j++)
    {
        for (size_t i = 0; i < 200; i++)
        {
            dc_motor_set(etb->motor, MOTOR_DIRECTION_FORWARD, 100);
            osDelay(1);
            dc_motor_set(etb->motor, MOTOR_DIRECTION_REVERSE, 50);
            osDelay(1);
        }
        osDelay(100); // pause between the two buzzes
    }

    dc_motor_set(etb->motor, MOTOR_DIRECTION_FORWARD, 0);
}

void electronic_throttle_enable_end_of_travel_protection(electronic_throttle_t *etb, percent_t duty_limit_closing, percent_t duty_limit_opening)
{
    if (etb == NULL)
    {
        return;
    }
    etb->is_duty_cycle_limiting_enabled = true;
    etb->duty_cycle_limiting_lower = mapf(duty_limit_closing, (percent_t)0, (percent_t)100, (float)0, (float)255);
    etb->duty_cycle_limiting_upper = mapf(duty_limit_opening, (percent_t)0, (percent_t)100, (float)0, (float)255);
}
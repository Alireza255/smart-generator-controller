#include "electronic_throttle.h"

void electronic_throttle_tone_fail(electronic_throttle_t *etb);
void electronic_throttle_tone_success(electronic_throttle_t *etb);

bool electronic_throttle_init(electronic_throttle_t *etb, pid_t *pid, sensor_tps_t *sensor, dc_motor_t *motor)
{
    if (etb == NULL || sensor == NULL || motor == NULL)
    {
        log_error("Electronic throttle init failed");
        if (etb->etb_number == ETB_NUMBER_1)
        {
            change_bit(&runtime.status, STATUS_ETB1_OK, false);
        }
        else if (etb->etb_number == ETB_NUMBER_2)
        {
            change_bit(&runtime.status, STATUS_ETB2_OK, false);
        }
        else
        {
            change_bit(&runtime.status, STATUS_ETB1_OK, false);
            change_bit(&runtime.status, STATUS_ETB2_OK, false);
        }
        return false;
    }

    etb->pid = pid;
    etb->sensor = sensor;
    etb->motor = motor;
    etb->target_position = ELECTRONIC_THROTTLE_FAIL_SAFE_POSITION;

    etb->pid->limit_output_max = (float)255;
    etb->pid->limit_output_min = (float)-255;
    etb->pid->limit_integrator_max = (float)250;
    etb->pid->limit_integrator_min = (float)-250;

    // electronic_throttle_auto_tune(etb);

    // electronic_throttle_update(etb);

    etb->state = ETB_STATE_NORMAL;
    if (etb->etb_number == ETB_NUMBER_1)
    {
        change_bit(&runtime.status, STATUS_ETB1_OK, true);
    }
    else if (etb->etb_number == ETB_NUMBER_2)
    {
        change_bit(&runtime.status, STATUS_ETB2_OK, true);
    }
    return true;
}

bool electronic_throttle_set(electronic_throttle_t *etb, percent_t position)
{
    if (!IS_IN_RANGE(position, (percent_t)0, (percent_t)100))
    {
        etb->target_position = ELECTRONIC_THROTTLE_FAIL_SAFE_POSITION;
        return false;
    }
    etb->target_position = position;
    return true;
}

void electronic_throttle_update(void *arg)
{
    electronic_throttle_t *etb = (electronic_throttle_t *)arg;
    if (etb == NULL || etb->pid == NULL || etb->sensor == NULL || etb->motor == NULL)
    {
        if (etb->etb_number == ETB_NUMBER_1)
        {
            change_bit(&runtime.status, STATUS_ETB1_OK, false);
        }
        else if (etb->etb_number == ETB_NUMBER_2)
        {
            change_bit(&runtime.status, STATUS_ETB2_OK, false);
        }
        else
        {
            change_bit(&runtime.status, STATUS_ETB1_OK, false);
            change_bit(&runtime.status, STATUS_ETB2_OK, false);
        }
        log_error("Electronic throttle not initialized");
        return;
    }
    percent_t position = sensor_tps_get(etb->sensor);
    etb->current_position = position;
    pid_set_setpoint(etb->pid, etb->target_position);
    percent_t motor_effort = pid_compute(etb->pid, get_time_us(), position);
    dc_motor_direction_t dir = motor_effort > 0 ? MOTOR_DIRECTION_FORWARD : MOTOR_DIRECTION_REVERSE;
    /* Limiting the end of travel duty cycle for protection */
    if (etb->is_duty_cycle_limiting_enabled)
    {
        if (ABS(position - 100.0f) < ELECTRONIC_THROTTLE_NEAR_END_OF_TRAVEL_THRESHOLD)
        {
            motor_effort = CLAMP(motor_effort, 0, etb->duty_cycle_limiting_upper);
        }
        if (ABS(position - 0.0f) < ELECTRONIC_THROTTLE_NEAR_END_OF_TRAVEL_THRESHOLD)
        {
            motor_effort = CLAMP(motor_effort, 0, etb->duty_cycle_limiting_lower);
        }
    }
    dc_motor_set(etb->motor, dir, (uint8_t)ABS(motor_effort));
    return;
}

bool electronic_throttle_auto_tune(electronic_throttle_t *etb)
{
    if (etb == NULL)
    {
        log_error("ETB Auto-Tune failed. ETB is null!");
        if (etb->etb_number == ETB_NUMBER_1)
        {
            change_bit(&runtime.status, STATUS_ETB1_OK, false);
        }
        else if (etb->etb_number == ETB_NUMBER_2)
        {
            change_bit(&runtime.status, STATUS_ETB2_OK, false);
        }
        else
        {
            change_bit(&runtime.status, STATUS_ETB1_OK, false);
            change_bit(&runtime.status, STATUS_ETB2_OK, false);
        }
        return false;
    }

    /* First, we will try to find the end of travel for open and closed positions*/
    uint8_t opening_duty_cycle = 180;    // 0 to 255
    uint8_t closing_duty_cycle = 180;    // 0 to 255
    uint16_t settling_time_delay = 1000; // time it takes for the etb to move and settle
    uint8_t number_of_samples = 10;

    uint16_t open_adc_value = 0;
    uint16_t closed_adc_value = 0;
    uint32_t temp = 0;

    dc_motor_set(etb->motor, MOTOR_DIRECTION_FORWARD, opening_duty_cycle);
    osDelay(settling_time_delay);
    temp = 0;
    for (size_t i = 0; i < number_of_samples; i++)
    {
        uint16_t adc_value = analog_inputs_get_data(etb->sensor->analog_channel);
        temp += adc_value;
    }
    open_adc_value = temp / number_of_samples;

    dc_motor_set(etb->motor, MOTOR_DIRECTION_REVERSE, 50);
    osDelay(settling_time_delay);
    dc_motor_set(etb->motor, MOTOR_DIRECTION_REVERSE, closing_duty_cycle);
    osDelay(settling_time_delay);
    temp = 0;
    for (size_t i = 0; i < number_of_samples; i++)
    {
        uint16_t adc_value = analog_inputs_get_data(etb->sensor->analog_channel);
        temp += adc_value;
    }
    closed_adc_value = temp / number_of_samples;

    etb->sensor->closed_throttle_adc_value = closed_adc_value;
    etb->sensor->wide_open_throttle_adc_value = open_adc_value;

    return true;
}
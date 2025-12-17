#include "pid.h"

void pid_init(pid_t *pid, pid_parameters_config_t *config)
{
    if (pid == NULL || config == NULL)
    {
        return;
    }
    
    /* Clearing controller variables */
    pid->integrator = 0.0f;
    pid->differentiator = 0.0f;
    pid->prev_error = 0.0f;
    pid->prev_input = 0.0f;
    pid->prev_controller_time_us = 0;
    pid->output = 0.0f;
    pid->config = config;
    /**
     * @todo implement checks for the parameters
     */
    
}

void pid_set_setpoint(pid_t *pid, float setpoint)
{
    pid->setpoint = setpoint;
}

float pid_get_error(pid_t *pid)
{
    return pid->prev_error;
}

float pid_compute(pid_t *pid, time_us_t current_time_us, float input)
{
    if (pid->config == NULL)
    {
        return 0;
    }
    
    /* Compute the time between the function calls to determine T */
    time_us_t T_us = current_time_us - pid->prev_controller_time_us;
    float T_sec = (float)T_us / (float)1e6f;

    /* Finding the error */
    float error = pid->setpoint - input;

    /* Finding the P-term */
    float proportional = pid->config->Kp * error;

    /* Finding the integral term*/
    pid->integrator = pid->integrator + 0.5f * pid->config->Ki * T_sec *(error + pid->prev_error);
    
    /* Anti-windup - works by clamping the integrator */
    if (pid->integrator > pid->config->limit_integrator_max)
    {
        pid->integrator = pid->config->limit_integrator_max;
    }
    else if (pid->integrator < pid->config->limit_integrator_min)
    {
        pid->integrator = pid->config->limit_integrator_min;
    }

    /* Finding the derivative term @note has a low pass filter */
    /**
     * @note only derivative on input has been implemented for now
     */
    pid->differentiator = -(2.0f * pid->config->Kd * (input - pid->prev_input)
                          +(2.0f * pid->config->derivative_filter_tau - T_sec) * pid->differentiator)
                          / (2.0f * pid->config->derivative_filter_tau + T_sec);

    /* Summing the terms to make the output */
    pid->output = proportional + pid->integrator + pid->differentiator;

    /* Clamp the output the the output bounds */
    if (pid->output > pid->limit_output_max)
    {
        pid->output = pid->limit_output_max;
    }
    else if (pid->output < pid->limit_output_min)
    {
        pid->output = pid->limit_output_min;
    }

    /* Save the variables for the next function call */
    pid->prev_error = error;
    pid->prev_input = input;
    pid->prev_controller_time_us = current_time_us;
    
    return pid->output;
}

#include "pid.h"

void pid_init(pid_t *pid)
{
    /* Clearing controller variables */
    pid->integrator = 0.0f;
    pid->differentiator = 0.0f;
    pid->prev_error = 0.0f;
    pid->prev_input = 0.0f;

    pid->prev_controller_time_us = 0;
    pid->output = 0.0f;

    /**
     * @todo implement checks for the parameters
     */
    
}

void pid_set_setpoint(pid_t *pid, float setpoint)
{
    pid->setpoint = setpoint;
}

float pid_compute(pid_t *pid, time_us_t current_time_us, float input)
{
    /* Compute the time between the function calls to determine T */
    time_us_t T_us = current_time_us - pid->prev_controller_time_us;
    float T_sec = (float)T_us / (float)1e6f;

    /* Finding the error */
    float error = pid->setpoint - input;

    /* Finding the P-term */
    float proportional = pid->Kp * error;

    /* Finding the integral term*/
    pid->integrator = pid->integrator + 0.5f * pid->Ki * T_sec *(error + pid->prev_error);

    /* Anti-windup - works by clamping the integrator */
    if (pid->integrator > pid->limit_integrator_max)
    {
        pid->integrator = pid->limit_integrator_max;
    }
    else if (pid->integrator < pid->limit_integrator_min)
    {
        pid->integrator = pid->limit_integrator_min;
    }

    /* Finding the derivative term @note has a low pass filter */
    /**
     * @note only derivative on input has been implemented for now
     */
    pid->differentiator = -(2.0f * pid->Kd * (input - pid->prev_input)
                          +(2.0f * pid->derivative_filter_tau - T_sec) * pid->differentiator)
                          / (2.0f * pid->derivative_filter_tau + T_sec);

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

void pid_set_tuning(pid_t *pid, pid_configuration_t *conf)
{
    if (pid == NULL)
    {
        return;
    }
    /**
     * @todo provide propper range checks in the furture
     */
    pid->Kp = conf->Kp;
    pid->Ki = conf->Ki;
    pid->Kd = conf->Kd;
    pid->limit_integrator_min = conf->limit_integrator_min;
    pid->limit_integrator_max = conf->limit_integrator_max;
    pid->derivative_filter_tau = conf->derivative_filter_tau;
}
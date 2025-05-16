/**
 * @file pid.h
 * @brief Contains a complete pid controller with all the bells and whistles
 * @author Alireza Eskandari
 */

#ifndef PID_H
#define PID_H

#include "controller_time.h"


typedef struct
{
    /* Controller gains */
    float Kp;
    float Ki;
    float Kd;


    /* Derivative low pass filter time constant */
    float derivative_filter_tau;

    /* Output bounds */
    float limit_output_min;
    float limit_output_max;

    /* Integrator limits */
    float limit_integrator_min;
    float limit_integrator_max;

    /* Sample time in seconds */
    /**
     * @note might not implement this and compute the sample time instead!
     */
    float T;

    /* Controller memory - internal to the controller */
    float integrator;
    float differentiator;
    float prev_error;
    float prev_input;
    time_us_t prev_controller_time_us;

    /* Controller setpoint */
    float setpoint;
    /* Controller output */
    float output;

} pid_t;

void pid_init(pid_t *pid);

void pid_set_setpoint(pid_t *pid, float setpoint);

float pid_compute(pid_t *pid, time_us_t current_time_us, float input);

#endif // PID_H
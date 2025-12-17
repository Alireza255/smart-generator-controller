/**
 * @file pid.h
 * @brief Contains a complete pid controller with all the bells and whistles
 * @author Alireza Eskandari
 */

#ifndef PID_H
#define PID_H

#include <math.h>
#include "timing.h"
#include "types.h"

typedef struct
{
    /* Controller gains and parameters */
    pid_parameters_config_t *config;

    /* Output bounds */
    float limit_output_min;
    float limit_output_max;

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

void pid_init(pid_t *pid, pid_parameters_config_t *config);

void pid_set_setpoint(pid_t *pid, float setpoint);

float pid_get_error(pid_t *pid);

float pid_compute(pid_t *pid, time_us_t timestamp, float input);

#endif // PID_H
/**
 * @file electronic_throttle.h
 * @author Alireza Eskandari
 * @brief a library that uses the dc motor library and pid to run ETBs
 */

#ifndef ELECTRONIC_THROTTLE_H
#define ELECTRONIC_THROTTLE_H

#include "cmsis_os2.h"
#include "dc_motors.h"
#include "pid.h"
#include "sensors.h"
#include "utils.h"
#include "error_handling.h"
#include "math.h"
#include "trigger.h"

#define ELECTRONIC_THROTTLE_FAIL_SAFE_POSITION (percent_t)0
#define ELECTRONIC_THROTTLE_NEAR_END_OF_TRAVEL_THRESHOLD (percent_t)0.8f
#define ELECTRONIC_THROTTLE_AUTO_CALIB_MIN_VBAT (voltage_t)11

typedef struct
{
    status_t status_flag; // true when ok
    dc_motor_t *motor;
    pid_t *pid;
    sensor_tps_t *sensor;
    table_1d_t *feed_forward_table;
    percent_t target_position;
    percent_t current_position;
    percent_t duty_cycle_limiting_lower;
    percent_t duty_cycle_limiting_upper;
    bool is_duty_cycle_limiting_enabled;
    bool control_loop_give_up_control;
} electronic_throttle_t;

/**
 * @note sensor and motor have to be initialized before calling this function
 */
void electronic_throttle_init(electronic_throttle_t *etb, pid_t *pid, sensor_tps_t *sensor, dc_motor_t *motor, table_1d_t *feed_forward_table, status_t status_flag);

void electronic_throttle_auto_tune(electronic_throttle_t *etb);

void electronic_throttle_set(electronic_throttle_t *etb, percent_t position);

void electronic_throttle_disable(electronic_throttle_t *etb);
void electronic_throttle_enable(electronic_throttle_t *etb);


void electronic_throttle_update(void *arg);

/**
 * @brief Limits the duty cycle in order to not burn out the motor in case the control loop asks the motor to something stupid.
 * 
 * @param etb pointer to the etb of choice
 * @param duty_limit_opening Duty cycle limit of the motor when protection is active (percent)
 * @param duty_limit_closing Duty cycle limit of the motor when protection is active (percent)
 */
void electronic_throttle_enable_end_of_travel_protection(electronic_throttle_t *etb, percent_t duty_limit_closing, percent_t duty_limit_opening);

void electronic_throttle_tone_fail(electronic_throttle_t *etb);
void electronic_throttle_tone_success(electronic_throttle_t *etb);


#endif // ELECTRONIC_THROTTLE_H
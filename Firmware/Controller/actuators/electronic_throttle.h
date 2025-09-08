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

#define ELECTRONIC_THROTTLE_FAIL_SAFE_POSITION (percent_t)0
#define ELECTRONIC_THROTTLE_NEAR_END_OF_TRAVEL_THRESHOLD (percent_t)0.5f

typedef enum
{
    ETB_STATE_NO_INIT,
    ETB_STATE_NORMAL,
    ETB_STATE_ERROR,
} electronic_throttle_state_t;

typedef enum
{
    ETB_NUMBER_1,
    ETB_NUMBER_2,
} electronic_throttle_number_t;

typedef struct
{
    electronic_throttle_state_t state;
    dc_motor_t *motor;
    pid_t *pid;
    sensor_tps_t *sensor;
    percent_t target_position;
    percent_t current_position;
    percent_t duty_cycle_limiting_lower;
    percent_t duty_cycle_limiting_upper;
    electronic_throttle_number_t etb_number;
    bool is_duty_cycle_limiting_enabled;
} electronic_throttle_t;

/**
 * @note sensor and motor have to be initialized before calling this function
 */
bool electronic_throttle_init(electronic_throttle_t *etb, pid_t *pid, sensor_tps_t *sensor, dc_motor_t *motor);

bool electronic_throttle_auto_tune(electronic_throttle_t *etb);

bool electronic_throttle_set(electronic_throttle_t *etb, percent_t position);

void electronic_throttle_update(void *arg);
 
#endif // ELECTRONIC_THROTTLE_H
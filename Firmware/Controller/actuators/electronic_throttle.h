/**
 * @file electronic_throttle.h
 * @author Alireza Eskandari
 * @brief a library that uses the dc motor library and pid to run ETBs
 */

#ifndef ELECTRONIC_THROTTLE_H
#define ELECTRONIC_THROTTLE_H

#include "dc_motors.h"
#include "pid.h"
#include "tps.h"
#include "utils.h"
#include "error_handling.h"
#include "math.h"

#define ELECTRONIC_THROTTLE_FAIL_SAFE_POSITION (percent_t)0

typedef enum
{
    ETB_STATE_NO_INIT,
    ETB_STATE_NORMAL,
    ETB_STATE_ERROR,
} electronic_throttle_state_e;

typedef struct
{
    electronic_throttle_state_e state;
    dc_motor_s *motor;
    pid_t *pid;
    sensor_tps_s *sensor;
    percent_t target_position;
} electronic_throttle_s;

/**
 * @note sensor and motor have to be initialized before calling this function
 */
void electronic_throttle_init(electronic_throttle_s *etb, pid_t *pid, sensor_tps_s *sensor, dc_motor_s *motor);

void electronic_throttle_set(electronic_throttle_s *etb, percent_t position);

void electronic_throttle_update(electronic_throttle_s *etb);

#endif // ELECTRONIC_THROTTLE_H
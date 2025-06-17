#include "electronic_throttle.h"

void electronic_throttle_init(electronic_throttle_t *etb, pid_t *pid, sensor_tps_t *sensor, dc_motor_t *motor)
{
    if (etb == NULL || sensor == NULL || motor == NULL)
    {
        log_error("Electronic throttle init failed");
        return;
    }
    
    etb->pid = pid;
    etb->sensor = sensor;
    etb->motor = motor;
    etb->target_position = ELECTRONIC_THROTTLE_FAIL_SAFE_POSITION;

    etb->pid->limit_output_max = (float)255;
    etb->pid->limit_output_min = (float)-255;

    //electronic_throttle_update(etb);
    osTimerId_t timer_id = osTimerNew(
        electronic_throttle_update,
        osTimerPeriodic,
        etb,
        NULL
    );
    osTimerStart(timer_id, 1); // every 1 tick = every 1ms = 1000Hz
    etb->state = ETB_STATE_NORMAL;

}

void electronic_throttle_set(electronic_throttle_t *etb, percent_t position)
{
    if (!IS_IN_RANGE(position, (percent_t)0, (percent_t)100))
    {
        etb->target_position = ELECTRONIC_THROTTLE_FAIL_SAFE_POSITION;
    }
    etb->target_position = position;
}

void electronic_throttle_update(void *arg)
{
    electronic_throttle_t *etb = (electronic_throttle_t *)arg;
    if (etb == NULL || etb->pid == NULL || etb->sensor == NULL || etb->motor == NULL)
    {
        log_error("Electronic throttle not initialized");
        return;
    }
    percent_t postion = sensor_tps_get(etb->sensor);
    pid_set_setpoint(etb->pid, etb->target_position);
    percent_t motor_effort = pid_compute(etb->pid, get_time_us(), postion);
    dc_motor_direction_t dir = motor_effort > 0 ? MOTOR_DIRECTION_FORWARD : MOTOR_DIRECTION_REVERSE;
    dc_motor_set(etb->motor, dir ,  (uint8_t)ABS(motor_effort));
}



#include "electronic_throttle.h"

void electronic_throttle_init(electronic_throttle_s *etb, pid_t *pid, sensor_tps_s *sensor, dc_motor_s *motor)
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

}

void electronic_throttle_set(electronic_throttle_s *etb, percent_t position)
{
    if (!IS_IN_RANGE(position, (percent_t)0, (percent_t)100))
    {
        etb->target_position = ELECTRONIC_THROTTLE_FAIL_SAFE_POSITION;
    }
    etb->target_position = position;
}

void electronic_throttle_update(electronic_throttle_s *etb)
{
    if (etb == NULL || etb->pid == NULL || etb->sensor == NULL || etb->motor || NULL)
    {
        log_error("Electronic throttle not initialized");
        return;
    }
    percent_t postion = sensor_tps_get(etb->sensor);
    percent_t motor_effort = pid_compute(etb->pid, get_time_us(), postion);
    dc_motor_direction_e dir = motor_effort > 0 ? MD_FORWARD : MD_REVERSE;
    dc_motor_set(etb->motor, dir ,  ABS((uint8_t)motor_effort));
}



#include "governer.h"
#include "controller.h"

electronic_throttle_t *governer_etb = NULL;
static pid_t governer_pid = {0};

void governer_init(electronic_throttle_t *etb, pid_parameters_config_t *config)
{
    if (etb == NULL || config == NULL)
    {
        log_error("Governer init failed!");
        return;
    }
    governer_etb = etb;

    pid_init(&governer_pid, config);
    governer_pid.limit_output_max = (percent_t)100;
    governer_pid.limit_output_min = (percent_t)0;

    /**
     * Can this lead to windup? imagine the motor is running at a low rpm (say 800rpm) so the rpm gets updated at a rate of
     * 800Hz, this means that sometimes, the pid algorithm runs over old data which can lead to windup since it doesn't see
     * a change in the plant when it has already commanded something.
     * I suspect this is a very small problem and nothing that could cause any issues.
     * Alireza Eskandari
     * @date 8/6/2025
     */
}

void governer_update(time_us_t timestamp)
{
    if (governer_etb == NULL)
    {
        return;
    }
    
    percent_t throttle_setpoint = ELECTRONIC_THROTTLE_FAIL_SAFE_POSITION;

    if (runtime.spinning_state != SS_RUNNING)
    {
        throttle_setpoint = config.cranking_throttle;
        runtime.governer_status = GOVERNER_STATUS_IGNORED;
        runtime.governer_target_rpm = config.governer_idle_rpm;
    }
    else
    {
        runtime.governer_status = GOVERNER_STATUS_TARGET;
        pid_set_setpoint(&governer_pid, config.governer_target_rpm);
        rpm_t rpm = crankshaft_get_rpm();
        throttle_setpoint = pid_compute(&governer_pid, timestamp, rpm);
        runtime.governer_target_rpm = config.governer_target_rpm;
    }
    electronic_throttle_set(governer_etb, throttle_setpoint);
    
}
governer_status_t governer_get_status()
{
    return runtime.governer_status;
}

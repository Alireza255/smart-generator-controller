#include "governer.h"
#include "controller.h"

electronic_throttle_t *governer_etb = NULL;
static pid_t governer_pid;

void governer_init(electronic_throttle_t *etb)
{
    if (etb == NULL)
    {
        log_error("Governer init failed!");
        return;
    }
    governer_etb = etb;

    pid_init(&governer_pid);
    governer_pid.limit_output_max = (percent_t)100;
    governer_pid.limit_output_min = (percent_t)0;
    pid_configuration_t dummy_config = {0};

    governer_pid.limit_output_min = (percent_t)0;
    governer_pid.limit_output_max = (percent_t)100;

    governer_pid.Kp = config.governer_pid_Kp;
    governer_pid.Ki = config.governer_pid_Ki;
    governer_pid.Kd = config.governer_pid_Kd;
    governer_pid.limit_integrator_min = config.governer_pid_limit_integrator_min;
    governer_pid.limit_integrator_max = config.governer_pid_limit_integrator_max;
    governer_pid.derivative_filter_tau = config.governer_pid_derivative_filter_tau;

    pid_set_tuning(&governer_pid, &dummy_config);

    osTimerId_t timer = osTimerNew(governer_update, osTimerPeriodic, NULL, NULL);
    osTimerStart(timer, 1);
    /**
     * Can this lead to windup? imagine the motor is running at a low rpm (say 800rpm) so the rpm gets updated at a rate of
     * 800Hz, this means that sometimes, the pid algorithm runs over old data which can lead to windup since it doesn't see
     * a change in the plant when it has already commanded something.
     * I suspect this is a very small problem and nothing that could cause any issues.
     * Alireza Eskandari
     * @date 8/6/2025
     */
}

void governer_update()
{
    if (governer_etb == NULL)
    {
        log_error("Governer no init!");
        return;
    }
    percent_t throttle_setpoint = ELECTRONIC_THROTTLE_FAIL_SAFE_POSITION;

    governer_pid.Kp = config.governer_pid_Kp;
    governer_pid.Ki = config.governer_pid_Ki;
    governer_pid.Kd = config.governer_pid_Kd;
    governer_pid.limit_integrator_min = config.governer_pid_limit_integrator_min;
    governer_pid.limit_integrator_max = config.governer_pid_limit_integrator_max;
    governer_pid.derivative_filter_tau = config.governer_pid_derivative_filter_tau;

    if (runtime.spinning_state != SS_RUNNING)
    {
        throttle_setpoint = config.cranking_throttle;
        runtime.governer_status = GOVERNER_STATUS_IGNORED;
        electronic_throttle_set(governer_etb, throttle_setpoint);
    }
    else
    {
        
        runtime.governer_status = GOVERNER_STATUS_TARGET;
        pid_set_setpoint(&governer_pid, config.governer_target_rpm);
        rpm_t rpm = crankshaft_get_rpm();
        throttle_setpoint = pid_compute(&governer_pid, get_time_us(), rpm);
        electronic_throttle_set(governer_etb, throttle_setpoint);
    }
}
governer_status_e governer_get_status()
{
    return runtime.governer_status;
}

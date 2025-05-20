#include "governer.h"
#include "controller.h"

electronic_throttle_s *governer_etb = NULL;
static pid_t governer_pid;

void governer_init(electronic_throttle_s *etb)
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
    pid_set_tuning(&governer_pid, &configuration.governer_pid);

    osTimerId_t timer = osTimerNew(governer_update, osTimerPeriodic, NULL, NULL);
    osTimerStart(timer, 1);
    
}

void governer_update()
{
    if (governer_etb == NULL)
    {
        log_error("Governer no init!");
        return;
    }
    percent_t throttle_setpoint = ELECTRONIC_THROTTLE_FAIL_SAFE_POSITION;

    if (engine.spinning_state != SS_RUNNING)
    {
        throttle_setpoint = configuration.cranking_throttle;
        engine.governer_status = GOVERNER_STATUS_IGNORED;
        electronic_throttle_set(governer_etb, throttle_setpoint);
    }
    else
    {
        engine.governer_status = GOVERNER_STATUS_TARGET;
        pid_set_setpoint(&governer_pid, configuration.governer_target_rpm);
        rpm_t rpm = crankshaft_get_rpm();
        throttle_setpoint = pid_compute(&governer_pid, get_time_us(), rpm);
        electronic_throttle_set(governer_etb, throttle_setpoint);
    }
}
governer_status_e governer_get_status()
{
    return engine.governer_status;
}


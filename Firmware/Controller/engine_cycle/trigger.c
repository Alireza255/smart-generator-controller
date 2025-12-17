#include "trigger.h"
#include "controller.h"

static angle_t next_tooth_angle = 0;

static trigger_crankshaft_t crank_trigger = {0};
static trigger_camshaft_t cam_trigger = {0};


void trigger_crankshaft_init(trigger_wheel_type_crankshaft_t wheel_type)
{
  crank_trigger.initialized = false;
  switch (wheel_type)
  {
  case TRIGGER_WHEEL_TYPE_CRANK_58_TOOTH_2_MISSING:
    crank_trigger.missing_teeth = 2;
    crank_trigger.full_teeth = 60;
    break;

  case TRIGGER_WHEEL_TYPE_CRANK_29_TOOTH_1_MISSING:
    crank_trigger.missing_teeth = 1;
    crank_trigger.full_teeth = 29;
    break;

  default:

    log_error("Trigger initialization failed. Wheel type not recognized. Defaulting to 58-2.");
    // Default to 58-2
    config.trigger_crank_type = TRIGGER_WHEEL_TYPE_CRANK_58_TOOTH_2_MISSING;
    trigger_crankshaft_init(config.trigger_crank_type);
    return;
    break;
  }
  crank_trigger.trigger_actual_teeth = crank_trigger.full_teeth - crank_trigger.missing_teeth;
  crank_trigger.filter_time = 100;
  change_bit(&runtime.status, STATUS_TRIGGER_CRANKSHAFT_SYNCED, false);
  change_bit(&runtime.status, STATUS_TRIGGER_ERROR, false);

  crank_trigger.initialized = true;
}

void trigger_camshaft_init(trigger_wheel_type_camshaft_t wheel_type, uint8_t *filtering)
{
  cam_trigger.initialized = false;

  if (filtering == NULL)
  {
    change_bit(&runtime.status, STATUS_TRIGGER_ERROR, true);
    change_bit(&runtime.status, STATUS_TRIGGER_CAMSHAFT_SYNCED, false);
    return;
  }

  switch (wheel_type)
  {
  case TRIGGER_WHEEL_TYPE_CAM_HALF_CIRCLE:

    break;

  default:
    break;
  }

  cam_trigger.initialized = true;
  cam_trigger.filtering = filtering;
}

void trigger_crank_handle(trigger_event_t *event)
{

  if (!crank_trigger.initialized)
  {
    log_error("Crank trigger is not initialized.");
    change_bit(&runtime.status, STATUS_CRITICAL_ERROR, true);
    change_bit(&runtime.status, STATUS_TRIGGER_ERROR, true);
    return;
  }
  if (event->type == TRIGGER_EVENT_TYPE_TIMEOUT)
  {
    // No tooth arrived in time → signal lost
    runtime.rpm = 0;

    change_bit(&runtime.status, STATUS_TRIGGER_CRANKSHAFT_SYNCED, false);

    runtime.spinning_state = SS_STOPPED;
    change_bit(&runtime.status, STATUS_CRANKING, false);
    change_bit(&runtime.status, STATUS_RUNNING, false);
    return;
  }

  time_us_t current_tooth_time = event->timestamp;
  time_us_t current_tooth_gap = current_tooth_time - crank_trigger.tooth_time_history[0];
  time_us_t prev_tooth_gap = crank_trigger.tooth_time_history[0] - crank_trigger.tooth_time_history[1];

  crank_trigger.tooth_time_history[1] = crank_trigger.tooth_time_history[0];
  crank_trigger.tooth_time_history[0] = current_tooth_time;

  /* Debounce: ignore too-close pulses */
  if (current_tooth_gap < crank_trigger.filter_time)
  {
    return; // noise
  }

  if (prev_tooth_gap <= 0)
  {
    return; // Don't devide by 0 okay? :)   Also also for a few more events to fill the history so prev_tooth_gap will be correct
  }

  bool is_sync_event = false;
  float gap_ratio = (float)current_tooth_gap / (float)prev_tooth_gap;
  if (IS_IN_RANGE(gap_ratio, (float)TRIGGER_TOOTH_GAP_SYNC_RATIO_LOWER, (float)TRIGGER_TOOTH_GAP_SYNC_RATIO_UPPER * crank_trigger.missing_teeth))
  {
    is_sync_event = true;
  }

  static bool is_trigger_sync_achieved = false;

  if (is_sync_event)
  {
    if (crank_trigger.current_tooth_index == (crank_trigger.trigger_actual_teeth - 1))
    {
      /**
       * If were are at a sync event and we have counted the currect number of teeth so far,
       * we have correctly determined the angle of and rpm at each trigger event in the last
       * rotation.
       */
      is_trigger_sync_achieved = true;
      runtime.total_revolutions++;
    }
    else
    {
      /**
       * A sync loss has occored because we counted the wrong number of teeth before the wheel
       * rotated once!
       */
      runtime.sync_loss_counter++;
      is_trigger_sync_achieved = false;
      // We will reset the teeth we have counted so far to hopefully achieve sync on the next rotation
      runtime.total_revolutions = 0;
    }
    crank_trigger.current_tooth_index = 0;
  }
  else
  {
    crank_trigger.current_tooth_index++;
  }

  change_bit(&runtime.status, STATUS_TRIGGER_CRANKSHAFT_SYNCED, is_trigger_sync_achieved);

  runtime.crankshaft_angle = wrap_angle_360(360.0f / (angle_t)crank_trigger.full_teeth * (angle_t)crank_trigger.current_tooth_index + (angle_t)config.trigger_offset_deg);

  if (!is_sync_event)
  {
    runtime.rpm = (rpm_t)((float)CONVERSION_FACTOR_SECONDS_TO_MICROSECONDS * (float)CONVERSION_FACTOR_MINUTES_TO_SECONDS / (float)current_tooth_gap / (float)crank_trigger.full_teeth);
  }

  if (crank_trigger.current_tooth_index >= crank_trigger.trigger_actual_teeth - 1)
  {
    next_tooth_angle = (angle_t)wrap_angle_360(config.trigger_offset_deg);
  }
  else
  {
    next_tooth_angle = wrap_angle_360(runtime.crankshaft_angle + 360.0f / (angle_t)crank_trigger.full_teeth);
  }

  switch (runtime.spinning_state)
  {
  case SS_STOPPED:
    if (!is_trigger_sync_achieved && runtime.rpm != (rpm_t)0)
    {
      runtime.spinning_state = SS_SPINNING_UP;
    }

    break;

  case SS_SPINNING_UP:
    if (!is_trigger_sync_achieved && runtime.rpm == (rpm_t)0)
    {
      runtime.spinning_state = SS_STOPPED;
    }
    else if (is_trigger_sync_achieved && runtime.rpm != (rpm_t)0)
    {
      runtime.spinning_state = SS_CRANKING;
    }

    break;

  case SS_CRANKING:
    if (!is_trigger_sync_achieved && runtime.rpm < (config.cranking_rpm_threshold - TRIGGER_SPINNING_STATE_RPM_HYSTEERSIS))
    {
      runtime.spinning_state = SS_SPINNING_UP;
    }
    if (is_trigger_sync_achieved && runtime.rpm > (config.cranking_rpm_threshold + TRIGGER_SPINNING_STATE_RPM_HYSTEERSIS))
    {
      runtime.spinning_state = SS_RUNNING;
    }

    break;

  case SS_RUNNING:
    if (runtime.rpm < (config.cranking_rpm_threshold - TRIGGER_SPINNING_STATE_RPM_HYSTEERSIS) && is_trigger_sync_achieved)
    {
      runtime.spinning_state = SS_CRANKING;
    }

    break;

  default:
    runtime.spinning_state = SS_STOPPED;
    break;
  }

  change_bit(&runtime.status, STATUS_CRANKING, runtime.spinning_state == SS_CRANKING);
  change_bit(&runtime.status, STATUS_RUNNING, runtime.spinning_state == SS_RUNNING);

  
}

void trigger_camshaft_signal_handle(bool is_rising_edge)
{
  if (!cam_trigger.initialized)
  {
    return;
  }
  if (!config.trigger_cam_enabled)
  {
    return;
  }

  time_us_t filter_time = 0;
  float on_to_off_ratio_allowed_jitter = 0;
  switch (*cam_trigger.filtering)
  {
  case TRIGGER_FILTERING_NONE:
    filter_time = 0;
    on_to_off_ratio_allowed_jitter = 0;
    break;
  case TRIGGER_FILTERING_LITE:
    on_to_off_ratio_allowed_jitter = (float)0.25;
    filter_time = 100;
    break;
  case TRIGGER_FILTERING_MEDIUM:
    on_to_off_ratio_allowed_jitter = (float)0.125;
    filter_time = 250;
    break;
  case TRIGGER_FILTERING_AGGRESSIVE:
    on_to_off_ratio_allowed_jitter = (float)0.0625;
    filter_time = 500;
    break;

  default:
    *cam_trigger.filtering = TRIGGER_FILTERING_NONE;
    on_to_off_ratio_allowed_jitter = (float)0;
    break;
  }

  time_us_t current_time = get_time_us();

  /* Debounce: ignore too-close pulses */
  if (cam_trigger.last_edge_time != 0 &&
      (current_time - cam_trigger.last_edge_time) < filter_time)
  {
    change_bit(&runtime.status, STATUS_TRIGGER_CAMSHAFT_SYNCED, false);
    return; // noise
  }

  time_us_t current_edge_gap = current_time - cam_trigger.last_edge_time;

  float on_off_ratio = (float)current_edge_gap / (float)cam_trigger.last_edge_gap;

  if (on_to_off_ratio_allowed_jitter != 0 && !IS_IN_RANGE(on_off_ratio, (float)1 - on_to_off_ratio_allowed_jitter, (float)1 + on_to_off_ratio_allowed_jitter))
  {
    change_bit(&runtime.status, STATUS_TRIGGER_CAMSHAFT_SYNCED, false);
    return;
  }

  change_bit(&runtime.status, STATUS_TRIGGER_CAMSHAFT_SYNCED, true);

  if (is_rising_edge)
  {
    /* Rising edge */
    cam_trigger.phase = true;
  }
  else
  {
    /* Falling edge */
    cam_trigger.phase = false;
  }

  cam_trigger.last_edge_gap = current_edge_gap;
  cam_trigger.last_edge_time = current_time;
}

bool camshaft_get_phase()
{
  return cam_trigger.phase;
}
rpm_t crankshaft_get_rpm()
{
  return runtime.rpm;
}

/**
 * @brief returns the instant angle of the crankshaft
 * @note angle is updated on every trigger event!
 */
angle_t crankshaft_get_angle()
{
  return runtime.crankshaft_angle;
}

angle_t crankshaft_get_next_trigger_angle()
{
  return next_tooth_angle;
}

__weak void trigger_driven_events_callback()
{
  /* This is a weak function. User can override it in their code */
}

spinning_state_t trigger_spinning_state_get()
{
  return (spinning_state_t)runtime.spinning_state;
}
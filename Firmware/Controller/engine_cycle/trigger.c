#include "trigger.h"
#include "controller.h"



void set_filtering(trigger_t *trigger)
{
  switch (*(trigger->filtering))
  {
  case TRIGGER_FILTERING_NONE:
    trigger->_trigger_filter_time_us = 0;
    break;
  case TRIGGER_FILTERING_LITE:
    trigger->_trigger_filter_time_us = trigger->_current_tooth_gap_us / 4U;
    break;
  case TRIGGER_FILTERING_MEDIUM:
    trigger->_trigger_filter_time_us = trigger->_current_tooth_gap_us / 1U;
    break;
  case TRIGGER_FILTERING_AGGRESSIVE:
    trigger->_trigger_filter_time_us = trigger->_current_tooth_gap_us * 3 / 4;
    break;
  default:
    // Handle invalid filtering level
    trigger->_trigger_filter_time_us = 0;
    break;
  }
}

void trigger_init(trigger_t *trigger, trigger_wheel_type_t wheel_type, uint8_t *filtering, status_t sync_status_bit, uint8_t trigger_number)
{
  bool temp_status = true;
  if (trigger == NULL)
  {
    change_bit(&runtime.status, STATUS_TRIGGER_ERROR, true);
    log_error("Trigger initialization failed. Trigger is NULL.");
    return;
  }
  trigger->filtering = filtering;
  trigger->status_synced_index = sync_status_bit;

  switch (wheel_type)
  {
  case TW_58_TOOTH_2_MISSING:
    trigger->_missing_teeth = 2;
    trigger->_full_teeth = 60;
    break;

  case TW_29_TOOTH_1_MISSING:
    trigger->_missing_teeth = 1;
    trigger->_full_teeth = 29;
    break;

  default:
    trigger->initialized = temp_status;
    change_bit(&runtime.status, STATUS_TRIGGER_ERROR, true);
    log_error("Trigger initialization failed. Wheel type not recognized.");
    break;
  }

  switch (trigger->trigger_number)
  {
  case TRIGGER_NUMBER_1:
    change_bit(&runtime.status, STATUS_TRIGGER1_SYNCED, true);
    break;
  case TRIGGER_NUMBER_2:
    change_bit(&runtime.status, STATUS_TRIGGER2_SYNCED, true);
    break;
  default:
    log_error("Trigger initialization failed. unknown trigger number.");
    change_bit(&runtime.status, STATUS_TRIGGER_ERROR, true);
    trigger->initialized = false;
    return;
  }

  set_filtering(trigger);
  trigger->_trigger_actual_teeth = trigger->_full_teeth - trigger->_missing_teeth;
  trigger->initialized = true;
}

void trigger_tooth_handle(trigger_t *trigger)
{

  if (!trigger->initialized)
  {
    change_bit(&runtime.status, STATUS_TRIGGER_ERROR, true);
    log_error("Trigger is not initialized.");
    return;
  }

  trigger->_tooth_time_us[0] = get_time_us();
  trigger->_current_tooth_gap_us = trigger->_tooth_time_us[0] - trigger->_tooth_time_us[1];

  if (trigger->_current_tooth_gap_us < trigger->_trigger_filter_time_us) // Pulses should never be less than _trigger_filter_time_us, so if they are it means a false trigger. (A 36-1 wheel at 8000rpm will have triggers approx. every 200uS)
  {
    // Reset the filter to avoid getting stuck
    // trigger->_trigger_filter_time_us = trigger->_current_tooth_gap_us / 2U;
    return;
  }

  trigger->_counted_tooth++; // Increment the tooth counter

  bool is_missing_teeth = false;

  // if we had previous data calculate the tooth gap
  if (trigger->_tooth_time_us[1] == 0 || trigger->_tooth_time_us[2] == 0)
  {
    // return;
  }

  time_us_t previous_tooth_gap = 0;

  previous_tooth_gap = trigger->_tooth_time_us[1] - trigger->_tooth_time_us[2];

  // this selects the shorter tooth gap since that is more appropriate
  if (previous_tooth_gap < trigger->_current_tooth_gap_us)
  {
    /**
     * We use the previous tooth gap since the current tooth gap might be long and indicate missing teeth
     */
    trigger->_shorter_tooth_gap = previous_tooth_gap;
  }
  else
  {
    trigger->_shorter_tooth_gap = trigger->_current_tooth_gap_us;
  }

  // If the time between the current tooth and the last is greater than 1.5x the time between the last tooth and the tooth before that, we make the assertion that we must be at the first tooth after the gap
  if (trigger->_missing_teeth == 1)
  {
    trigger->_target_tooth_gap_us = (3 * trigger->_shorter_tooth_gap) >> 1;
  } // Multiply by 1.5 (Checks for a gap 1.5x greater than the last one) (Uses bitshift to multiply by 3 then divide by 2. Much faster than multiplying by 1.5)
  else
  {
    trigger->_target_tooth_gap_us = trigger->_shorter_tooth_gap * trigger->_missing_teeth;
  } // Multiply by 2 (Checks for a gap 2x greater than the last one

  // if( (_tooth_time_us[1] == 0) || (_tooth_time_us[2] == 0) ) { _current_tooth_gap_us = 0; }

  if ((trigger->_current_tooth_gap_us > trigger->_target_tooth_gap_us) || (trigger->_counted_tooth > trigger->_trigger_actual_teeth))
  {
    // Missing tooth detected
    is_missing_teeth = true;
    /**
     * this means that we  have missed a few teeth for some reason
     */
    if (trigger->_counted_tooth < trigger->_trigger_actual_teeth)
    {
      change_bit(&runtime.status, trigger->status_synced_index, TS_NOT_SYNCED);
      runtime.sync_loss_counter++;
      if (trigger->status_synced_index == STATUS_TRIGGER1_SYNCED)
      {
        runtime.total_revolutions = 0;
        runtime.rpm = 0;
      }
      trigger->_counted_tooth = 0;
    }
    else
    {
      change_bit(&runtime.status, trigger->status_synced_index, TS_FULLY_SYNCED);
      if (trigger->status_synced_index == STATUS_TRIGGER1_SYNCED)
      {
        runtime.total_revolutions++;
      }
      trigger->_counted_tooth = 0;
      // here we can say we are at TDC!
      // osEventFlagsSet(engine.flags, ENGINE_FLAG_ROTATION_EVENT);
    }
  }

  runtime.crankshaft_angle = 360.0f / (angle_t)trigger->_full_teeth * (angle_t)trigger->_counted_tooth;
  runtime.rpm = (rpm_t)(CONVERSION_FACTOR_SECONDS_TO_MICROSECONDS * CONVERSION_FACTOR_MINUTES_TO_SECONDS / trigger->_shorter_tooth_gap / trigger->_full_teeth);
  if (trigger->trigger_number == TRIGGER_NUMBER_1)
  {
      if (get_bit(runtime.status, STATUS_TRIGGER1_SYNCED) == TS_FULLY_SYNCED && runtime.rpm >= config.cranking_rpm_threshold)
  {
    runtime.spinning_state = SS_RUNNING;
  }
  else if (get_bit(runtime.status, STATUS_TRIGGER1_SYNCED) == TS_FULLY_SYNCED && runtime.rpm < config.cranking_rpm_threshold)
  {
    runtime.spinning_state = SS_CRANKING;
  }
  else if (get_bit(runtime.status, STATUS_TRIGGER1_SYNCED) == TS_NOT_SYNCED)
  {
    runtime.spinning_state = SS_SPINNING_UP;
  }
  else
  {
    runtime.spinning_state = SS_STOPPED;
  }
  }
  


  /**
   * @todo make sure the filter cannot get stuck in any condition
   */
  if (is_missing_teeth == false)
  {
    // Regular (non-missing) tooth
    set_filtering(trigger);
  }

  trigger->_tooth_time_us[2] = trigger->_tooth_time_us[1];
  trigger->_tooth_time_us[1] = trigger->_tooth_time_us[0];

  /* call trigger driven events such as ignition etc... */
  ignition_trigger_event_handle(crankshaft_get_angle(), crankshaft_get_rpm(), get_time_us());
  /* set the os flags for the engine */
  //osEventFlagsSet(engine.flags, ENGINE_FLAG_TRIGGER_EVENT);
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

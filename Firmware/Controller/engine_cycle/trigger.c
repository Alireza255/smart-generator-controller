#include "trigger.h"
#include "controller.h"

trigger_s *trigger = NULL;

void set_filtering(trigger_s *trigger)
{
  switch (configuration.trigger.filtering)
  {
    case TF_FILTERING_NONE:
      trigger->_trigger_filter_time_us = 0;
        break;
    case TF_FILTERING_LITE:
      trigger->_trigger_filter_time_us = trigger->_current_tooth_gap_us / 4U; 
        break;
    case TF_FILTERING_MEDIUM:
      trigger->_trigger_filter_time_us = trigger->_current_tooth_gap_us / 1U;
        break;
    case TF_FILTERING_AGGRESSIVE:
      trigger->_trigger_filter_time_us = trigger->_current_tooth_gap_us * 3 / 4;
        break;
    default:
        // Handle invalid filtering level
        trigger->_trigger_filter_time_us = 0;
        break;
  }
}

void trigger_init(trigger_s *instance)
{   
  bool temp_status = true;
  if (instance == NULL)
  {
    /**
     * @todo and maybe throw an error!
     */
    return;
  }
  trigger = instance;
  
  trigger->sync_status = TS_NOT_SYNCED;
  set_filtering(trigger);
  trigger->_trigger_actual_teeth = configuration.trigger.full_teeth - configuration.trigger.missing_teeth;
    
  if (configuration.trigger.full_teeth == 0 || configuration.trigger.missing_teeth == 0)
  {
    temp_status = false;
  }
  trigger->initialized = temp_status;
    
}

void trigger_tooth_handle()
{

  if (!trigger->initialized)
  {
      /**
       * @todo add an error message
       */
      return;
  }

  trigger->_tooth_time_us[0] = get_time_us();
  trigger->_current_tooth_gap_us = trigger->_tooth_time_us[0] - trigger->_tooth_time_us[1];
  
  if (trigger->_current_tooth_gap_us < trigger->_trigger_filter_time_us) // Pulses should never be less than _trigger_filter_time_us, so if they are it means a false trigger. (A 36-1 wheel at 8000rpm will have triggers approx. every 200uS)
  {
    // Reset the filter to avoid getting stuck
    //trigger->_trigger_filter_time_us = trigger->_current_tooth_gap_us / 2U;
    return;
  }
  
  trigger->_counted_tooth++; //Increment the tooth counter
  
  bool is_missing_teeth = false;
  
  // if we had previous data calculate the tooth gap
  if (trigger->_tooth_time_us[1] == 0 || trigger->_tooth_time_us[2] == 0)
  {
    //return;
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
    

  //If the time between the current tooth and the last is greater than 1.5x the time between the last tooth and the tooth before that, we make the assertion that we must be at the first tooth after the gap
  if(configuration.trigger.missing_teeth == 1)
  {
    trigger->_target_tooth_gap_us = (3 * trigger->_shorter_tooth_gap) >> 1; 
  } //Multiply by 1.5 (Checks for a gap 1.5x greater than the last one) (Uses bitshift to multiply by 3 then divide by 2. Much faster than multiplying by 1.5)
  else
  {
    trigger->_target_tooth_gap_us = trigger->_shorter_tooth_gap * configuration.trigger.missing_teeth;
  } //Multiply by 2 (Checks for a gap 2x greater than the last one    
  
  //if( (_tooth_time_us[1] == 0) || (_tooth_time_us[2] == 0) ) { _current_tooth_gap_us = 0; } 

    
  if ((trigger->_current_tooth_gap_us > trigger->_target_tooth_gap_us) || (trigger->_counted_tooth > trigger->_trigger_actual_teeth))
  {
    //Missing tooth detected
    is_missing_teeth = true;
    /**
     * this means that we  have missed a few teeth for some reason
     */
    if (trigger->_counted_tooth < trigger->_trigger_actual_teeth)
    {
      trigger->sync_status = TS_NOT_SYNCED;
      trigger->sync_loss_counter++;
      engine.total_revolutions = 0;
      engine.rpm = 0;
      trigger->_counted_tooth = 0;
    }
    else
    {
      trigger->sync_status = TS_FULLY_SYNCED;
      engine.total_revolutions++;
      trigger->_counted_tooth = 0;
     //here we can say we are at TDC!
      osEventFlagsSet(engine.flags, ENGINE_FLAG_ROTATION_EVENT);
    }
  }
  
  engine.crankshaft_angle = 360.0f / (angle_t)configuration.trigger.full_teeth * (angle_t)trigger->_counted_tooth;
  engine.rpm = (rpm_t)(CONVERSION_FACTOR_SECONDS_TO_MICROSECONDS * CONVERSION_FACTOR_MINUTES_TO_SECONDS / trigger->_shorter_tooth_gap / configuration.trigger.full_teeth);


  if (trigger->sync_status == TS_FULLY_SYNCED && engine.rpm >= configuration.cranking_rpm_threshold)
  {
    engine.spinning_state = SS_RUNNING;
  }
  else if (trigger->sync_status == TS_FULLY_SYNCED && engine.rpm < configuration.cranking_rpm_threshold)
  {
    engine.spinning_state = SS_CRANKING;
  }
  else if (trigger->sync_status == TS_NOT_SYNCED)
  {
    engine.spinning_state = SS_SPINNING_UP;
  }
  else
  {
    engine.spinning_state = SS_STOPPED;
  }
  

  /**
   * @todo make sure the filter cannot get stuck in any condition
   */
  if(is_missing_teeth == false)
  {
    //Regular (non-missing) tooth
    set_filtering(trigger); 
  }
  
  trigger->_tooth_time_us[2] = trigger->_tooth_time_us[1];
  trigger->_tooth_time_us[1] = trigger->_tooth_time_us[0];

  /* call trigger driven events such as ignition etc... */
  ignition_trigger_event_handle(crankshaft_get_angle(), crankshaft_get_rpm(), get_time_us());
  /* set the os flags for the engine */
  osEventFlagsSet(engine.flags, ENGINE_FLAG_TRIGGER_EVENT);

}


rpm_t crankshaft_get_rpm()
{
  return engine.rpm;
}

/**
 * @brief returns the instant angle of the crankshaft
 * @note angle is updated on every trigger event!
 */
angle_t crankshaft_get_angle()
{
  return engine.crankshaft_angle;
}


#include "trigger.h"


trigger_status_s trigger_status = {0};

void set_filtering(void);

volatile controller_time_us_t trigger_filter_time_us = 0;

volatile controller_time_us_t tooth_time_us[3] = {0}; // the higher the index, the older the sample. 0 is the current tooth time
volatile controller_time_us_t current_tooth_gap_us;
volatile controller_time_us_t target_tooth_gap_us;
volatile uint16_t counted_tooth = 0;

uint8_t trigger_actual_teeth = 0;

void trigger_set_filtering(trigger_filtering_e filtering_level)
{
  settings.trigger_filtering = filtering_level;

  set_filtering();
}

void set_filtering(void)
{
  switch (settings.trigger_filtering)
  {
    case TF_FILTERING_NONE:
        trigger_filter_time_us = 0;
        break;
    case TF_FILTERING_LITE:
        trigger_filter_time_us = current_tooth_gap_us / 4U; 
        break;
    case TF_FILTERING_MEDIUM:
        trigger_filter_time_us = current_tooth_gap_us / 1U;
        break;
    case TF_FILTERING_AGGRESSIVE:
        trigger_filter_time_us = current_tooth_gap_us * 3 / 4;
        break;
    default:
        // Handle invalid filtering level
        trigger_filter_time_us = 0;
        break;
  }
}

void trigger_init(void)
{   
    trigger_status.sync_status = TS_NOT_SYNCED;
    trigger_set_filtering(settings.trigger_filtering);
    trigger_actual_teeth = settings.trigger_full_teeth - settings.trigger_missing_teeth;
    
    
    trigger_status.initialized = true;
    
}
void trigger_tooth_handle(void)
{

  if (!trigger_status.initialized)
  {
      /**
       * @todo add an error message
       */
      return;
  }

  tooth_time_us[0] = get_time_us();
  current_tooth_gap_us = tooth_time_us[0] - tooth_time_us[1];
  
  if (current_tooth_gap_us < trigger_filter_time_us) // Pulses should never be less than trigger_filter_time_us, so if they are it means a false trigger. (A 36-1 wheel at 8000rpm will have triggers approx. every 200uS)
  {
    return;
  }
  
  counted_tooth++; //Increment the tooth counter
  
  bool is_missing_teeth = false;
  
  // if we had previous data calculate the tooth gap
  if (tooth_time_us[1] == 0 || tooth_time_us[2] == 0)
  {
    //return;
  }
  
  
  controller_time_us_t previous_tooth_gap = 0;
  controller_time_us_t shorter_tooth_gap = 0;
  previous_tooth_gap = tooth_time_us[1] - tooth_time_us[2];

  // this selects the shorter tooth gap since that is more approprate 
  if (previous_tooth_gap < current_tooth_gap_us) 
  {
    /**
     * We use the previous tooth gap since the current tooth gap might be long and indicate missing teeth
     */
    shorter_tooth_gap = previous_tooth_gap;
  }
  else
  {
    shorter_tooth_gap = current_tooth_gap_us;
  }
    
    
    
  //If the time between the current tooth and the last is greater than 1.5x the time between the last tooth and the tooth before that, we make the assertion that we must be at the first tooth after the gap
  if(settings.trigger_missing_teeth == 1)
  {
    target_tooth_gap_us = (3 * shorter_tooth_gap) >> 1; 
  } //Multiply by 1.5 (Checks for a gap 1.5x greater than the last one) (Uses bitshift to multiply by 3 then divide by 2. Much faster than multiplying by 1.5)
  else
  {
      target_tooth_gap_us = shorter_tooth_gap * settings.trigger_missing_teeth;
  } //Multiply by 2 (Checks for a gap 2x greater than the last one    
  
  //if( (tooth_time_us[1] == 0) || (tooth_time_us[2] == 0) ) { current_tooth_gap_us = 0; } 

    
  if ((current_tooth_gap_us > target_tooth_gap_us) || (counted_tooth > trigger_actual_teeth))
  {
    //Missing tooth detected
    is_missing_teeth = true;
    /**
     * this means that we  have missed a few teeth for some reason
     */
    if (counted_tooth < trigger_actual_teeth)
    {
      trigger_status.sync_status = TS_NOT_SYNCED;
      trigger_status.cync_loss_counter++;
      engine_parameters.total_revolutions = 0;
      counted_tooth = 0;
    }
    else
    {
      trigger_status.sync_status = TS_FULLY_SYNCED;
      engine_parameters.total_revolutions++;
      counted_tooth = 0;
     //here we can say we are at TDC!
    }
    
  }
    
  /**
   * @todo make sure the filter cannot get stuck in any condition
   */
  if(is_missing_teeth == false)
  {
    //Regular (non-missing) tooth
    set_filtering(); 
  }
  
  tooth_time_us[2] = tooth_time_us[1];
  tooth_time_us[1] = tooth_time_us[0];

}

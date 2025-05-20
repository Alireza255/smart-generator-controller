#include "timing.h"

TIM_HandleTypeDef *timer = NULL;
static scheduler_event_s event_queue[SCHEDULER_MAX_EVENTS];

void update_next_event(void);

void controller_timing_start(TIM_HandleTypeDef *htim)
{
    if (htim == NULL)
    {
        /**
         * @todo throw an error
         */
        return;
    }
    timer = htim;
    
    /**
     * This assumes that sys clock is the same as timer clock
     */
    timer->Instance->PSC = (HAL_RCC_GetSysClockFreq() / 1000000UL) - 1;
    //timer->Instance->ARR = 100000;
    timer->Instance->EGR |= TIM_EGR_UG;  // Force an update event → immediately reloads PSC

    HAL_TIM_Base_Start_IT(timer);
    HAL_TIM_OC_Start_IT(timer, TIM_CHANNEL_1);    

    for (size_t i = 0; i < SCHEDULER_MAX_EVENTS; i++)
    {
        event_queue[i].state = EVENT_STATE_INACTIVE;
    }
    
}

time_us_t get_time_us(void)
{
    return timer->Instance->CNT;
}

time_ms_t get_time_ms(void)
{
    return timer->Instance->CNT / 1000U;
}

void update_next_event(void)
{
    time_us_t nearest_time = 0xFFFFFFFF;
    bool found = false;

    for (size_t i = 0; i < SCHEDULER_MAX_EVENTS; i++)
    {
        if (event_queue[i].state != EVENT_STATE_PENDING) continue;
        
        /* This will handle wraparound safely aparantly, but i don't know how it works yet :( */
        if (!found || (int32_t)(event_queue[i].timestamp_us - nearest_time) < 0)
        {
            found = true;
            nearest_time = event_queue[i].timestamp_us;
        }        
    }

    if (found) {
        // Schedule next interrupt
        timer->Instance->CCR1 = nearest_time;
    } else {
        // No pending events — disable future interrupts or set a dummy compare
        /* Should i set a compare in the near future ?*/
        //timer->Instance->CCR1 = 0xFFFFFFFF;
    }
}

void scheduler_timer_callback()
{
    // Exit early if this wasn't a CCR1 match
    if (!(timer->Instance->SR & TIM_SR_CC1IF))
    {
        return;
    }

    timer->Instance->SR &= ~TIM_SR_CC1IF;  // Clear the compare match flag
    time_us_t time_now_us = timer->Instance->CNT; // I have avoided the function call overhead here. might be minimal anyways...

    for (int i = 0; i < SCHEDULER_MAX_EVENTS; i++) {
        /* Continue searching if the current event is not pending*/
        if (event_queue[i].state != EVENT_STATE_PENDING) continue;

        // Check if event is due (handles wraparound safely)
        if ((int32_t)(time_now_us - event_queue[i].timestamp_us) < 0)
            continue;

        event_queue[i].state = EVENT_STATE_ACTIVE;

        if (event_queue[i].uses_arg)
        {
            event_queue[i].callback_arg(event_queue[i].arg);
        }
        else
        {
            event_queue[i].callback_noarg();
        }

        event_queue[i].state = EVENT_STATE_INACTIVE;
    }

    update_next_event();  // Schedule the next event
}


bool scheduler_schedule_event_with_arg(time_us_t timestamp_us, void (*callback)(void *), void *arg)
{
    if (callback == NULL) return false;

    for (size_t i = 0; i < SCHEDULER_MAX_EVENTS; i++) {
        if (event_queue[i].state == EVENT_STATE_INACTIVE) {
            event_queue[i].timestamp_us = timestamp_us;
            event_queue[i].callback_arg = callback;
            event_queue[i].arg = arg;
            event_queue[i].uses_arg = true;
            event_queue[i].state = EVENT_STATE_PENDING;

            update_next_event();
            return true;
        }
    }

    return false;
}


bool scheduler_schedule_event(time_us_t timestamp_us, void (*callback)(void))
{
    if (callback == NULL) return false;

    for (size_t i = 0; i < SCHEDULER_MAX_EVENTS; i++) {
        if (event_queue[i].state == EVENT_STATE_INACTIVE) {
            event_queue[i].timestamp_us = timestamp_us;
            event_queue[i].callback_noarg = callback;
            event_queue[i].arg = NULL;
            event_queue[i].uses_arg = false;
            event_queue[i].state = EVENT_STATE_PENDING;

            update_next_event();
            return true;
        }
    }
    return false;
}

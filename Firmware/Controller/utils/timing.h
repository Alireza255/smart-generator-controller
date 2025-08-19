#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "types.h"
#include "tim.h"

/* Maximum number of events that can be scheduled at any time*/
#define SCHEDULER_MAX_EVENTS 100

typedef enum
{
    /* The event has never been used, or has been completed or cancelled */
    EVENT_STATE_INACTIVE,
    /* The event is scheduled and waiting for execution */
    EVENT_STATE_PENDING,
    /* The event is currently being executed */
    EVENT_STATE_ACTIVE
} scheduler_event_state_e;

typedef struct {
    time_us_t timestamp_us;
    void (*callback_arg)(void *arg);     // used when passing an argument
    void (*callback_noarg)(void);        // used when not passing an argument
    void *arg;                           // data to pass to callback_arg
    scheduler_event_state_e state;
    bool uses_arg;                       // true = use callback_arg, false = callback_noarg
} scheduler_event_s;

/**
 * @brief starts the timer used for timing in the controller and starts the scheduler
 * @note make sure output compare for channel 1 and interrupts are enabled
 */
void controller_timing_start(TIM_HandleTypeDef *htim);

/**
 * integer time is microseconds since MCU reset, used for precise timing
 * @note must use "delta = (current - start)" to handle overflows properly.
*/
time_us_t get_time_us(void);

/**
* @brief time in milliseconds (1/1_000 of a second)
* @note must use "delta = (current - start)" to handle overflows properly.
*/
time_ms_t get_time_ms(void);

/**
 * @brief Schedule an event to be executed at a timestamp.
 *
 * @note Events that are scheduled too closely won't run at the exact specified time
 * @param timestamp_us  time in microseconds when the event will happen.
 * @param callback      Pointer to the function to call when the event is due.
 *
 * @return true if the event can be scheduled, false if the event queue is full.
 */
bool scheduler_schedule_event(time_us_t timestamp_us, void (*callback)(void));

bool scheduler_schedule_event_with_arg(time_us_t timestamp_us, void (*callback)(void *), void *arg);

void scheduler_timer_callback();



#endif // SCHEDULER_H
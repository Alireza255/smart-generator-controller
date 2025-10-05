#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <stdbool.h>
#include "types.h"   /* provides time_us_t, time_ms_t, etc. */
#include "tim.h"     /* TIM_HandleTypeDef */

/* Maximum number of events that can be scheduled at any time.
 * Can be overridden by defining SCHEDULER_MAX_EVENTS before including this header.
 */
#ifndef SCHEDULER_MAX_EVENTS
#define SCHEDULER_MAX_EVENTS 100
#endif

/* A scheduler handle: 32-bit value where high 16 bits = generation, low 16 bits = index.
 * Value 0 is reserved as an invalid handle.
 */
typedef uint32_t scheduler_handle_t;
#define SCHEDULER_HANDLE_INVALID 0U

/* Public event state enum and public event struct (for inspection/debugging only).
 * The scheduler uses an internal event-slot type with extra fields (generation, volatile
 * qualifiers). External code should not manipulate the public struct directly.
 */
typedef enum {
    EVENT_STATE_INACTIVE = 0,
    EVENT_STATE_PENDING = 1,
    EVENT_STATE_ACTIVE = 2,
} scheduler_event_state_e;

typedef struct {
    time_us_t timestamp_us;                 /* when event should fire (microseconds) */
    void (*callback_arg)(void *arg);        /* callback when passing an argument */
    void (*callback_noarg)(void);           /* callback when not passing an argument */
    void *arg;                              /* argument passed to callback_arg */
    scheduler_event_state_e state;          /* public view of state */
    bool uses_arg;                          /* true = use callback_arg */
} scheduler_event_s;

/* ---------- API ---------- */

/**
 * @brief Start and configure the timer used by the scheduler.
 *
 * @param htim Pointer to HAL TIM handle. Must be a valid timer instance and remain
 *             available for the lifetime of the scheduler.
 *
 * @note This function sets the timer prescaler to achieve ~1us ticks using
 *       HAL_RCC_GetHCLKFreq(). If your timer runs on a different clock domain
 *       (APB1/APB2 with multiplier) adjust controller_timing_start implementation
 *       in timing.c accordingly.
 */
void controller_timing_start(TIM_HandleTypeDef *htim);

/**
 * @brief Return current 32-bit microsecond timer value (raw timer CNT)
 * @note Use delta arithmetic (current - start) to handle wraparound.
 */
time_us_t get_time_us(void);

/**
 * @brief Return milliseconds derived from the microsecond timer.
 */
time_ms_t get_time_ms(void);

/**
 * @brief Schedule an event to run at timestamp_us (legacy bool-style API).
 *
 * @return true if scheduled, false if queue is full.
 */
bool scheduler_schedule_event(time_us_t timestamp_us, void (*callback)(void));

/**
 * @brief Schedule an event with an argument (legacy bool-style API).
 */
bool scheduler_schedule_event_with_arg(time_us_t timestamp_us, void (*callback)(void *), void *arg);

/* New handle-based APIs (recommended):
 * - Return a non-zero scheduler_handle_t on success. Use scheduler_cancel() to cancel.
 */
scheduler_handle_t scheduler_schedule_event_handle(time_us_t timestamp_us, void (*callback)(void));
scheduler_handle_t scheduler_schedule_event_with_arg_handle(time_us_t timestamp_us, void (*callback)(void *), void *arg);

/**
 * @brief Cancel a scheduled event referenced by handle. Returns true if cancelled.
 */
bool scheduler_cancel(scheduler_handle_t handle);

/**
 * @brief Timer ISR must call this (or your TIM IRQ handler should call it when CC1 occurs).
 *
 * @note This implementation executes callbacks in ISR context (short callbacks only).
 * @warning Long-running or blocking callbacks in ISR will break realtime behaviour.
 */
void scheduler_timer_callback(void);

#endif /* SCHEDULER_H */
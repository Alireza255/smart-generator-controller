/* timing.c
 *
 * Robust single-file scheduler implementation.
 * - Critical sections protect shared state
 * - Generation-based handles avoid reuse races
 * - Cancel API provided
 * - Callbacks are executed in ISR context (by request)
 *
 * Requirements/assumptions:
 * - CMSIS macros __get_PRIMASK(), __disable_irq(), __enable_irq() are available
 *   (standard in STM32 HAL projects).
 * - types.h defines time_us_t / time_ms_t.
 * - tim.h provides TIM_HandleTypeDef and HAL symbols.
 * - Uses 32-bit microsecond ticks (timer CNT -> 1us per tick). Tweak PSC calculation
 *   in controller_timing_start if your timer clock is different.
 */

#include "timing.h"
#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <string.h>

/* sanity check */
_Static_assert(SCHEDULER_MAX_EVENTS <= 0xFFFF, "SCHEDULER_MAX_EVENTS must fit in 16-bit index");

/* Internal event slot (keeps extra fields not present in public struct) */
typedef struct {
    volatile uint32_t timestamp_us;    /* when the event should fire */
    void (*callback_noarg)(void);
    void (*callback_arg)(void *);
    volatile void *arg;
    volatile uint8_t uses_arg;         /* 0 or 1 */
    volatile uint8_t state;            /* EVENT_STATE_* values, stored as small ints */
    volatile uint16_t generation;      /* incremented on allocation */
} event_slot_t;

/* Module state */
static TIM_HandleTypeDef *s_timer = NULL;
static event_slot_t s_slots[SCHEDULER_MAX_EVENTS];

/* ---------- Critical section helpers (PRIMASK-based) ---------- */
static inline uint32_t enter_critical(void)
{
    uint32_t prim = __get_PRIMASK();
    __disable_irq();
    return prim;
}

static inline void exit_critical(uint32_t prim)
{
    if ((prim & 1U) == 0U) {
        __enable_irq();
    }
}

/* ---------- Handle helpers ---------- */
static inline scheduler_handle_t make_handle(uint16_t generation, uint16_t index)
{
    if (generation == 0U) generation = 1U; /* avoid zero-generation (reserved) */
    return (((uint32_t)generation << 16) | (uint32_t)index);
}

static inline void decode_handle(scheduler_handle_t h, uint16_t *out_idx, uint16_t *out_gen)
{
    if (h == SCHEDULER_HANDLE_INVALID) {
        *out_idx = UINT16_MAX;
        *out_gen = 0;
        return;
    }
    *out_idx = (uint16_t)(h & 0xFFFFU);
    *out_gen = (uint16_t)((h >> 16) & 0xFFFFU);
}

/* ---------- Internal helpers ---------- */
/* Allocate a free slot. Caller must hold critical section. Returns index or -1. */
static int allocate_slot_locked(void)
{
    for (int i = 0; i < SCHEDULER_MAX_EVENTS; ++i) {
        if (s_slots[i].state == EVENT_STATE_INACTIVE) {
            /* bump generation, avoid zero */
            uint16_t gen = s_slots[i].generation + 1U;
            if (gen == 0U) gen = 1U;
            s_slots[i].generation = gen;
            s_slots[i].state = EVENT_STATE_PENDING;
            return i;
        }
    }
    return -1;
}

/* update_next_event assumes caller holds critical section. It programs CCR1 and DIER.
 * It selects the pending event with the smallest non-negative delta from now.
 * If any pending event is overdue, it will choose one and force a CC1 event.
 */
static void update_next_event_locked(void)
{
    if (!s_timer) return;

    uint32_t now = s_timer->Instance->CNT;
    bool found = false;
    uint32_t best_ts = 0;
    int best_idx = -1;
    int32_t best_dt = INT32_MAX;

    for (int i = 0; i < SCHEDULER_MAX_EVENTS; ++i) {
        if (s_slots[i].state != EVENT_STATE_PENDING) continue;

        int32_t dt = (int32_t)(s_slots[i].timestamp_us - now);
        if (dt >= 0) {
            if (!found || dt < best_dt) {
                best_dt = dt;
                best_ts = s_slots[i].timestamp_us;
                best_idx = i;
                found = true;
            }
        } else {
            /* overdue event — pick immediately (we choose the first overdue we find)
               and force the CC1 event to service it now */
            best_idx = i;
            best_ts = s_slots[i].timestamp_us;
            found = true;
            break;
        }
    }

    if (found) {
        s_timer->Instance->CCR1 = best_ts;
        s_timer->Instance->DIER |= TIM_DIER_CC1IE;
        if ((int32_t)(s_timer->Instance->CNT - best_ts) >= 0) {
            /* time has passed — generate immediate CC1 event */
            s_timer->Instance->EGR |= TIM_EGR_CC1G;
        }
    } else {
        /* no pending events — disable CC1 interrupt to save CPU */
        s_timer->Instance->DIER &= ~TIM_DIER_CC1IE;
    }
}

/* Public wrapper: obtains lock then calls locked version */
void update_next_event(void)
{
    uint32_t saved = enter_critical();
    update_next_event_locked();
    exit_critical(saved);
}

/* ---------- Initialization ---------- */
void controller_timing_start(TIM_HandleTypeDef *htim)
{
    if (htim == NULL) return;

    s_timer = htim;

    /* Configure PSC for ~1us ticks. This assumes timer input clock equals HCLK.
     * If your timer uses APB1/APB2 with multiplier, adjust accordingly.
     */
    s_timer->Instance->PSC = (HAL_RCC_GetHCLKFreq() / 1000000UL) - 1U;
    s_timer->Instance->EGR |= TIM_EGR_UG;  /* reload PSC */
    s_timer->Instance->CNT = 0;

    uint32_t saved = enter_critical();
    memset(s_slots, 0, sizeof(s_slots));
    for (int i = 0; i < SCHEDULER_MAX_EVENTS; ++i) {
        s_slots[i].state = EVENT_STATE_INACTIVE;
        s_slots[i].generation = 0;
    }
    /* Enable interrupts/OC — user should ensure HAL_TIM_Base_Start_IT and HAL_TIM_OC_Start_IT are called
     * from their initialization code (or we can call them here). To match prior behaviour we start here. */
    exit_critical(saved);

    HAL_TIM_Base_Start_IT(s_timer);
    HAL_TIM_OC_Start_IT(s_timer, TIM_CHANNEL_1);
}

/* ---------- Time getters ---------- */

time_us_t get_time_us(void)
{
    if (!s_timer) return 0;
    return (time_us_t)s_timer->Instance->CNT;
}

time_ms_t get_time_ms(void)
{
    return (time_ms_t)(get_time_us() / 1000U);
}

/* ---------- Schedule / Cancel APIs ---------- */

scheduler_handle_t scheduler_schedule_event_with_arg_handle(time_us_t timestamp_us, void (*callback)(void *), void *arg)
{
    if (callback == NULL || s_timer == NULL) return SCHEDULER_HANDLE_INVALID;

    uint32_t saved = enter_critical();
    int idx = allocate_slot_locked();
    if (idx < 0) {
        exit_critical(saved);
        return SCHEDULER_HANDLE_INVALID;
    }

    s_slots[idx].timestamp_us = timestamp_us;
    s_slots[idx].callback_arg = callback;
    s_slots[idx].callback_noarg = NULL;
    s_slots[idx].arg = arg;
    s_slots[idx].uses_arg = 1U;
    /* state already set to PENDING by allocate_slot_locked */

    scheduler_handle_t h = make_handle(s_slots[idx].generation, (uint16_t)idx);

    update_next_event_locked();
    exit_critical(saved);
    return h;
}

scheduler_handle_t scheduler_schedule_event_handle(time_us_t timestamp_us, void (*callback)(void))
{
    if (callback == NULL || s_timer == NULL) return SCHEDULER_HANDLE_INVALID;

    uint32_t saved = enter_critical();
    int idx = allocate_slot_locked();
    if (idx < 0) {
        exit_critical(saved);
        return SCHEDULER_HANDLE_INVALID;
    }

    s_slots[idx].timestamp_us = timestamp_us;
    s_slots[idx].callback_noarg = callback;
    s_slots[idx].callback_arg = NULL;
    s_slots[idx].arg = NULL;
    s_slots[idx].uses_arg = 0U;

    scheduler_handle_t h = make_handle(s_slots[idx].generation, (uint16_t)idx);

    update_next_event_locked();
    exit_critical(saved);
    return h;
}

/* Legacy bool wrappers (match existing header API) */
bool scheduler_schedule_event(time_us_t timestamp_us, void (*callback)(void))
{
    return (scheduler_schedule_event_handle(timestamp_us, callback) != SCHEDULER_HANDLE_INVALID);
}

bool scheduler_schedule_event_with_arg(time_us_t timestamp_us, void (*callback)(void *), void *arg)
{
    return (scheduler_schedule_event_with_arg_handle(timestamp_us, callback, arg) != SCHEDULER_HANDLE_INVALID);
}

bool scheduler_cancel(scheduler_handle_t handle)
{
    if (handle == SCHEDULER_HANDLE_INVALID) return false;

    uint16_t idx, gen;
    decode_handle(handle, &idx, &gen);
    if (idx >= SCHEDULER_MAX_EVENTS) return false;

    uint32_t saved = enter_critical();
    if (s_slots[idx].generation == gen &&
        (s_slots[idx].state == EVENT_STATE_PENDING || s_slots[idx].state == EVENT_STATE_ACTIVE)) {
        s_slots[idx].state = EVENT_STATE_INACTIVE;
        s_slots[idx].arg = NULL;
        s_slots[idx].callback_noarg = NULL;
        s_slots[idx].callback_arg = NULL;
        s_slots[idx].uses_arg = 0U;
        /* Do not change generation — allocate_slot_locked will bump on next allocate */
        update_next_event_locked();
        exit_critical(saved);
        return true;
    }
    exit_critical(saved);
    return false;
}

/* ---------- Timer ISR callback ---------- */
void scheduler_timer_callback(void)
{
    if (!s_timer) return;

    /* Exit early if this wasn't a CCR1 match */
    if (!(s_timer->Instance->SR & TIM_SR_CC1IF)) return;

    /* Clear the flag first */
    s_timer->Instance->SR &= ~TIM_SR_CC1IF;

    /* take a lock while we discover due events; we will release it while running callbacks
     * to avoid holding interrupts disabled for too long. (Callbacks are still executed in ISR.)
     */
    uint32_t saved = enter_critical();
    time_us_t now = (time_us_t)s_timer->Instance->CNT;

    for (int i = 0; i < SCHEDULER_MAX_EVENTS; ++i) {
        if (s_slots[i].state != EVENT_STATE_PENDING) continue;

        if ((int32_t)(now - s_slots[i].timestamp_us) < 0) continue; /* not due yet */

        /* Mark active so the slot won't be reused while we execute */
        s_slots[i].state = EVENT_STATE_ACTIVE;

        /* Snapshot callback info under lock */
        void (*cb_noarg)(void) = s_slots[i].callback_noarg;
        void (*cb_arg)(void *) = s_slots[i].callback_arg;
        void *arg = (void *)s_slots[i].arg;
        uint8_t uses_arg = s_slots[i].uses_arg;

        /* Release lock while executing callback (still inside ISR) */
        exit_critical(saved);

        /* Execute callback (ISR context). Keep these short. */
        if (uses_arg) {
            if (cb_arg) cb_arg(arg);
        } else {
            if (cb_noarg) cb_noarg();
        }

        /* Reacquire lock and mark slot inactive */
        saved = enter_critical();
        s_slots[i].state = EVENT_STATE_INACTIVE;
        s_slots[i].arg = NULL;
        s_slots[i].callback_noarg = NULL;
        s_slots[i].callback_arg = NULL;
        s_slots[i].uses_arg = 0U;
        /* continue scanning */
    }

    /* Re-arm next event and release lock */
    update_next_event_locked();
    exit_critical(saved);
}

/* Optional: a function to copy internal state into the public event array for debug/inspection
 * (not required by API; you can add if you want to expose pending list)
 */
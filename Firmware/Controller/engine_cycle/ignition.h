#ifndef IGNITION_H
#define IGNITION_H


#include "types.h"
#include "config_and_runtime.h"
#include "tables.h"
#include "timing.h"
#include "utils.h"
#include "sensors.h"
#include "trigger.h"

#define IGNITION_MIN_DWELL_TIME_MS 0.5f
#define IGNITION_MAX_DWELL_TIME_MS 4.0f
#define IGNITION_MULTI_SPARK_MAX_SPARKS 3

#define IGNITION_MIN_ADVANCE (angle_t)0
#define IGNITION_MAX_ADVANCE (angle_t)40

#define IGNITION_CLT_CORRECTION_MIN_ADVANCE (angle_t)-15
#define IGNITION_CLT_CORRECTION_MAX_ADVANCE (angle_t)15


#define IGNITION_ADVANCE_FAIL_SAFE (angle_t)10

#define IGNITION_WATCHDOG_TIMER_EXTRA_TIME_MS (float_time_ms_t)1

typedef enum
{
    IGNITION_EVENT_INACTIVE = 0,
    IGNITION_EVENT_PENDING = 1,
    IGNITION_EVENT_DWELL = 2,
    IGNITION_EVENT_FIRED = 3,
} ignition_event_status_t;

typedef struct
{
    time_us_t dwell_start_time;
    time_us_t fire_spark_time;
    controller_output_pin_t *primary_output;
    controller_output_pin_t *secondary_output;
    angle_t crank_angle_at_tdc; // The angle from 0 to 720 in which the cylinder would fire with 0 deg of advance
    ignition_event_status_t status;
} ignition_event_t;

typedef struct
{
    controller_output_pin_t pin[FIRMWARE_IGNITION_OUTPUT_MAX];
} ignition_output_pin_conf_t;

/**
 * @brief Initializes the ignition system.
 * 
 * @param outputs Pointer to the controller output pins.
 * @return true if initialization was successful, false otherwise.
 */
bool ignition_init(ignition_output_pin_conf_t *output_pin_conf);

/**
 * @brief Handles an ignition trigger event based on the crankshaft angle, RPM, and current time.
 * 
 * @param crankshaft_angle The current angle of the crankshaft.
 * @param rpm The current revolutions per minute of the engine.
 * @param current_time_us The current time in microseconds.
 * @return true if the event was handled successfully, false otherwise.
 */
void ignition_trigger_event_handle(angle_t crankshaft_angle, rpm_t rpm, time_us_t current_time_us);


void ignition_coil_begin_charge(void *arg);


void ignition_coil_fire_spark(void *arg);

/**
 * @brief Gets the duty cycle of the ignition coil as a percentage.
 * 
 * @return The duty cycle of the ignition coil.
 */
percent_t ignition_get_coil_duty_cycle();

/**
 * @brief Gets the current ignition advance angle.
 * 
 * @return The current ignition advance angle.
 */
angle_t ignition_get_advance();

#endif // IGNITION_H

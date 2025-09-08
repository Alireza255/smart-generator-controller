#ifndef TRIGGER_H
#define TRIGGER_H

#include "timing.h"
#include "types.h"
#include "cmsis_os2.h"
#include "error_handling.h"
#include "config_and_runtime.h"

typedef enum
{
    TRIGGER_FILTERING_NONE = 0,
    TRIGGER_FILTERING_LITE = 1,
    TRIGGER_FILTERING_MEDIUM = 2,
    TRIGGER_FILTERING_AGGRESSIVE = 3,
} trigger_filtering_t;

typedef enum
{
    TS_NOT_SYNCED = 0,
    TS_FULLY_SYNCED = 1,
} trigger_sync_status_t;

typedef enum
{
    TRIGGER_NUMBER_1,
    TRIGGER_NUMBER_2,
} trigger_number_t;

typedef struct
{
    trigger_number_t trigger_number;
    bool initialized;
    uint8_t status_synced_index; // which of the status bits in runtime we are using
    uint8_t *filtering;
    time_us_t _trigger_filter_time_us;
    time_us_t _tooth_time_us[3]; // the higher the index, the older the sample. 0 is the current tooth time
    time_us_t _current_tooth_gap_us;
    time_us_t _shorter_tooth_gap;
    time_us_t _target_tooth_gap_us;
    uint16_t _counted_tooth;
    uint8_t _trigger_actual_teeth;
    uint8_t _full_teeth;
    uint8_t _missing_teeth;
} trigger_t;

typedef enum {
    /**
     * The engine is not spinning, RPM=0
     */
    SS_STOPPED = 0,
    /**
     * The engine is spinning up (reliable RPM is not detected yet).
     * In this state, rpmValue is >= 0 (can be zero).
     */
    SS_SPINNING_UP = 1,
    /**
     * The engine is cranking (0 < RPM < cranking rpm)
     */
    SS_CRANKING = 2,
    /**
     * The engine is running (RPM >= cranking rpm)
     */
    SS_RUNNING = 3,
} spinning_state_t;

/**
 * @brief returns the state of the engine - ie: cranking, running, stopped
 */

/**
 * @brief returns the instant angle of the crankshaft
 * @note angle is updated on every trigger event!
 */
angle_t crankshaft_get_angle();

/**
 * @brief returns the most recent rpm calculation
 * @note might return invalid rpm, check spinning ?
 */
rpm_t crankshaft_get_rpm();

angle_t camshaft_get_angle();

/**
 * @brief called by an interrupt in the middle of the tooth i.e.- zero crossing of the signal
 * @todo add filtering capability
 */
void trigger_tooth_handle(trigger_t *trigger);
void trigger_init(trigger_t *trigger, trigger_wheel_type_t wheel_type, uint8_t *filtering, status_t sync_status_bit, uint8_t trigger_number);

#endif // TRIGGER_H

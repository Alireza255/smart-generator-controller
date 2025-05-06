#ifndef TRIGGER_H
#define TRIGGER_H

#include "controller_time.h"
#include "types.h"


typedef enum
{
    TF_FILTERING_NONE,
    TF_FILTERING_LITE,
    TF_FILTERING_MEDIUM,
    TF_FILTERING_AGGRESSIVE,
} trigger_filtering_e;
typedef struct
{
    trigger_filtering_e filtering;
    uint8_t full_teeth;
    uint8_t missing_teeth;
} trigger_settings_s;

typedef enum
{
    TS_NOT_SYNCED,
    TS_FULLY_SYNCED,
} trigger_sync_status_e;


typedef struct
{
    bool initialized;
    trigger_sync_status_e sync_status;
    uint16_t sync_loss_counter;
    trigger_settings_s settings;
    rpm_t cranking_rpm_threshold;
    
    time_us_t _trigger_filter_time_us;
    time_us_t _tooth_time_us[3]; // the higher the index, the older the sample. 0 is the current tooth time
    time_us_t _current_tooth_gap_us;
    time_us_t _shorter_tooth_gap;
    time_us_t _target_tooth_gap_us;
    uint16_t _counted_tooth;
    uint8_t _trigger_actual_teeth;

} trigger_s;

typedef enum {
    /**
     * The engine is not spinning, RPM=0
     */
    SS_STOPPED,
    /**
     * The engine is spinning up (reliable RPM is not detected yet).
     * In this state, rpmValue is >= 0 (can be zero).
     */
    SS_SPINNING_UP,
    /**
     * The engine is cranking (0 < RPM < cranking rpm)
     */
    SS_CRANKING,
    /**
     * The engine is running (RPM >= cranking rpm)
     */
    SS_RUNNING,
} spinning_state_e;

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

/**
 * @brief called by an interrupt in the middle of the tooth i.e.- zero crossing of the signal
 * @todo add filtering capability
 */
void trigger_tooth_handle();
void trigger_init(trigger_s *trigger);

#endif // TRIGGER_H

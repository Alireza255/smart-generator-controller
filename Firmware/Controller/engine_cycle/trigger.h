#ifndef TRIGGER_H
#define TRIGGER_H

#include "timing.h"
#include "types.h"
#include "cmsis_os2.h"
#include "error_handling.h"
#include "config_and_runtime.h"

#define TRIGGER_TOOTH_GAP_SYNC_RATIO_LOWER 1.5
#define TRIGGER_TOOTH_GAP_SYNC_RATIO_UPPER 3

#define TRIGGER_SPINNING_STATE_RPM_HYSTEERSIS (rpm_t)50

typedef enum
{
    TRIGGER_EVENT_TYPE_TIMEOUT,
    TRIGGER_EVENT_TYPE_CRANKSHAFT,
    TRIGGER_EVENT_TYPE_CAMSHAFT,
} trigger_event_type_t;
typedef enum
{
    TRIGGER_EVENT_EDGE_FALLING = 0,
    TRIGGER_EVENT_EDGE_RISING = 1,
    TRIGGER_EVENT_EDGE_NONE = 3,
} trigger_event_edge_t;

typedef struct
{
    time_us_t timestamp;
    trigger_event_type_t type;
    trigger_event_edge_t edge;
} trigger_event_t;


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


typedef struct
{
    bool initialized;
    uint8_t *filtering;
    
    time_us_t       tooth_time_history[2]; // the higher the index, the older the sample.
    time_us_t       filter_time;
    uint16_t        current_tooth_index;
    uint8_t         trigger_actual_teeth;
    uint8_t         full_teeth;
    uint8_t         missing_teeth;

} trigger_crankshaft_t;

typedef struct
{
    bool initialized;
    uint8_t *filtering;
    bool        phase;
    time_us_t   last_edge_time;
    time_us_t   last_edge_gap;
} trigger_camshaft_t;

typedef enum 
{
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
 * @brief returns the instant angle of the crankshaft
 * @note angle is updated on every trigger event!
 */
angle_t crankshaft_get_angle();

/**
 * @brief returns the most recent rpm calculation
 * @note might return invalid rpm, check spinning ?
 */
rpm_t crankshaft_get_rpm();

bool camshaft_get_phase();

/**
 * @brief called by an interrupt in the middle of the tooth i.e.- zero crossing of the signal
 */
void trigger_crank_handle(trigger_event_t *event);

/**
 * @brief gets called on either edge of the camshaft signal
 */
void trigger_camshaft_signal_handle(bool is_rising_edge);

void trigger_crankshaft_init(trigger_wheel_type_crankshaft_t wheel_type);

void trigger_camshaft_init(trigger_wheel_type_camshaft_t wheel_type, uint8_t *filtering);

void trigger_driven_events_callback();

angle_t crankshaft_get_next_trigger_angle();

spinning_state_t trigger_spinning_state_get();

#endif // TRIGGER_H

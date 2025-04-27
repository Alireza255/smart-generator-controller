#ifndef TRIGGER_H
#define TRIGGER_H

#include "types.h"
//#include "enums.h"
#include "controller_time.h"
#include "controller.h"
#include "settings.h"

typedef enum
{
    TS_NOT_SYNCED,
    TS_FULLY_SYNCED,
} trigger_sync_status_e;

typedef struct
{
    bool initialized;
    
    trigger_sync_status_e sync_status;

    uint16_t cync_loss_counter;
    
} trigger_status_s;

extern trigger_status_s trigger_status;

typedef enum
{
    TF_FILTERING_NONE,
    TF_FILTERING_LITE,
    TF_FILTERING_MEDIUM,
    TF_FILTERING_AGGRESSIVE,
} trigger_filtering_e;

/**
 * @brief called by an interrupt in the middle of the tooth ie- zero crossing of the signal
 * @todo add filtering capability
 */
void trigger_tooth_handle(void);
void trigger_set_filtering(trigger_filtering_e filtering_level);
void trigger_init(void);

#endif // TRIGGER_H

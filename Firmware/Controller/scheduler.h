#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "types.h"

typedef enum
{
    EVENT_STATUS_PENDING,
    EVENT_STATUS_RUNNING,
    EVENT_STATUS_STOPPED,
} event_status_e;

typedef struct
{
    event_status_e status;
    time_us_t charge_duration;


    
} ignition_event_s;


#endif // SCHEDULER_H
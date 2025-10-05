#ifndef PROTECTION_H
#define PROTECTION_H

#include "config_and_runtime.h"
#include "types.h"
#include "trigger.h"
#include "sensors.h"
#include "ignition.h"

typedef enum
{
    REV_LIMIT_ARMED = 0,
    REV_LIMIT_ETB = 1,
    REV_LIMIT_SPARK_RETARD = 2,
    REV_LIMIT_FUEL_CUT = 4,
    REV_LIMIT_SPARK_CUT = 8,
} protection_rev_limit_modes_t;

void protections_update();



#endif // PROTECTION_H
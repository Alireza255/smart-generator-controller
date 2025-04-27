#ifndef SETTINGS_H
#define SETTINGS_H

#include "enums.h"
#include "types.h"

typedef struct
{
    uint8_t trigger_filtering;
    uint8_t trigger_missing_teeth;
    uint8_t trigger_full_teeth;
} settings_s;

extern settings_s settings;

#endif // SETTINGS_H


#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <stdint.h>
#include "constants.h"

typedef struct
{
    float ve_table_1[16][16];
    float ve_table_2[16][16];
    float ignition_table[16][16];

} engine_configuration_t;


#endif // CONFIGURATION_H
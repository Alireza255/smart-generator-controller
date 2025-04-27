#ifndef CONTROLLER_H
#define CONTROLLER_H

//#include "enums.h"
#include "types.h"

typedef struct
{
    /**
     * total number of revolutions from startup
     */
    uint32_t total_revolutions;
    angle_t crankshaft_angle;
    angle_t camshaft_angle;
    
} engine_parameters_s;

extern engine_parameters_s engine_parameters;

#endif // CONTROLLER_H

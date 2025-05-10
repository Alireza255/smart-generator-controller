#ifndef CONTROLLER_H
#define CONTROLLER_H


#include "enums.h"
#include "types.h"
#include "trigger.h"
#include "dc_motors.h"
#include "tables.h"





typedef struct
{
    /**
     * total number of revolutions from startup
     */
    uint32_t total_revolutions;
    angle_t crankshaft_angle;
    angle_t camshaft_angle;
    rpm_t rpm;
   
    uint8_t cylinder_count;
    
    spinning_state_e spinning_state;

    ignition_mode_e ignition_mode;
    angle_t firing_interval;


    trigger_s trigger;
} engine_s;

extern engine_s engine;

#endif // CONTROLLER_H

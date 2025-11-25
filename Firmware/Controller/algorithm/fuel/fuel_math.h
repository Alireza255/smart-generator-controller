#ifndef FUEL_MATH_H
#define FUEL_MATH_H


#include "math.h"
#include "types.h"
#include "config_and_runtime.h"
#include "trigger.h"
#include "sensors.h"
#include "tables.h"
#include "utils.h"
#include "error_handling.h"

#define FUEL_CLT_BASED_CORRECTION_RANGE (percent_t)500
#define FUEL_ACCEL_ENRICH_RANGE (percent_t)800

// this will apply the corrections too! hopefully in the future
mass_t fuel_get_required_mass_petrol();
mass_t fuel_get_required_mass_gas();
mass_t fuel_get_required_mass_petrol_per_cycle();
void fuel_math_use_tps_map_sensors(sensor_tps_t *tps, sensor_map_t *map);
#endif // FUEL_MATH_H
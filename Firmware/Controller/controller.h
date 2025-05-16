#ifndef CONTROLLER_H
#define CONTROLLER_H


#include "enums.h"
#include "types.h"
#include "eeprom.h"
#include "trigger.h"
#include "dc_motors.h"
#include "tables.h"
#include "tps.h"
#include "ignition.h"
#include "thermistor.h"
#include "spark_logic.h"
#include "pid.h"

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

    angle_t firing_interval;

    trigger_s trigger;
} engine_s;

extern engine_s engine;

typedef struct
{
    bool is_fuel_injection_enabled;
    bool is_gas_injection_enabled;
    
    firing_order_e firing_order;

    ignition_mode_e ignition_mode;
    float_time_ms_t ignition_dwell;
    bool ignition_is_multi_spark;
    uint8_t ignition_multi_spark_number_of_sparks;
    rpm_t ignition_multi_spark_rpm_threshold;

    volume_liter_t engine_displacment;
    afr_t stoich_afr_gas;
    afr_t stoich_afr_petrol;
    
    tps_sensor_calib_s tps_calib;
    tps_sensor_calib_s gvps_calib;

    thermistor_conf_s clt_thermistor_calib;

    sensor_iat_type_e iat_sensor_type;

    sensor_map_type_e map_sensor_type;
    

} configuration_s;

extern configuration_s configuration;

void controller_init();

void controller_load_configuration();

void controller_save_configuration();

#endif // CONTROLLER_H

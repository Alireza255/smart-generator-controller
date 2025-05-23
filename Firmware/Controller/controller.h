#ifndef CONTROLLER_H
#define CONTROLLER_H


#include "enums.h"
#include "timing.h"
#include "types.h"
#include "eeprom.h"
#include "trigger.h"
#include "dc_motors.h"
#include "tables.h"
#include "tps.h"
#include "ignition.h"
#include "thermistor.h"
#include "pid.h"
#include "electronic_throttle.h"
#include "adc.h"
#include "governer.h"

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
    
    governer_status_e governer_status;
} engine_s;

extern engine_s engine;

typedef struct
{
    volume_liter_t engine_displacment;
    firing_order_e firing_order;
    fuel_type_e fuel_type;
    
    
    trigger_settings_s trigger;

    rpm_t cranking_rpm_threshold;
    angle_t cranking_advance;
    percent_t cranking_throttle;


    ignition_mode_e ignition_mode;
    float_time_ms_t ignition_dwell;
    bool ignition_is_multi_spark;
    uint8_t ignition_multi_spark_number_of_sparks;
    rpm_t ignition_multi_spark_rpm_threshold;
    float_time_ms_t ignition_multi_spark_rest_time;

    afr_t stoich_afr_gas;
    afr_t stoich_afr_petrol;
    
    sensor_tps_s tps1;
    sensor_tps_s tps2;

    thermistor_conf_s clt_thermistor_calib;

    sensor_iat_type_e iat_sensor_type;

    sensor_map_type_e map_sensor_type;

    rpm_t governer_target_rpm;
    rpm_t governer_idle_rpm;

    pid_configuration_s etb_pid;
    pid_configuration_s governer_pid;

    time_ms_t protection_oil_pressure_time;
    bool protection_oil_pressure_enabled;

    bool protection_clt_enabled;
    bool protection_clt_load_disconnect_enabled;
    temperature_t protection_clt_shutdown_temprature;
    temperature_t protection_clt_load_disconnect_temprature;
    
    
} configuration_s;

extern configuration_s configuration;

void controller_init();

void controller_load_configuration();

void controller_save_configuration();


#endif // CONTROLLER_H

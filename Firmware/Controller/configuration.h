#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "controller.h"


typedef struct
{
    bool is_ignition_enabled;
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
    
} configuration_s;

extern configuration_s configuration;

#endif // CONFIGURATION_H
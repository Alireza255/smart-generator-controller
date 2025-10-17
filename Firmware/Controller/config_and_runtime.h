

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <stdint.h>
#include "constants.h"
#include "tables.h"

#define CONFIG_CHECKSUM 140051411009

typedef struct __attribute__((packed)) 
{
    /* ---------- 4-byte items (floats and table handles) ---------- */

    /* VE tables */
    table_2d_t ve_table_1;
    table_2d_t ve_table_2;

    /* Ignition tables */
    table_2d_t ign_table_1;
    table_2d_t ign_table_2;

    /* Fuel blend */
    table_2d_t fuel_blend_table;

    /* Cranking / ignition */
    float cranking_rpm_threshold;
    float cranking_advance;
    float cranking_throttle;
    float trigger_offset_deg;

    float ignition_dwell;
    float ignition_fixed_timing_advance;
    float multi_spark_rpm_threshold;
    float multi_spark_rest_time_ms;
    float multi_spark_max_trailing_angle;

    /* Stoich AFRs */
    float stoich_afr_gas;
    float stoich_afr_petrol;

    /* Gas / priming */
    float gas_control_etb_flowrate_grams_per_sec;
    float gas_reference_pressure;
    float gas_priming_time_sec;
    float petrol_priming_time_sec;

    /* Injection */
    float injection_end_of_injection_angle; /* before TDC */
    float injector_flow_rate_cc_per_min;

    /* Governer (PID) floats */
    float governer_target_rpm;
    float governer_idle_rpm;
    float governer_pid_Kp;
    float governer_pid_Ki;
    float governer_pid_Kd;
    float governer_pid_limit_integrator_min;
    float governer_pid_limit_integrator_max;
    float governer_pid_derivative_filter_tau;

    /* ETB1 (grouped) */
    table_1d_t etb1_feedforward_table;
    float etb1_pid_Kp;
    float etb1_pid_Ki;
    float etb1_pid_Kd;
    float etb1_pid_limit_integrator_min;
    float etb1_pid_limit_integrator_max;
    float etb1_pid_derivative_filter_tau;
    float etb1_end_of_travel_duty_cycle_limit_upper;
    float etb1_end_of_travel_duty_cycle_limit_lower;

    /* ETB2 (grouped) */
    table_1d_t etb2_feedforward_table;
    float etb2_pid_Kp;
    float etb2_pid_Ki;
    float etb2_pid_Kd;
    float etb2_pid_limit_integrator_min;
    float etb2_pid_limit_integrator_max;
    float etb2_pid_derivative_filter_tau;
    float etb2_end_of_travel_duty_cycle_limit_upper;
    float etb2_end_of_travel_duty_cycle_limit_lower;

    /* Protection & fans (floats) */
    float protection_clt_shutdown_temprature;
    float protection_clt_load_disconnect_temprature;
    float fan1_on_temp;
    float fan1_off_temp;
    float fan2_on_temp;
    float fan2_off_temp;

    /* CLT-based tables */
    table_1d_t accel_enrichment_tps_table;
    table_1d_t accel_enrichment_map_table;

    table_1d_t clt_based_advance_correction_table;
    table_1d_t clt_based_fuel_correction_table_gas;
    table_1d_t clt_based_fuel_correction_table_petrol;

    /* ---------- 2-byte items (uint16_t) grouped together ---------- */
    uint16_t rev_limit;
    uint16_t rev_limit_hystersis;
    uint16_t engine_displacement_cc;

    uint16_t tps1_calib_wide_open_throttle_adc_value;
    uint16_t tps1_calib_closed_throttle_adc_value;
    uint16_t tps2_calib_wide_open_throttle_adc_value;
    uint16_t tps2_calib_closed_throttle_adc_value;

    /* ---------- 1-byte items (uint8_t) grouped & ordered by subsystem ---------- */

    /* Trigger / firing */
    uint8_t firing_order;
    uint8_t fuel_type;
    uint8_t trigger_crank_type;
    uint8_t trigger_crank_filtering;
    uint8_t trigger_cam_type;
    uint8_t trigger_cam_filtering;
    uint8_t trigger_cam_enabled;

    /* Modes & ignition */
    uint8_t injection_mode;
    uint8_t ignition_mode;
    uint8_t ignition_fixed_timing_enabled;

    /* Multi-spark */
    uint8_t multi_spark_enabled;
    uint8_t multi_spark_number_of_sparks;

    /* TPS calibration & types */
    uint8_t tps1_calib_is_inverted;
    uint8_t tps2_calib_is_inverted;
    uint8_t tps1_type;
    uint8_t tps2_type;

    /* Sensors */
    uint8_t sensor_clt_type;
    uint8_t sensor_iat_type;
    uint8_t sensor_map_type;

    /* Protections */
    uint8_t protection_oil_pressure_time;
    uint8_t protection_oil_pressure_enabled;
    uint8_t protection_clt_enabled;
    uint8_t protection_clt_load_disconnect_enabled;

    /* ETB flags */
    uint8_t etb1_enabled;
    uint8_t etb2_enabled;
    uint8_t etb1_motor_inverted;
    uint8_t etb2_motor_inverted;
    uint8_t etb1_end_of_travel_duty_cycle_limit_enabled;
    uint8_t etb2_end_of_travel_duty_cycle_limit_enabled;

    /* Fans */
    uint8_t fan1_enabled;
    uint8_t fan2_enabled;

} config_t;


typedef struct {
    /* ---------- 4-byte items (uint32 and floats) ---------- */
    uint32_t status;
    uint32_t sync_loss_counter;
    uint32_t total_revolutions;
    uint32_t seconds;

    float crankshaft_angle;
    float camshaft_angle;
    float rpm;
    float governer_target_rpm;

    float firing_interval_deg;
    float ignition_advance_deg;

    /* Sensors / inputs */
    float tps1;
    float tps2;
    float tps1_dot;
    float map_kpa;
    float map_dot;
    float baro_kpa;
    float clt_degc;
    float iat_degc;
    float egt_degc;
    float lambda;
    float lambda_target;
    float vbatt_volts;
    float etb1_target;
    float etb2_target;
    float etb1_motor_effort;
    float etb2_motor_effort;
    float airmass_grams_per_sec;
    float fuel_correction_percent;

    /* Actuators / timings */
    float dwell_actual;
    float injector_pulse_width_ms;

    uint8_t spinning_state;
    uint8_t governer_status;
    uint8_t multi_spark_actual_spark_count;

} runtime_t;

typedef enum
{
    STATUS_CRITICAL_ERROR = 0,
    STATUS_CHECK_ENGINE = 1,
    STATUS_WARNING = 2,
    STATUS_TRIGGER_CRANKSHAFT_SYNCED = 3,
    STATUS_TRIGGER_CAMSHAFT_SYNCED = 4,
    STATUS_TRIGGER_ERROR = 5,
    STATUS_FAN1_ON = 6,
    STATUS_FAN2_ON = 7,
    STATUS_NEED_BURN = 8,
    STATUS_MAIN_RELAY_ON = 9,
    STATUS_GAS_SOLENOID_ON = 10,
    STATUS_FUEL_PUMP_ON = 11,
    STATUS_O2_HEATER_ON = 12,
    STATUS_TPS1_ERROR = 13,
    STATUS_TPS2_ERROR = 14,
    STATUS_MAP_ERROR = 15,
    STATUS_CLT_ERROR = 16,
    STATUS_IAT_ERROR = 17,
    STATUS_IGNITION_ERROR = 18,
    STATUS_INJECTOR_ERROR = 19,
    STATUS_REV_LIMIT_IGNITION = 20,
    STATUS_REV_LIMIT_ETB = 21,
    STATUS_ETB1_OK = 22,
    STATUS_ETB2_OK = 23,
    STATUS_FUEL_PRESSURE_LOW_GAS = 24,
    STATUS_FUEL_PRESSURE_LOW_PETROL = 25,
    STATUS_CRANKING = 26,
    STATUS_RUNNING = 27,
    STATUS_OIL_PRESSURE_LOW = 28,
    STATUS_TOOTH_LOG_READY = 29,
    STATUS_RESERVED3 = 30,
    STATUS_RESERVED4 = 31
} status_t;



extern config_t config;
extern runtime_t runtime;

#endif // CONFIGURATION_H
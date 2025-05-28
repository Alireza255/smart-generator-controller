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
#include "comms.h"


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


/**
 * Constants Structure (Calibration Data) ---
 * This structure must exactly match the offsets and types in the .ini [Constants] section.
 * Ensure 4-byte alignment for F32 types if possible.
 * @note using packed to prevent the compiler from padding the values
 */
typedef struct __attribute__((__packed__))
{
    // Offset 0: ve_table (16x16 F32)
    float ve_table[16][16]; // Size: 16*16*4 = 1024 bytes

    // Offset 1024: ve_rpm_bins (16 U16)
    uint16_t ve_rpm_bins[16]; // Size: 16*2 = 32 bytes

    // Offset 1056: ve_load_bins (16 U16)
    uint16_t ve_load_bins[16]; // Size: 16*2 = 32 bytes

    // Offset 1088: ign_table (16x16 F32)
    float ign_table[16][16]; // Size: 16*16*4 = 1024 bytes

    // Offset 2112: ign_rpm_bins (16 U16)
    uint16_t ign_rpm_bins[16]; // Size: 16*2 = 32 bytes

    // Offset 2144: ign_load_bins (16 U16)
    uint16_t ign_load_bins[16]; // Size: 16*2 = 32 bytes

    // Offset 2176: rpm_limiter (U16)
    uint16_t rpm_limiter; // Size: 2 bytes

    // Offset 2178: engine_displacement (U16)
    uint16_t engine_displacement; // Size: 2 bytes

    // Offset 2180: firing_order (U08, bits [0:2]) - TunerStudio sees this as 1 byte
    uint8_t firing_order; // Size: 1 byte

    // Offset 2181: fuel_type (U08, bits [0:1])
    uint8_t fuel_type; // Size: 1 byte

    // Offset 2182: trigger_type (U08, bits [0:1])
    uint8_t trigger_type; // Size: 1 byte

    // Offset 2183: trigger_filtering (U08, bits [0:1])
    uint8_t trigger_filtering; // Size: 1 byte

    // Offset 2184: trigger_missing_teeth (U08)
    uint8_t trigger_missing_teeth; // Size: 1 byte

    // Offset 2185: trigger_full_teeth (U08)
    uint8_t trigger_full_teeth; // Size: 1 byte

    // Offset 2188: cranking_rpm_threshold (F32) - Note offset jump (2185 + 1 = 2186, but F32 starts at 2188)
    // This implies 2 bytes of padding/unallocated space, or a misunderstanding of how F32 offsets behave.
    // If you need strict alignment, ensure the struct member aligns correctly.
    float cranking_rpm_threshold; // Size: 4 bytes

    // Offset 2192: cranking_advance (F32)
    float cranking_advance; // Size: 4 bytes

    // Offset 2196: cranking_throttle (F32)
    float cranking_throttle; // Size: 4 bytes

    // Offset 2200: ignition_mode (U08, bits [0:1])
    uint8_t ignition_mode; // Size: 1 byte

    // Offset 2204: ignition_dwell (F32) - Note offset jump (2200+1 = 2201, but F32 starts at 2204)
    float ignition_dwell; // Size: 4 bytes

    // Offset 2208: ignition_is_multi_spark (U08, bits [0:1])
    uint8_t ignition_is_multi_spark; // Size: 1 byte

    // Offset 2209: ignition_multi_spark_number_of_sparks (U08)
    uint8_t ignition_multi_spark_number_of_sparks; // Size: 1 byte

    // Offset 2212: ignition_multi_spark_rpm_threshold (F32) - Note offset jump
    float ignition_multi_spark_rpm_threshold; // Size: 4 bytes

    // Offset 2216: ignition_multi_spark_rest_time (F32)
    float ignition_multi_spark_rest_time; // Size: 4 bytes

    // Offset 2220: stoich_afr_gas (F32)
    float stoich_afr_gas; // Size: 4 bytes

    // Offset 2224: stoich_afr_petrol (F32)
    float stoich_afr_petrol; // Size: 4 bytes

    // Offset 2228: tps1_calib_wide_open_throttle_adc_value (U16)
    uint16_t tps1_calib_wide_open_throttle_adc_value; // Size: 2 bytes

    // Offset 2230: tps1_calib_closed_throttle_adc_value (U16)
    uint16_t tps1_calib_closed_throttle_adc_value; // Size: 2 bytes

    // Offset 2232: tps1_calib_is_inverted (U08, bits [0:1])
    uint8_t tps1_calib_is_inverted; // Size: 1 byte

    // Offset 2234: tps2_calib_wide_open_throttle_adc_value (U16) - Note offset jump
    uint16_t tps2_calib_wide_open_throttle_adc_value; // Size: 2 bytes

    // Offset 2236: tps2_calib_closed_throttle_adc_value (U16)
    uint16_t tps2_calib_closed_throttle_adc_value; // Size: 2 bytes

    // Offset 2238: tps2_calib_is_inverted (U08, bits [0:1])
    uint8_t tps2_calib_is_inverted; // Size: 1 byte

    // Offset 2240: clt_calib_tempC_1 (F32) - Note offset jump
    float clt_calib_temp_c_1; // Size: 4 bytes

    // Offset 2244: clt_calib_tempC_2 (F32)
    float clt_calib_temp_c_2; // Size: 4 bytes

    // Offset 2248: clt_calib_tempC_3 (F32)
    float clt_calib_temp_c_3; // Size: 4 bytes

    // Offset 2252: clt_calib_resistance_1 (F32)
    float clt_calib_resistance_1; // Size: 4 bytes

    // Offset 2256: clt_calib_resistance_2 (F32)
    float clt_calib_resistance_2; // Size: 4 bytes

    // Offset 2260: clt_calib_resistance_3 (F32)
    float clt_calib_resistance_3; // Size: 4 bytes

    // Offset 2264: sensor_iat_type (U08, bits [0:2])
    uint8_t sensor_iat_type; // Size: 1 byte

    // Offset 2265: sensor_map_type (U08, bits [0:2])
    uint8_t sensor_map_type; // Size: 1 byte

    // Offset 2268: governer_target_rpm (F32) - Note offset jump
    float governer_target_rpm; // Size: 4 bytes

    // Offset 2272: governer_idle_rpm (F32)
    float governer_idle_rpm; // Size: 4 bytes

    // Offset 2276: governer_pid_Kp (F32)
    float governer_pid_kp; // Size: 4 bytes

    // Offset 2280: governer_pid_Ki (F32)
    float governer_pid_ki; // Size: 4 bytes

    // Offset 2284: governer_pid_Kd (F32)
    float governer_pid_kd; // Size: 4 bytes

    // Offset 2288: governer_pid_limit_integrator_min (F32)
    float governer_pid_limit_integrator_min; // Size: 4 bytes

    // Offset 2292: governer_pid_limit_integrator_max (F32)
    float governer_pid_limit_integrator_max; // Size: 4 bytes

    // Offset 2296: governer_pid_derivative_filter_tau (F32)
    float governer_pid_derivative_filter_tau; // Size: 4 bytes

    // Offset 2300: etb1_pid_Kp (F32)
    float etb1_pid_kp; // Size: 4 bytes

    // Offset 2304: etb1_pid_Ki (F32)
    float etb1_pid_ki; // Size: 4 bytes

    // Offset 2308: etb1_pid_Kd (F32)
    float etb1_pid_kd; // Size: 4 bytes

    // Offset 2312: etb1_pid_limit_integrator_min (F32)
    float etb1_pid_limit_integrator_min; // Size: 4 bytes

    // Offset 2316: etb1_pid_limit_integrator_max (F32)
    float etb1_pid_limit_integrator_max; // Size: 4 bytes

    // Offset 2320: etb1_pid_derivative_filter_tau (F32)
    float etb1_pid_derivative_filter_tau; // Size: 4 bytes

    // Offset 2324: etb2_pid_Kp (F32)
    float etb2_pid_kp; // Size: 4 bytes

    // Offset 2328: etb2_pid_Ki (F32)
    float etb2_pid_ki; // Size: 4 bytes

    // Offset 2332: etb2_pid_Kd (F32)
    float etb2_pid_kd; // Size: 4 bytes

    // Offset 2336: etb2_pid_limit_integrator_min (F32)
    float etb2_pid_limit_integrator_min; // Size: 4 bytes

    // Offset 2340: etb2_pid_limit_integrator_max (F32)
    float etb2_pid_limit_integrator_max; // Size: 4 bytes

    // Offset 2344: etb2_pid_derivative_filter_tau (F32)
    float etb2_pid_derivative_filter_tau; // Size: 4 bytes

    // Offset 2348: protection_oil_pressure_time (U08)
    uint8_t protection_oil_pressure_time; // Size: 1 byte

    // Offset 2349: protection_oil_pressure_enabled (U08, bits [0:1])
    uint8_t protection_oil_pressure_enabled; // Size: 1 byte

    // Offset 2350: protection_clt_enabled (U08, bits [0:1])
    uint8_t protection_clt_enabled; // Size: 1 byte

    // Offset 2351: protection_clt_load_disconnect_enabled (U08, bits [0:1])
    uint8_t protection_clt_load_disconnect_enabled; // Size: 1 byte

    // Offset 2352: protection_clt_shutdown_temprature (F32)
    float protection_clt_shutdown_temprature; // Size: 4 bytes

    // Offset 2356: protection_clt_load_disconnect_temprature (F32)
    float protection_clt_load_disconnect_temprature; // Size: 4 bytes

    // Total size calculated from .ini: ~2360 bytes
    // Total size from struct definition: 1024 + 32 + 32 + 1024 + 32 + 32 + 2 + 2 + 1 + 1 + 1 + 1 + 1 + 1 + 4 + 4 + 4 + 1 + 4 + 1 + 1 + 4 + 4 + 4 + 4 + 2 + 2 + 1 + 2 + 2 + 1 + 4 + 4 + 4 + 4 + 4 + 1 + 1 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 1 + 1 + 1 + 1 + 4 + 4 = 2360 bytes.
    // Ensure `sizeof(ecu_constants_t)` matches `page_size` in your INI for the used portion.
    // The `page_size` in INI (7168) is larger than the used constants, implying room for future expansion or other data.
    // You must ensure that the memory region is at least this large and accessible.
} ecu_constants_t;


// --- Output Channels Structure (Runtime Data) ---
// This structure must exactly match the offsets and types in your .ini [OutputChannels] section.
typedef struct __attribute__((__packed__))
{
    // Offset 0: Status flags (U32)
    // Bits 0:0, 1:1, 2:2, 4:4, 5:5, 6:6, 9:9, 10:10, 11:11, 12:12, 13:13, 14:14, 15:15, 19:19, 20:20, 21:21, 22:22, 23:23, 24:24, 25:25, 26:26, 27:27
    uint32_t status_flags; // Size: 4 bytes

    // Offset 4: RPM_value (F32)
    float rpm_value; // Size: 4 bytes

    // Offset 8: MAP_value (F32)
    float map_value; // Size: 4 bytes

    // Offset 12: TPS_value (F32)
    float tps_value; // Size: 4 bytes

    // Offset 16: lambda_value (F32)
    float lambda_value; // Size: 4 bytes

    // Offset 20: ignition_advance_value (F32)
    float ignition_advance_value; // Size: 4 bytes

    // Offset 24: spark_dwell_value (F32)
    float spark_dwell_value; // Size: 4 bytes

    // Offset 28: VBatt_value (F32)
    float v_batt_value; // Size: 4 bytes

    // Offset 32: coolant_temp_value (F32)
    float coolant_temp_value; // Size: 4 bytes

    // Total size calculated: 4 + 4*8 = 36 bytes.
    // You might want to define a maximum expected size for this runtime data block
    // to match your `och_block_size` in the INI if you define one later.
} ecu_runtime_data_t;


// Global instances (usually defined in a .c file, e.g., main.c or ecu_config.c)
extern ecu_constants_t ecu_constants;
extern ecu_runtime_data_t ecu_runtime_data;

// Function to initialize default constants (call on first boot or factory reset)
void ecu_constants_init_defaults(void);
// Function to load constants from non-volatile memory
void ecu_constants_load(void);
// Function to save constants to non-volatile memory
void ecu_constants_save(void);

// Helper for little-endian conversions (if needed for individual byte access or cross-platform data exchange)
// For `float` to `uint32_t` and vice versa.
uint32_t float_to_uint32_le(float f);
float uint32_le_to_float(uint32_t u);
uint16_t uint16_le(uint16_t val); // Converts to little-endian representation if system is big-endian
uint32_t uint32_le(uint32_t val);


extern configuration_s configuration;

void controller_init();

void controller_load_configuration();

void controller_save_configuration();


#endif // CONTROLLER_H

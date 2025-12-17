#include "protection.h"
/*
===============================================================
                   PROBLEMS FOUND IN THIS CODE
===============================================================


@bug 2) ETB1 health logic is broken:
        is_etb1_healthy = true;  // overwrites actual health state
        - This disables ETB1 safety entirely.
        - Remove this line.

@todo 3) Improve oil pressure logic:
        - sensor_ops_get() not validated (no RPM check, no cranking exclusion,
          no warm-up delay).
        - Causes false shutdown during cranking or transient sensor dips.

@bug 4) Oil pressure timer mixes microseconds and milliseconds.
        - delta_time is in µs.
        - counter variable name suggests ms but stores µs.
        - Fix by converting delta_time or renaming the variable.

@todo 5) Add debounce/hysteresis to oil pressure detection.
        - Currently any brief glitch resets or triggers shutdown.

@bug 6) CLT shutdown happens on a single reading.
        - No filtering, no debounce, noise = immediate shutdown.
        - Add time threshold or sample averaging.

@todo 7) protection_shutdown_gracefully() is not actually graceful.
        - Immediate ETB close + fuel cut is harsh.
        - Implement staged or ramped shutdown instead.

@bug 8) 'throttle' pointer may be NULL.
        - protections_init only sets it when input is non-NULL.
        - shutdown routine still dereferences throttle.
        - Add NULL check or enforce initialization.

@bug 9) Gas ETB logic appears incorrect:
        is_gas_fuel_injected &= config.etb2_enabled;
        - Gas fuel mode should not depend on ETB2 enabled flag.
        - Reevaluate intended logic.

@todo 10) Add latch to prevent repeated shutdown triggers.
         - protection_shutdown_gracefully() may run every cycle.
         - Add a runtime.protection_active flag.

@todo 11) Add logging / diagnostic messages for all protection events.
         - ETB fail, oil pressure fail, CLT overtemp, etc.
         - Currently impossible to debug why shutdown occurred.

@todo 12) Add filtering to sensor readings (CLT / oil pressure).
         - Noisy sensors can cause false shutdowns.

@todo 13) Standardize time units across protections.
         - Avoid mixing raw timestamps, µs, and ms in logic.
         - Choose one unit for all timers (preferably ms).

===============================================================
*/

electronic_throttle_t *throttle = NULL;
/* protections that need to be implemented */
/**
 * close the gas solenoid when gas control etb fails
 * 
 */
void protections_init(electronic_throttle_t *air_ctrl_etb)
{
    if (air_ctrl_etb != NULL)
    {
        throttle = air_ctrl_etb;
    }

}

void protections_update(time_us_t timestamp)
{
    static time_us_t last_update_time = 0;
    time_us_t elapsed_time_since_last_update = timestamp - last_update_time;
    last_update_time = timestamp;
    if (elapsed_time_since_last_update > 2000U) elapsed_time_since_last_update = 2000U; // cap to 2ms to avoid large jumps

    const time_us_t max_accumulated_failure_time = (time_us_t)0xFFFFFFFu;

    bool shutdown_is_required = false;
    static bool protection_is_tripped = false;

    /* Gas control ETB fail protection */
    bool is_gas_fuel_used = config.fuel_type == FUEL_TYPE_DUAL_FUEL || config.fuel_type == FUEL_TYPE_GAS;
    bool is_etb2_enabled = config.etb2_enabled;
    bool is_gas_fuel_injected = is_gas_fuel_used && is_etb2_enabled;
    bool is_etb2_healthy = get_bit(runtime.status, STATUS_ETB2_OK);

    static time_us_t gas_injection_accumulated_failure_time = 0;
    if (is_gas_fuel_injected && !is_etb2_healthy && (gas_injection_accumulated_failure_time < max_accumulated_failure_time))
    {
        gas_injection_accumulated_failure_time += elapsed_time_since_last_update;
    }
    else
    {
        gas_injection_accumulated_failure_time = 0;
    }
    if (gas_injection_accumulated_failure_time > FIRMWARE_GAS_INJECTION_ALLOWED_FAULT_TIME)
    {
        shutdown_is_required = true;
    }
    
    /* Oil pressure protection */
    bool ops_sensor_result = sensor_ops_get();
    bool engine_is_running = (uint16_t)crankshaft_get_rpm() >= (uint16_t)750; // consider engine running above 750 rpm
    static time_us_t low_oil_pressure_condition_total_time = 0;
    if(!ops_sensor_result && engine_is_running && (low_oil_pressure_condition_total_time < max_accumulated_failure_time))
    {
        low_oil_pressure_condition_total_time += elapsed_time_since_last_update;
    }
    else
    {
        low_oil_pressure_condition_total_time = 0;
    }
    bool is_oil_pressure_acceptable = low_oil_pressure_condition_total_time <= (config.protection_oil_pressure_time * CONVERSION_FACTOR_SECONDS_TO_MICROSECONDS);
    if (!is_oil_pressure_acceptable && config.protection_oil_pressure_enabled)
    {
        shutdown_is_required = true;
    }
    
    /* CLT protection */
    temperature_t sensor_clt_value = sensor_clt_get();
    bool clt_is_above_shutdown_temperature = sensor_clt_value >= config.protection_clt_shutdown_temprature;
    bool clt_is_above_load_disconnect_temperature = sensor_clt_value >= config.protection_clt_load_disconnect_temprature;

    static time_us_t elevated_clt_above_shutdown_total_time = 0;
    static time_us_t elevated_clt_above_load_disconnect_total_time = 0;
    if (clt_is_above_shutdown_temperature && (elevated_clt_above_shutdown_total_time < max_accumulated_failure_time))
    {
        elevated_clt_above_shutdown_total_time += elapsed_time_since_last_update;
    }
    else
    {
        elevated_clt_above_shutdown_total_time = 0;
    }
    if (clt_is_above_load_disconnect_temperature && (elevated_clt_above_load_disconnect_total_time < max_accumulated_failure_time))
    {
        elevated_clt_above_load_disconnect_total_time += elapsed_time_since_last_update;
    }
    else
    {
        elevated_clt_above_load_disconnect_total_time = 0;
    }
    bool is_clt_shutdown_condition_met = elevated_clt_above_shutdown_total_time >= FIRMWARE_PROTECTION_OVERHEATING_CONDITION_ALLOEWED_TIME;
    bool is_clt_load_disconnect_condition_met = elevated_clt_above_load_disconnect_total_time >= FIRMWARE_PROTECTION_OVERHEATING_CONDITION_ALLOEWED_TIME;

    if (config.protection_clt_enabled && is_clt_shutdown_condition_met)
    {
        shutdown_is_required = true;
    }
    if (config.protection_clt_load_disconnect_enabled && is_clt_load_disconnect_condition_met)
    {
        shutdown_is_required = true;
    }
    
    if (shutdown_is_required && !protection_is_tripped)
    {
        protection_shutdown_gracefully();
        protection_is_tripped = true;
    }

}

void protection_shutdown_gracefully()
{

    bool is_etb1_healthy = get_bit(runtime.status, STATUS_ETB1_OK);
    bool is_etb1_enabled = config.etb1_enabled;

    /* Choke the engine from air ? */
    if (is_etb1_healthy && is_etb1_enabled && throttle != NULL)
    {
        electronic_throttle_override_enable(throttle, (percent_t)0);
    }
    
    output_override(&gas_solenoid_output, false); // Shut off the gas going to the engine
    injection_disable();            // Stop the fuel injection
    
}
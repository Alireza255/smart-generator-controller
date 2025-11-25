#include "protection.h"

/* protections that need to be implemented */
/**
 * close the gas solenoid when gas control etb fails
 * 
 */
void protections_update(time_us_t time)
{
    static time_us_t prev_time = 0;
    time_us_t delta_time = time - prev_time;

    /* Gas control ETB fail protection */
    bool is_gas_fuel_injected = config.fuel_type == FUEL_TYPE_DUAL_FUEL || config.fuel_type == FUEL_TYPE_GAS;
    bool is_etb2_healthy = get_bit(runtime.status, STATUS_ETB2_OK);

    if (is_gas_fuel_injected && !is_etb2_healthy)
    {
        /* Gracefully shut down the engine or do not allow it to start*/
        protection_shutdown_gracefully();
    }
    
    /* Oil pressure protection */
    bool is_oil_pressure_good = sensor_ops_get();
    time_ms_t low_oil_pressure_counter = 0;
    if(!is_oil_pressure_good && runtime.spinning_state == SS_RUNNING)
    {
        low_oil_pressure_counter += delta_time;
    }
    if (low_oil_pressure_counter > (config.protection_oil_pressure_time * 1000) && config.protection_oil_pressure_enabled)
    {
        protection_shutdown_gracefully();
    }
    
    /* CLT protection */
    bool is_clt_acceptable = sensor_clt_get() < config.protection_clt_shutdown_temprature;
    if (!is_clt_acceptable && config.protection_clt_enabled)
    {
        protection_shutdown_gracefully();
    }

}

void protection_shutdown_gracefully()
{
    static bool protection_has_tripped = false;
    if (protection_has_tripped == true)
    {
        return;
    }
    protection_has_tripped = true;


    bool is_etb1_healthy = get_bit(runtime.status, STATUS_ETB1_OK);
    if (is_etb1_healthy)
    {
        /* Choke the engine from air ? */
    }
    
    output_gas_solenoid_set(false); // Shut off the gas going to the engine
    injection_disable();            // Stop the fuel injection
    
}
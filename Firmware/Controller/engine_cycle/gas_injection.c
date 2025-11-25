#include "gas_injection.h"

static electronic_throttle_t *etb = NULL;

void gas_injection_init(electronic_throttle_t *gas_control_etb)
{
    if (gas_control_etb == NULL)
    {
        log_error("gas injection init failed!");
        return;
    }

    etb = gas_control_etb;
}

void gas_injection_update(void *arg)
{
    if (etb == NULL)
    {
        return;
    }

    percent_t etb_opening = 0;
    mass_t gas_mass = 0;
    pressure_t gas_inlet_pressure = 2.0f;

    gas_mass = fuel_get_required_mass_gas();

    if (gas_inlet_pressure < FIRMWARE_LIMIT_GAS_PRESSURE_MIN)
    {
        change_bit(&runtime.status, STATUS_FUEL_PRESSURE_LOW_GAS, true);
        electronic_throttle_set(etb, (percent_t)0);
        return;
    }
    if (get_time_us() < (time_us_t)(config.gas_priming_time_sec * (float)CONVERSION_FACTOR_SECONDS_TO_MICROSECONDS) && trigger_spinning_state_get() == SS_STOPPED)
    {
        etb_opening = config.gas_control_etb_priming_position_percent;
        output_gas_solenoid_set(true);
        change_bit(&runtime.status, STATUS_GAS_SOLENOID_ON, true);
    }
    else if (gas_mass > FIRMWARE_GAS_INJECTION_MIN_MASS)
    {
        etb_opening = gas_mass / config.gas_control_etb_flowrate_grams_per_sec * config.gas_reference_pressure / gas_inlet_pressure;
        etb_opening = CLAMP(etb_opening, (float)0, (float)1);
        etb_opening = asinf(etb_opening) / PI * (float)200; // convert to percentage
        etb_opening = CLAMP(etb_opening, (float)0, (float)100);
        output_gas_solenoid_set(true);
        change_bit(&runtime.status, STATUS_GAS_SOLENOID_ON, true);
    }
    else
    {
        etb_opening = (percent_t)0;
        output_gas_solenoid_set(false);
        change_bit(&runtime.status, STATUS_GAS_SOLENOID_ON, false);
    }
    
    electronic_throttle_set(etb, etb_opening);
}

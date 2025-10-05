#include "fuel_math.h"
/**
 * Ideal gas law is PV = nRT
 * n = m / M        while M is molar weight
 * Derived via:
 * (8.31 J K mol^-1)  <- ideal gas constant R
 *
 * (28.97g mol^-1)    <- molar mass of air
 * = 0.28705 J*K/g
 */
#define AIR_MOLAR_MASS 28.9647f          // g/mol
#define UNIVERSAL_GAS_CONST 8.314462618f // J/(K*mol)

mass_t air_mass_get()
{
    mass_t air_mass = 0;

    rpm_t rpm = crankshaft_get_rpm();
    pressure_t map = sensor_map_get();
    temperature_t iat = sensor_iat_get();
    percent_t ve = 0;

    // get VE from table based on fuel type
    switch (config.fuel_type)
    {
    case FUEL_TYPE_GAS:
        ve = table_2d_get_value(&config.ve_table_1, rpm, map);
        break;
    case FUEL_TYPE_GASOLINE:
        ve = table_2d_get_value(&config.ve_table_2, rpm, map);
        break;

    case FUEL_TYPE_DUAL_FUEL:
        float blend_ratio = table_2d_get_value(&config.fuel_blend_table, crankshaft_get_rpm(), sensor_map_get());
        ve = (table_2d_get_value(&config.ve_table_1, rpm, map) * (blend_ratio * 0.01f)) +
             (table_2d_get_value(&config.ve_table_2, rpm, map) * (1.0f - (blend_ratio * 0.01f)));
        break;

    default:
        /* Defaults to natural gas */
        ve = 0;
        log_warning("Unknown fuel type.");
        change_bit(&runtime.status, STATUS_CRITICAL_ERROR, true);
        break;
    }

    // check if this is mathematically correct
    // map in kpa, iat in degC, ve in percent (0-100), rpm in revolutions per minute
    // engine_displacement_cc in cubic centimeters
    air_mass = map * (float)config.engine_displacement_cc * rpm * (ve * 0.01f) * AIR_MOLAR_MASS * 1.0e-3f / (120.0f * UNIVERSAL_GAS_CONST * CELSIUS_TO_KELVIN(iat));
    return air_mass;
}

mass_t fuel_get_required_mass_petrol()
{
    mass_t air_mass = air_mass_get();
    mass_t fuel_mass = 0;
    // get VE from table based on fuel type
    switch (config.fuel_type)
    {
    case FUEL_TYPE_GAS:
        fuel_mass = 0;
        break;
    case FUEL_TYPE_GASOLINE:
        fuel_mass = air_mass / config.stoich_afr_petrol;
        break;

    case FUEL_TYPE_DUAL_FUEL:
        float blend_ratio = table_2d_get_value(&config.fuel_blend_table, crankshaft_get_rpm(), sensor_map_get());
        fuel_mass = (air_mass / config.stoich_afr_petrol) * (1.0f - (blend_ratio * 0.01f));
        break;

    default:
        /* no fuel will be injected */
        fuel_mass = 0;
        log_warning("Unknown fuel type.");
        change_bit(&runtime.status, STATUS_CRITICAL_ERROR, true);
        break;
    }
    percent_t clt_correction = (percent_t)table_1d_get_value(&config.clt_based_fuel_correction_table_petrol, sensor_clt_get());
    if (IS_IN_RANGE(clt_correction, (percent_t)((float)-1 * FUEL_CLT_BASED_CORRECTION_RANGE), (percent_t)(FUEL_CLT_BASED_CORRECTION_RANGE)) && !isnan(clt_correction))
    {
        fuel_mass += clt_correction / (percent_t)100 * fuel_mass;
    }
    return fuel_mass;
}

mass_t fuel_get_required_mass_petrol_per_cycle()
{
    mass_t petrol_mass_grams_per_second = 0;
    petrol_mass_grams_per_second = fuel_get_required_mass_petrol();
    return petrol_mass_grams_per_second / (crankshaft_get_rpm() / 120.0f);
}

mass_t fuel_get_required_mass_gas_per_cycle()
{
    mass_t gas_mass_grams_per_second = 0;
    gas_mass_grams_per_second = fuel_get_required_mass_gas();
    return gas_mass_grams_per_second / (crankshaft_get_rpm() / 120.0f);
}

mass_t fuel_get_required_mass_gas()
{
    mass_t air_mass = air_mass_get();
    mass_t fuel_mass = 0;
    // get VE from table based on fuel type
    switch (config.fuel_type)
    {
    case FUEL_TYPE_GAS:
        fuel_mass = air_mass / config.stoich_afr_gas;
        break;
    case FUEL_TYPE_GASOLINE:
        fuel_mass = 0;
        break;

    case FUEL_TYPE_DUAL_FUEL:
        float blend_ratio = table_2d_get_value(&config.fuel_blend_table, crankshaft_get_rpm(), sensor_map_get());
        fuel_mass = (air_mass / config.stoich_afr_gas) * (blend_ratio * 0.01f);
        break;

    default:
        /* no fuel will be injected */
        fuel_mass = 0;
        log_warning("Unknown fuel type.");
        change_bit(&runtime.status, STATUS_CRITICAL_ERROR, true);
        break;
    }
    
    percent_t clt_correction = (percent_t)table_1d_get_value(&config.clt_based_fuel_correction_table_gas, sensor_clt_get());
    if (IS_IN_RANGE(clt_correction, (percent_t)((float)-1 * FUEL_CLT_BASED_CORRECTION_RANGE), (percent_t)(FUEL_CLT_BASED_CORRECTION_RANGE)) && !isnan(clt_correction))
    {
        fuel_mass += clt_correction / (percent_t)100 * fuel_mass;
    }

    return fuel_mass;
}
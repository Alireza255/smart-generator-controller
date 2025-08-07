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
#define AIR_R 0.28705f

mass_t ideal_gas_law(float volume, pressure_t pressure, temperature_t temperature);

/**
 * @brief Calculates air mass in grams using RPM, volumetric efficiency (VE),
 *        and intake air temperature (IAT).
 *
 * This function uses the ideal gas law and engine parameters to estimate
 * the mass of air entering the engine.
 * Ideal gas law PV = nRT
 *
 * @param rpm Engine speed in revolutions per minute.
 * @param pressure Manifold abslute pressure.
 * @param iat Intake air temperature in Kelvin.
 * @return The calculated air mass in grams.
 */
mass_t air_mass_get(rpm_t rpm, percent_t ve, pressure_t map, temperature_t iat);

mass_t ideal_gas_law(float volume, pressure_t pressure, temperature_t temperature)
{
	return volume * pressure / (AIR_R * temperature);
}

mass_t air_mass_get(rpm_t rpm, percent_t ve, pressure_t map, temperature_t iat)
{
    mass_t air_mass = 0;
    // check if this is mathematically correct
    air_mass = ve * ideal_gas_law(config.engine_displacement_cc, map, CELSIUS_TO_KELVIN(iat));
    return air_mass;
}

mass_t fuel_get_required_mass(mass_t air_mass, afr_t afr)
{
    mass_t fuel_mass = 0;
    switch (config.fuel_type)
    {
    case FUEL_TYPE_GAS:
        fuel_mass = air_mass / config.stoich_afr_gas;
        break;
    case FUEL_TYPE_GASOLINE:
        fuel_mass = air_mass / config.stoich_afr_petrol;
        break;

    case FUEL_TYPE_DUAL_FUEL:
        /* @todo implement this*/
        break;
        
    default:
        /* Defults to natural gas */
        fuel_mass = air_mass / config.stoich_afr_gas;
        log_warning("Defaulting to natural gas as fuel.");
        break;
    }

    return fuel_mass;
}

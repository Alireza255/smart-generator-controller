#include "airmass.h"
#include "utils.h"

mass_t ideal_gas_law(float volume, pressure_t pressure, temperature_t temperature)
{
	return volume * pressure / (AIR_R * temperature);
}

mass_t air_mass_get(rpm_t rpm, percent_t ve, pressure_t map, temperature_t iat)
{
    mass_t air_mass = 0;
    // check if this is mathematically correct
    air_mass = ve * ideal_gas_law(configuration.engine_displacment, map, CELSIUS_TO_KELVIN(iat));
    return air_mass;
}

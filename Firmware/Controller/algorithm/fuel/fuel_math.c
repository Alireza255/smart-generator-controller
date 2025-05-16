#include "fuel_math.h"

mass_t fuel_get_required_mass(mass_t air_mass, afr_t afr)
{
    return air_mass / configuration.stoich_afr_gas;
}

#ifndef FUEL_MATH_H
#define FUEL_MATH_H

#include "airmass.h"

// this will apply the corrections too! hopefully in the future
mass_t fuel_get_required_mass(mass_t air_mass, afr_t afr);


#endif // FUEL_MATH_H
/**
 * @file airmass.h
 * @brief Header file for air mass calculation algorithm.
 *
 * This file contains the declarations and definitions required for calculating
 * the air mass entering the engine. The calculation is based on various factors
 * such as engine RPM, manifold absolute pressure (MAP) sensor, and intake air
 * temperature (IAT) sensor.
 *
 * @author Alireza Eskandari
 */


#ifndef AIRMASS_H
#define AIRMASS_H

#include "types.h"
#include "configuration.h"

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

mass_t ideal_gas_law(float volume, pressure_t pressure, temperature_t temperature)
{
	return volume * pressure / (AIR_R * temperature);
}

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
 * @param iat Intake air temperature in degrees Celsius.
 * @return The calculated air mass in grams.
 */
mass_t air_mass_get(rpm_t rpm, percent_t ve, pressure_t map, temperature_t iat);


#endif // AIRMASS_H
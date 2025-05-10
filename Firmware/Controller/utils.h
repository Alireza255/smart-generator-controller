/**
/**
 * @file utils.h
 * @brief Header file containing utility functions and definitions for the smart generator controller firmware.
 * 
 * This file provides declarations for utility functions and macros that are
 * used throughout the firmware to support various functionalities.
 * 
 * @note Ensure that this file is included wherever utility functions are required.
 */
#ifndef UTILS_H
#define UTILS_H

#include "types.h"
#include "math.h"
#define IS_IN_RANGE(value, min, max) ((value) >= (min) && (value) <= (max))
#define ABS(value) ((value) < 0 ? -(value) : (value))

#define CELSIUS_TO_KELVIN(celsius) ((celsius) + 273.15f)
#define KELVIN_TO_CELSIUS(kelvin) ((kelvin) - 273.15f)

/**
 * @brief function to calculate the microseconds it takes for the engine to turn one degree
 * @note returns 0 if rpm is 0
 */
static inline time_us_t microseconds_per_degree(rpm_t rpm)
{
    if (rpm == 0)
    {
        return 0;
    }
    return (time_us_t) 60.0f * 1e6f / (rpm * 360.0f);
}


#endif // UTILS_H
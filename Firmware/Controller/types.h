#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <float.h>
#include <string.h>
#include "stm32f4xx_hal.h"

/**
 * @note What happens when a timing variable overflows?
 * it can be proven that wrapping won't cause any problems and the math still checks out.
 * That's how unsigned mathematics works.
 * If  current is smaller then start, the result will be: current + UINT32_MAX - start.
 * Simply do this:
 * uint32_t start, current, delta;
 * delta = (current - start); // The math works and accounts for the wrap. DON'T USE end = (start + x) type constructs.
 * we just have to make sure we use this type of math!
 */


 /**
 * integer time in milliseconds (1/1_000 of a second)
 * @note must use "delta = (current - start)" to handle overflows properly.
 */
typedef uint32_t controller_time_ms_t;


/**
 * integer time is microseconds since MCU reset, used for precise timing
 * @note must use "delta = (current - start)" to handle overflows properly.
*/
typedef uint32_t controller_time_us_t;


typedef struct {
    uint32_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    // uint16_t millisecond;
} controller_datetime_t;

typedef float angle_t;
typedef float rpm_t;

// temperature, in Celsius
typedef float temperature_t;

typedef float percent_t;

typedef uint16_t pwm_freq_t;
 
#define CONVERSION_FACTOR_SECONDS_TO_MICROSECONDS 1000000UL
#define CONVERSION_FACTOR_SECONDS_TO_MILLISECONDS 1000U
#define CONVERSION_FACTOR_MINUTES_TO_SECONDS 60U
#define CONVERSION_FACTOR_HOURS_TO_MINUTES 60U
#define CONVERSION_FACTOR_HOURS_TO_SECONDS (CONVERSION_FACTOR_HOURS_TO_MINUTES * CONVERSION_FACTOR_MINUTES_TO_SECONDS)
#define CONVERSION_FACTOR_DAYS_TO_HOURS 24
#define CONVERSION_FACTOR_DAYS_TO_MINUTES (CONVERSION_FACTOR_DAYS_TO_HOURS * CONVERSION_FACTOR_HOURS_TO_MINUTES)
#define CONVERSION_FACTOR_DAYS_TO_SECONDS (CONVERSION_FACTOR_DAYS_TO_HOURS * CONVERSION_FACTOR_HOURS_TO_SECONDS)
#define CONVERSION_FACTOR_MILLISECONDS_TO_MICROSECONDS 1000

#endif // TYPES_H
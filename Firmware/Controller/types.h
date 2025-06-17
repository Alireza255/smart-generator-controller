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
typedef uint32_t time_ms_t;


/**
 * integer time is microseconds since MCU reset, used for precise timing
 * @note must use "delta = (current - start)" to handle overflows properly.
*/
typedef uint32_t time_us_t;

/**
 * float time ms type
 */
typedef float float_time_ms_t;

/**
 * float time us type
 */
typedef float float_time_us_t;

typedef struct {
    uint32_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    // uint16_t millisecond;
} datetime_t;

typedef float angle_t;
typedef float rpm_t;

// temperature, in Celsius
typedef float temperature_t;
// mass in grams
typedef float mass_t;

// ratio of air mass to fuel mass
typedef float afr_t;

// pressure in kpa
typedef float pressure_t;

typedef float volume_liter_t;

typedef float percent_t;
typedef float voltage_t;

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


typedef enum
{
    ENGINE_FLAG_TRIGGER_EVENT = 1,
    ENGINE_FLAG_ROTATION_EVENT = 2,
    ENGINE_FLAG_OIL_PRESSURE_LOW = 4,
    ENGINE_FLAG_COOLANT_TEMPERATURE_HIGH = 8,
    ENGINE_FLAG_FAN_ON = 16,


} engine_flags_t;


typedef struct
{
    GPIO_TypeDef    *gpio;
    uint32_t         pin;
} controller_output_pin_t;



/**
 * @brief Trigger wheel type
 */
typedef enum __attribute__ ((__packed__)) {
    /**
     * 58 teeth with 2 missing tooth wheel
     */
    TW_58_TOOTH_2_MISSING = 0,
    /**
     * 29 teeth with 1 missing tooth wheel
     */
    TW_29_TOOTH_1_MISSING = 1,

} trigger_wheel_type_e;

/**
 * @brief Ignition Mode
 */
typedef enum __attribute__ ((__packed__)) {
    /**
     * in this mode the ecu won't produce any sparks
     */
    IM_NO_IGNITION = 0,
    /**
     * in this mode only one spark output is used
     */
    IM_ONE_COIL = 1,
    /**
     * in this mode we use as many coils as we have cylinders
     */
    IM_INDIVIDUAL_COILS = 2,
    /**
     * in this mode we use one coil for every two cylinders
     */
    IM_WASTED_SPARK = 3,
 
} ignition_mode_e;

/**
 * @brief Injection Mode
 */
typedef enum __attribute__ ((__packed__)) {
    /**
     * each cylinder has it's own injector but they all works in parallel
     */
    IM_SIMULTANEOUS = 0,
    /**
     * each cylinder has it's own injector, each injector is wired separately
     */
    IM_SEQUENTIAL = 1,
    /**
     * each cylinder has it's own injector but these injectors work in pairs. Injectors could be wired in pairs or separately.
     * Each pair is fired once per engine cycle
     */
    IM_BATCH = 2,
    /**
     * only one injector for the whole engine
     */
    IM_SINGLE_POINT = 3,

} injection_mode_e;

/**
 * @brief Fuel Type
 */
typedef enum __attribute__ ((__packed__)) {
    /**
     * Gaseus fuel
     */
    FUEL_TYPE_GAS = 0,
    /**
     * Gasoline fuel type
     */
    FUEL_TYPE_GASOLINE = 1,
    /**
     * both fuel types are used, useful for when we want to start on gasoline and switch to natural gas after startup
     *
     */
    FUEL_TYPE_DUAL_FUEL = 2,

} fuel_type_t;



typedef enum __attribute__ ((__packed__)) {
    /**
     * This is the default mode in which ECU controls timing dynamically
     */
    TM_DYNAMIC = 0,
    /**
     * Fixed timing is useful while we are playing with a timing gun - you need to have fixed
     * timing if you want to install your distributor at some specific angle
     */
    TM_FIXED = 1,
 
} ignition_timing_mode_e;

typedef enum __attribute__ ((__packed__))
{
    /**
     * Normal 4 cylinder firing order
     */
    FO_1342,
} firing_order_e;

typedef enum __attribute__ ((__packed__))
{
    SENSOR_IAT_TYPE_TEST,
    SENSOR_IAT_TYPE_BOSCH_816,
} sensor_iat_type_t;


typedef enum __attribute__ ((__packed__))
{
    SENSOR_MAP_TYPE_TEST,
    SENSOR_MAP_TYPE_BOSCH_816,
} sensor_map_type_t;

typedef enum __attribute__ ((__packed__))
{
    SENSOR_CLT_TYPE_TEST,
    SENSOR_CLT_TYPE_NISSAN,
} sensor_clt_type_t;



#endif // TYPES_H
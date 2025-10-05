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

#define PI 3.14159265358979323846f
#define IS_IN_RANGE(value, min, max) ((value) >= (min) && (value) <= (max))
#define ABS(value) ((value) < 0 ? -(value) : (value))
#define CLAMP(value, min, max) ((value) < (min) ? (min) : ((value) > (max) ? (max) : (value)))

#define SIZE_OF_ARRAY(arr) (sizeof(arr) / sizeof(arr[0]))

bool is_phase_in_range(float test, float current, float next);

size_t nearest_index_float(const float *arr, size_t size, float target);
size_t nearest_index_u8(const uint8_t *arr, size_t size, uint8_t target);
size_t nearest_index_u16(const uint16_t *arr, size_t size, uint16_t target);
size_t nearest_index_u32(const uint32_t *arr, size_t size, uint32_t target);

static inline void change_bit(uint32_t *var, uint8_t bit, bool state)
{
    if (bit > 31)
    {
        return;
    }
    if (state)
    {
        *var |= (1u << bit);
    }
    else
    {
        *var &= ~(1u << bit);
    }
}

static inline bool get_bit(uint32_t flags, uint8_t bit)
{
    if (bit < 32)
    {
        return (flags >> bit) & 1u;
    }
    return false;
}

/**
 * @brief Maps a value from one range to another.
 *
 * @param x The value to map.
 * @param in_min The lower bound of the input range.
 * @param in_max The upper bound of the input range.
 * @param out_min The lower bound of the output range.
 * @param out_max The upper bound of the output range.
 * @return The mapped value in the output range.
 */
#define MAP(x, in_min, in_max, out_min, out_max) \
    (((x) - (in_min)) * ((out_max) - (out_min)) / ((in_max) - (in_min)) + (out_min))

/**
 * @brief Maps a uint16_t value from one range to another.
 *
 * @param x The value to map.
 * @param in_min The lower bound of the input range.
 * @param in_max The upper bound of the input range.
 * @param out_min The lower bound of the output range.
 * @param out_max The upper bound of the output range.
 * @return The mapped value in the output range.
 */
static inline uint16_t map_uint16(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max)
{
    if (in_max == in_min)
    {
        return 0; // Avoid division by zero
    }
    return (uint16_t)((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
}

static inline float mapf(float x, float in_min, float in_max, float out_min, float out_max)
{
    if (in_max == in_min)
    {
        return 0.0f; // Avoid division by zero
    }
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// http://en.wikipedia.org/wiki/Endianness
inline uint16_t swap_endian_uint16(uint16_t x)
{
    return ((x << 8) | (x >> 8));
}
inline uint32_t swap_endian_uint32(uint32_t x)
{
    return (((x >> 24) & 0x000000ff) | ((x << 8) & 0x00ff0000) |
            ((x >> 8) & 0x0000ff00) | ((x << 24) & 0xff000000));
}

void swap_endian_copy_uint8(uint8_t *dst, const uint8_t *src, size_t size);

#define CELSIUS_TO_KELVIN(celsius) ((celsius) + 273.15f)
#define KELVIN_TO_CELSIUS(kelvin) ((kelvin) - 273.15f)

/**
 * @brief Function to calculate the microseconds it takes for the engine to turn one degree.
 * @note Returns 0 if rpm is 0.
 */
static inline time_us_t microseconds_per_degree(rpm_t rpm)
{
    if (rpm == 0)
    {
        return 0;
    }
    return (time_us_t)60.0f * 1e6f / (rpm * 360.0f);
}

/**
 * @brief Convert a rotation angle in degrees to time in microseconds at a given RPM.
 *
 * @param degrees Rotation angle in degrees (> 0)
 * @param rpm     Rotational speed in RPM (> 0)
 * @return Time in microseconds to rotate the given angle, or 0 if input is invalid
 */
static inline time_us_t degree_to_microseconds(angle_t degrees, rpm_t rpm)
{
    if (rpm <= 0 || degrees <= 0)
    {
        return 0;
    }
    return (time_us_t) (60.0f * 1e6f / (rpm * 360.0f) * degrees); 
}

/**
 * @brief Function to calculate the degrees the engine turns in one microsecond.
 * @note Returns 0 if rpm is 0.
 */
static inline angle_t degrees_per_microsecond(rpm_t rpm)
{
    if (rpm == 0)
    {
        return 0.0f;
    }
    return (angle_t)(rpm * 360.0f) / (60.0f * 1e6f);
}

static inline angle_t degrees_per_millisecond(rpm_t rpm)
{
    if (rpm == 0)
    {
        return 0.0f;
    }
    return (angle_t)(rpm * 360.0f) / (60.0f * 1e3f);
}

float rate_of_change_per_sec(float current_value, float prev_value, time_us_t current_time, time_us_t prev_time);


/**
 * @brief Convert milliseconds to microseconds, preserving the type of the input.
 *
 * Usage:
 *   time_us_t t = MILLISECONDS_TO_MICROSECONDS(5.5f);   // float -> float microseconds
 *   uint32_t t_int = MILLISECONDS_TO_MICROSECONDS(5U);  // uint32_t -> uint32_t microseconds
 */
#define MILLISECONDS_TO_MICROSECONDS(x) ((typeof(x))((x) * (typeof(x))1000))

float interpolate_2d(float x0, float y0, float x1, float y1, float x);

/*----------------------------------------------------------------------------*\
 *  NAME:
 *     Crc32_ComputeBuf() - computes the CRC-32 value of a memory buffer
 *  DESCRIPTION:
 *     Computes or accumulates the CRC-32 value for a memory buffer.
 *     The 'inCrc32' gives a previously accumulated CRC-32 value to allow
 *     a CRC to be generated for multiple sequential buffer-fuls of data.
 *     The 'inCrc32' for the first buffer must be zero.
 *  ARGUMENTS:
 *     inCrc32 - accumulated CRC-32 value, must be 0 on first call
 *     buf     - buffer to compute CRC-32 value for
 *     bufLen  - number of bytes in buffer
 *  RETURNS:
 *     crc32 - computed CRC-32 value
 *  ERRORS:
 *     (no errors are possible)
\*----------------------------------------------------------------------------*/
uint32_t crc32_inc(uint32_t in_crc32, const void *buf, size_t size);

/**
 * @brief Wraps an angle in degrees to the range [0, 360).
 *
 * This function ensures that the input angle is normalized to the range [0, 360).
 * If the input angle is negative, it adds 360 repeatedly until the angle is non-negative.
 * If the input angle is greater than or equal to 360, it subtracts 360 repeatedly
 * until the angle is less than 360.
 *
 * @param angle The input angle in degrees to be wrapped.
 * @return The wrapped angle in the range [0, 360).
 */
static inline angle_t wrap_angle_360(angle_t angle)
{
    while (angle < 0.0f)
    {
        angle += 360.0f;
    }
    while (angle >= 360.0f)
    {
        angle -= 360.0f;
    }
    return angle;
}
/**
 * @brief Wraps an angle in degrees to the range [0, 720).
 *
 * This function ensures that the input angle is normalized to the range [0, 720).
 * If the input angle is negative, it adds 720 repeatedly until the angle is non-negative.
 * If the input angle is greater than or equal to 720, it subtracts 720 repeatedly
 * until the angle is less than 720.
 *
 * @param angle The input angle in degrees to be wrapped.
 * @return The wrapped angle in the range [0, 720).
 */
static inline angle_t wrap_angle_720(angle_t angle)
{
    while (angle < 0.0f)
    {
        angle += 720.0f;
    }
    while (angle >= 720.0f)
    {
        angle -= 720.0f;
    }
    return angle;
}

/**
 * @brief Compute the forward angular distance from `from` to `to` in a 720° cycle.
 *
 * Always returns a value in the range [0, 720).
 * This represents the forward (positive rotation) distance,
 * not the shortest path between the angles.
 *
 * Useful whenever angles are cyclic with a period of 720°,
 *
 * Examples:
 *   angular_forward_distance_720(700, 20)   -> 40
 *   angular_forward_distance_720(100, 20)   -> 640
 *   angular_forward_distance_720(180, 270)  -> 90
 *
 * @param from Start angle, in degrees [0..720)
 * @param to   Target angle, in degrees [0..720)
 * @return Forward angular distance in degrees [0..720)
 */
static inline angle_t angular_forward_distance_720(angle_t from, angle_t to)
{
    angle_t dist = to - from;
    if (dist < 0.0f) {
        dist += 720.0f;
    }
    return dist;
}

/**
 * @brief Compute the forward angular distance from `from` to `to` in a 360° cycle.
 *
 * Always returns a value in the range [0, 360).
 * This represents the forward (positive rotation) distance,
 * not the shortest path between the angles.
 *
 * Useful whenever angles are cyclic with a period of 360°,
 *
 * Examples:
 *   angular_forward_distance_360(350, 10)  -> 20
 *   angular_forward_distance_360(100, 20)  -> 280
 *   angular_forward_distance_360(90, 180)  -> 90
 *
 * @param from Start angle, in degrees [0..360)
 * @param to   Target angle, in degrees [0..360)
 * @return Forward angular distance in degrees [0..360)
 */
static inline angle_t angular_forward_distance_360(angle_t from, angle_t to)
{
    angle_t dist = to - from;
    if (dist < 0.0f) {
        dist += 360.0f;
    }
    return dist;
}

#endif // UTILS_H
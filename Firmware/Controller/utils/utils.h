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
#define CLAMP(value, min, max) ((value) < (min) ? (min) : ((value) > (max) ? (max) : (value)))

void change_bit_uint32(uint32_t *var, uint8_t bit_index, bool state);

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
	return (((x >> 24) & 0x000000ff) | ((x <<  8) & 0x00ff0000) |
			((x >>  8) & 0x0000ff00) | ((x << 24) & 0xff000000));
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
    return (time_us_t) 60.0f * 1e6f / (rpm * 360.0f);
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

#endif // UTILS_H
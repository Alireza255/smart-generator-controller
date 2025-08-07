/**
 * @file constants.h
 * @brief Header file for defining constants used in the firmware of the smart generator controller.
 *
 * This is intended to hold constant definitions that are shared across the firmware.
 *
 */
#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "types.h"


// The highest rpm the firmware is expected to ever see
// this is different from cutt-off rpm
#define FIRMWARE_INJECTOR_OUPUT_COUNT 4
#define FIRMWARE_IGNITION_OUTPUT_COUNT 4

#define FIRMWARE_LIMIT_MAX_RPM 6000
#define FIRMWARE_LIMIT_MAX_MAP (pressure_t)120
#define FIRMWARE_LIMIT_MAX_TEMP (temperature_t)150
#define FIRMWARE_LIMIT_MIN_TEMP (temperature_t)-50

#define TABLE_PRIMARY_SIZE_Y 16
#define TABLE_PRIMARY_SIZE_X 16


#endif // CONSTANTS_H
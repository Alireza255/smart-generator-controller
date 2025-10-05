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
#define FIRMWARE_INJECTOR_OUTPUT_COUNT 4
#define FIRMWARE_IGNITION_OUTPUT_MAX 4
#define FIRMWARE_MAX_NUMBER_OF_CYLINDERS 8

#define FIRMWARE_LIMIT_GAS_PRESSURE_MIN (pressure_t)0.5
#define FIRMWARE_LIMIT_GAS_PRESSURE_MAX (pressure_t)3.0

#define FIRMWARE_NOMINAL_VBAT (voltage_t)14

#define FRIMWARE_TOOTH_LOGGER_BUFFER_ENTRIES 127

#define FIRMWARE_LIMIT_MAX_RPM 6000
#define FIRMWARE_LIMIT_MAX_MAP (pressure_t)120
#define FIRMWARE_LIMIT_MAX_TEMP (temperature_t)150
#define FIRMWARE_LIMIT_MIN_TEMP (temperature_t)-50
#define FIRMWARE_LIMIT_NUMBER_OF_CYLINDERS_MAX 8

#define TABLE_PRIMARY_SIZE_Y 16
#define TABLE_PRIMARY_SIZE_X 16


#endif // CONSTANTS_H
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
#define FIRMWARE_IGNITION_OUTPUT_COUNT 4
#define FIRMWARE_MAX_NUMBER_OF_CYLINDERS 8

#define FIRMWARE_LIMIT_GAS_PRESSURE_MIN (pressure_t)0.5
#define FIRMWARE_LIMIT_GAS_PRESSURE_MAX (pressure_t)3.0

#define FIRMWARE_GAS_INJECTION_ALLOWED_FAULT_TIME (time_us_t)500000u // 500 milliseconds

#define FIRMWARE_PROTECTION_OVERHEATING_CONDITION_ALLOEWED_TIME (time_us_t)5000000u // 5 seconds

#define FIRMWARE_NOMINAL_VBAT (voltage_t)12.6

#define FRIMWARE_TOOTH_LOGGER_BUFFER_ENTRIES 127

#define FIRMWARE_LIMIT_TPS_SENSORS_MAX 5
#define FIRMWARE_LIMIT_MAX_RPM 6000
#define FIRMWARE_LIMIT_MAX_MAP (pressure_t)120
#define FIRMWARE_LIMIT_MAX_TEMP (temperature_t)150
#define FIRMWARE_LIMIT_MIN_TEMP (temperature_t)-50
#define FIRMWARE_LIMIT_NUMBER_OF_CYLINDERS_MAX 8

#define FIRMWARE_TRIGGER_EVENT_QUEUE_DEPTH 16
#define FTRMWARE_TRIGGER_EVENT_TIMEOUT_MS 50

#define TABLE_PRIMARY_SIZE_Y 16
#define TABLE_PRIMARY_SIZE_X 16

#define TABLE_SMALL_SIZE_Y 8
#define TABLE_SMALL_SIZE_X 8

#endif // CONSTANTS_H
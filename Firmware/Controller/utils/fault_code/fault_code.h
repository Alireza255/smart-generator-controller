#ifndef FAULT_CODE_H
#define FAULT_CODE_H

#include <stdint.h>
#include <stdbool.h>
#include "types.h"

typedef enum __attribute__((packed))
{
    /* Engine Sensors */
    FAULT_CRANK_SENSOR_FAULT,
    FAULT_MAP_SENSOR1_FAULT,
    FAULT_IAT_SENSOR_FAULT,
    FAULT_CLT_SENSOR_FAULT,
    FAULT_EGT_SENSOR_FAULT,
    FAULT_LAMBDA_SENSOR_FAULT,
    FAULT_DBW1_SENSOR_FAULT,
    FAULT_DBW2_SENSOR_FAULT,

    FAULT_CRANK_POSITION_INVALID,
    FAULT_CAM_POSITION_INVALID,
    FAULT_CLT_INVALID,
    FAULT_MAP_INVALID,
    FAULT_IAT_INVALID,
    FAULT_EGT_INVALID,
    FAULT_LAMBDA_INVALID,


    /* Outputs */
    FAULT_OUTPUT_IGNITION_COIL_1,
    FAULT_OUTPUT_IGNITION_COIL_2,
    FAULT_OUTPUT_IGNITION_COIL_3,
    FAULT_OUTPUT_IGNITION_COIL_4,
    FAULT_OUTPUT_INJECTOR_1,
    FAULT_OUTPUT_INJECTOR_2,
    FAULT_OUTPUT_INJECTOR_3,
    FAULT_OUTPUT_INJECTOR_4,
    FAULT_OUTPUT_MAIN_RELAY,
    FAULT_OUTPUT_FUEL_PUMP,
    FAULT_OUTPUT_GAS_SOLENOID,
    FAULT_OUTPUT_FAN_1,
    FAULT_OUTPUT_FAN_2,
    FAULT_LAMBDA_SENSOR_HEATER,
    FAULT_DBW1_MOTOR_FAULT,
    FAULT_DBW2_MOTOR_FAULT,

    /* IOs */
    FAULT_CANBUS_COMM,
    FAULT_USB_COMM,
    FAULT_I2C_COMM,
    FAULT_SPI_COMM,

    /* Conditions */
    FAULT_OVERHEATING,
    FAULT_OVERHEATING_LOAD_DISCONNECT,
    FAULT_OVERHEATING_SHUTDOWN,
    FAULT_LOW_OIL_PRESSURE,
    FAULT_BATTERY_VOLTAGE_LOW,
    FAULT_BATTERY_VOLTAGE_HIGH,
    FAULT_ALTERNATOR,
    FAULT_OVERSPEED,
    FAULT_OVERBOOST,
    FAULT_KNOCK,
    FAULT_MISFIRE,

    /* Fuel System */
    FAULT_PETROL_PRESSURE_LOW,
    FAULT_PETROL_PRESSURE_HIGH,
    FAULT_GAS_PRESSURE_LOW,
    FAULT_GAS_PRESSURE_HIGH,


    /* Firmware related codes */
    FAULT_FIRMWARE_DATA_SAVE,
    FAULT_FIRMWARE_DATA_LOAD,
    FAULT_FIRMWARE_DATA_LOAD_CHECKSUM,

    FAULT_COUNT                       // Total number of fault codes
} fault_code_t;

typedef enum __attribute__((packed))
{
    FAULT_PERSISTENCY_TEMPORARY = 0, // clears automatically
    FAULT_PERSISTENCY_PERSISTENT     // stored in flash
} fault_persistency_t;

typedef enum __attribute__((packed))
{
    FAULT_SEVERITY_INFO = 1,
    FAULT_SEVERITY_WARNING = 2,
    FAULT_SEVERITY_CRITICAL = 3
} fault_severity_t;

typedef struct {
    const fault_code_t code;
    const char *name;
    const char *description;
    const char *suggested_fix;
    const fault_severity_t severity;
    const fault_persistency_t persistency;
} fault_info_t;

typedef struct {
    bool active;
    date_time_t date_time;
    uint16_t occurrence_count;
} fault_state_t;

typedef struct __attribute__((packed))
{
    uint16_t fault_code;
    uint16_t occurrence_count;
    date_time_t first_occurrence;
} fault_flash_record_t;

void fault_set(fault_code_t code);
void fault_clear(fault_code_t code);
bool fault_is_active(fault_code_t code);
bool fault_any_critical_active(void);
bool fault_any_warning_active(void);
bool fault_any_info_active(void);

#endif// FAULT_CODE_H
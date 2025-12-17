#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H

#define ERROR_HANDLING_ENABLED


#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#ifdef ERROR_HANDLING_ENABLED
#include "usbd_cdc_if.h"

#endif

#define ERROR_HANDLING_BUFFER_SIZE 100

typedef enum {
    /* Engine Sensors */
    FAULT_CRANK_SENSOR,
    FAULT_MAP_SENSOR,
    FAULT_IAT_SENSOR,
    FAULT_CLT_SENSOR,
    FAULT_EGT_SENSOR,
    FAULT_LAMBDA_SENSOR,
    FAULT_DBW_SENSOR1,
    FAULT_DBW_SENSOR2,

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
    FALUT_CODE_OUTPUT_GAS_SOLENOID,
    FAULT_OUTPUT_FAN_1,
    FAULT_OUTPUT_FAN_2,
    FAULT_LAMBDA_SENSOR_HEATER,
    FAULT_DBW_MOTOR1,
    FAULT_DBW_MOTOR2,

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

/**
 * @bug multiple logs back to back 
 */
/**
 * @brief Logs a warning message for debugging or diagnostic purposes.
 * 
 * @param message The error message to be logged.
 */

void log_error(const char* message);

/**
 * @brief Logs a warning message to the system log or console.
 *
 * This function is used to record warning messages that indicate
 * potential issues or non-critical errors in the system. The message
 * should provide enough detail to help diagnose the issue.
 *
 * @param message A string containing the warning message.
 */
void log_warning(const char* message);

/**
 * @brief Logs a debug message with variable arguments over USB CDC.
 *
 * This function behaves similarly to printf, allowing formatted output with a "Debug: " prefix.
 * It supports variable arguments for flexible message formatting.
 * A newline is appended if buffer space allows.
 * It checks if the format is NULL before proceeding.
 *
 * Example usage:
 * log_debug("Value of x: %d", x);
 *
 * @param format The format string (like printf).
 * @param ... Variable arguments corresponding to the format string.
 */
void log_debug(const char* format, ...);

void fault_code_set(uint8_t code);

#endif // ERROR_HANDLING_H
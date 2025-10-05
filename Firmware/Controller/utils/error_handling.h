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


#endif // ERROR_HANDLING_H
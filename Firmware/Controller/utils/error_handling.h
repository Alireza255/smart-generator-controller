#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H

#define ERROR_HANDLING_ENABLED true


#include <stdio.h>
#include <string.h>
#if ERROR_HANDLING_ENABLED == true
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


#endif // ERROR_HANDLING_H
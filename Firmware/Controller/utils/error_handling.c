#include "error_handling.h"

#ifdef ERROR_HANDLING_ENABLED
void log_error(const char* message)
{
    if (message == NULL) return;

    uint8_t buffer[ERROR_HANDLING_BUFFER_SIZE] = {0};

    snprintf((char*)buffer, ERROR_HANDLING_BUFFER_SIZE, "Error: %s\n", message);

    CDC_Transmit_FS(buffer, strlen((char*)buffer));
}

void log_warning(const char* message)
{
    if (message == NULL) return;

    uint8_t buffer[ERROR_HANDLING_BUFFER_SIZE] = {0};

    snprintf((char*)buffer, ERROR_HANDLING_BUFFER_SIZE, "Warning: %s\n", message);

    CDC_Transmit_FS(buffer, strlen((char*)buffer));
}

void log_debug(const char* format, ...)
{
    if (format == NULL) return;

    uint8_t buffer[ERROR_HANDLING_BUFFER_SIZE] = {0};
    va_list args;
    va_start(args, format);

    // Add prefix and format the rest
    int prefix_len = snprintf((char*)buffer, ERROR_HANDLING_BUFFER_SIZE, "Debug: ");
    if (prefix_len > 0 && prefix_len < ERROR_HANDLING_BUFFER_SIZE) {
        vsnprintf((char*)buffer + prefix_len, ERROR_HANDLING_BUFFER_SIZE - prefix_len, format, args);
    }
    // Append newline if space allows
    size_t len = strlen((char*)buffer);
    if (len < ERROR_HANDLING_BUFFER_SIZE - 1) {
        buffer[len] = '\n';
        buffer[len + 1] = '\0';
    }

    va_end(args);

    CDC_Transmit_FS(buffer, strlen((char*)buffer));
}
#else
void log_error(const char* message)
{
    
}

void log_warning(const char* message)
{
    
}

void log_debug(const char* format, ...)
{
    
}
#endif
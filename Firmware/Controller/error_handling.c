#include "error_handling.h"


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
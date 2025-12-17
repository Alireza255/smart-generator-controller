#include "trigger_logger.h"

static time_us_t tooth_data[FRIMWARE_TOOTH_LOGGER_BUFFER_ENTRIES] = {0};
static uint8_t composite_data[FRIMWARE_TOOTH_LOGGER_BUFFER_ENTRIES * sizeof(time_us_t) + FRIMWARE_TOOTH_LOGGER_BUFFER_ENTRIES];
static volatile bool trigger_logging_enabled = true;
static volatile size_t buffer_index = 0;
static bool is_logging_composite = 0;

void trigger_logger_entry_add_composite(time_us_t timestamp, composite_logger_state_t state)
{
    if (!trigger_logging_enabled) return;
    if (!is_logging_composite) return;
    if (buffer_index >= FRIMWARE_TOOTH_LOGGER_BUFFER_ENTRIES - 1)
    {
        change_bit(&runtime.status, STATUS_TOOTH_LOG_READY, true);
        return;
    }
    change_bit(&runtime.status, STATUS_TOOTH_LOG_READY, false);

    uint8_t *entry_index = &composite_data[buffer_index];
    *(time_us_t*)entry_index = timestamp;
    entry_index += sizeof(time_us_t);
    uint8_t state_translated = 0;
    change_bit8(&state_translated, 0, state.primary);
    change_bit8(&state_translated, 1, state.secondary);
    change_bit8(&state_translated, 2, state.tertiary);
    change_bit8(&state_translated, 3, state.trigger);
    change_bit8(&state_translated, 4, state.sync);
    change_bit8(&state_translated, 5, state.engine_cycle);
    *(uint8_t*)entry_index = state_translated;
    buffer_index++;
}
void trigger_logger_entry_add_tooth(time_us_t timestamp)
{
    if (!trigger_logging_enabled) return;
    if (is_logging_composite) return;
    if (buffer_index >= FRIMWARE_TOOTH_LOGGER_BUFFER_ENTRIES - 1)
    {
        change_bit(&runtime.status, STATUS_TOOTH_LOG_READY, true);
        return;
    }
    change_bit(&runtime.status, STATUS_TOOTH_LOG_READY, false);

    static time_us_t prev_time = 0;
    // simple logger
    tooth_data[buffer_index] = timestamp - prev_time;
    prev_time = timestamp;

    buffer_index++;
}

uint8_t *trigger_logger_get_composite_data()
{
    return composite_data;
}
size_t trigger_logger_get_composite_data_size()
{
    return sizeof(composite_data);
}
uint8_t *trigger_logger_get_tooth_data()
{
    return (uint8_t*)tooth_data;
}
size_t trigger_logger_get_tooth_data_size()
{
    return sizeof(tooth_data);
}
bool trigger_logger_get_buffer_status()
{
    return buffer_index >= FRIMWARE_TOOTH_LOGGER_BUFFER_ENTRIES - 1;
}

void trigger_logger_tooth_start()
{
    is_logging_composite = false;
    trigger_logging_enabled = true;
    buffer_index = 0;
}
void trigger_logger_tooth_stop()
{
    trigger_logging_enabled = false;
}

void trigger_logger_composite_start()
{
    is_logging_composite = true;
    trigger_logging_enabled = true;
    buffer_index = 0;
}
void trigger_logger_composite_stop()
{
    trigger_logging_enabled = false;
}

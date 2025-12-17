#ifndef TRIGGER_LOGGER_H
#define TRIGGER_LOGGER_H

#include "types.h"
#include "timing.h"
#include "constants.h"
#include "utils.h"
#include "config_and_runtime.h"

/**
 * state:
    bit 0 → pri cam level
    bit 1 → sec cam level
    bit 2 → third cam level
    bit 3 → which tooth
    bit 4 → sync
    bit 5 → engine cycle
    bit 6-7 → unused
 */
typedef struct
{
    uint8_t primary        : 1;
    uint8_t secondary      : 1;
    uint8_t tertiary       : 1;
    uint8_t trigger        : 1;
    uint8_t sync           : 1;
    uint8_t engine_cycle   : 1;
    uint8_t unused1        : 1;
    uint8_t unused2        : 1;
} composite_logger_state_t;
typedef struct
{

} trigger_logger_data_t;




void trigger_logger_entry_add_composite(time_us_t timestamp, composite_logger_state_t state);
void trigger_logger_entry_add_tooth(time_us_t timestamp);

uint8_t *trigger_logger_get_composite_data();
size_t trigger_logger_get_composite_data_size();
uint8_t *trigger_logger_get_tooth_data();
size_t trigger_logger_get_tooth_data_size();
bool trigger_logger_get_buffer_status();

void trigger_logger_tooth_start();
void trigger_logger_tooth_stop();

void trigger_logger_composite_start();
void trigger_logger_composite_stop();

#endif // TRIGGER_LOGGER_H
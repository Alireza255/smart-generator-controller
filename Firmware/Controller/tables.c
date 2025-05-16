#include "tables.h"
#include "error_handling.h"
#include "utils.h"

table_ignition_t ignition_table;
table_ve_t ve_table;


percent_t table_ve_get_value(table_ve_t *table, rpm_t rpm, pressure_t load)
{
    if (!IS_IN_RANGE(rpm, 0, FIRMWARE_LIMIT_MAX_RPM) || !IS_IN_RANGE(load, 0, FIRMWARE_LIMIT_MAX_MAP))
    {
        log_error("Table error! Out of bound access request");
        return 0;
    }
    
    uint8_t x_size = sizeof(table->data) / sizeof(table->data[0]); // Number of rows
    uint8_t y_size = sizeof(table->data[0]) / sizeof(table->data[0][0]); // Number of columns

    if (x_size == 0 || y_size == 0)
    {
        log_error("Table access error! Dim is 0");
        return 0;
    }
    rpm_t rpm_bin_size = FIRMWARE_LIMIT_MAX_RPM / x_size;
    pressure_t load_bin_size = FIRMWARE_LIMIT_MAX_MAP / y_size;

    uint8_t rpm_index = (uint8_t)(rpm / rpm_bin_size);
    uint8_t load_index = (uint8_t)(load / load_bin_size);

    return table->data[rpm_index][load_index];

}

angle_t table_ignition_get_value(table_ignition_t *table, rpm_t rpm, pressure_t load)
{
    if (!IS_IN_RANGE(rpm, 0, FIRMWARE_LIMIT_MAX_RPM) || !IS_IN_RANGE(load, 0, FIRMWARE_LIMIT_MAX_MAP))
    {
        log_error("Table error! Out of bound access request");
        return 0;
    }
    
    uint8_t x_size = sizeof(table->data) / sizeof(table->data[0]); // Number of rows
    uint8_t y_size = sizeof(table->data[0]) / sizeof(table->data[0][0]); // Number of columns

    if (x_size == 0 || y_size == 0)
    {
        log_error("Table access error! Dim is 0");
        return 0;
    }
    rpm_t rpm_bin_size = FIRMWARE_LIMIT_MAX_RPM / x_size;
    pressure_t load_bin_size = FIRMWARE_LIMIT_MAX_MAP / y_size;

    uint8_t rpm_index = (uint8_t)(rpm / rpm_bin_size);
    uint8_t load_index = (uint8_t)(load / load_bin_size);

    return table->data[rpm_index][load_index];
    
}

table_ignition_t* table_ignition_get_table()
{
    return &ignition_table;
}

table_ve_t* table_ve_get_table()
{
    return &ve_table;
}
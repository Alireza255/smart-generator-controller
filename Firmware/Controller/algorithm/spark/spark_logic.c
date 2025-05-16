#include "spark_logic.h"


angle_t spark_logic_get_advance()
{
    temperature_t intake_air_temp = sensor_iat_get();
    table_ignition_t *ignition_table = table_ignition_get_table();
    rpm_t rpm = crankshaft_get_rpm();
    pressure_t map = sensor_map_get();

    angle_t table_advance = table_ignition_get_value(ignition_table, rpm, map);

    /* Here we can apply all kinds of correction to the table */
    angle_t final_advance = table_advance; // + corrections
    


    return final_advance;
}
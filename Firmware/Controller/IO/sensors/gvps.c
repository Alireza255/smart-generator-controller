#include "gvps.h"

tps_sensor_s sensor_gvps = {0};

void sensor_gvps_init()
{
    sensor_tps_init(&sensor_gvps, &configuration.gvps_calib);
}

percent_t sensor_gvps_get()
{
    percent_t result = SENSOR_GVPS_FAIL_SAFE;

    result = sensor_tps_get(&sensor_gvps);
    return result;
}
#include "tps.h"
#include "error_handling.h"
#include <math.h>
#include "utils.h"


void sensor_tps_init(tps_sensor_s *sensor, tps_sensor_calib_s *sensor_calib)
{
    if (sensor == NULL || sensor_calib == NULL)
    {
        log_error("tps init failed!");
        return;
    }
    if (sensor_calib->wide_open_throttle_adc_value == 0 && sensor_calib->closed_throttle_adc_value == 0)
    {
        log_error("tps init fail no senosr calib");
        return;
    }
    sensor->calib = sensor_calib;
    sensor->is_initialized = true;
}

percent_t sensor_tps_get(tps_sensor_s *sensor)
{
    if (!sensor->is_initialized)
    {
        log_error("tps no init!");
        return SENSOR_TPS_FAIL_SAFE;
    }
    percent_t result = 0;
    uint16_t raw_data = analog_inputs_get_data(sensor->calib->analog_channel);
    if (sensor->calib->is_inverted)
    {
        result = MAP((float)raw_data, (float)sensor->calib->wide_open_throttle_adc_value, (float)sensor->calib->closed_throttle_adc_value, (float)0, (float)100);
    }
    else
    {
        result = MAP((float)raw_data, (float)sensor->calib->closed_throttle_adc_value, (float)sensor->calib->wide_open_throttle_adc_value, (float)0, (float)100);
    }
    if (!IS_IN_RANGE(result, (percent_t)0, (percent_t)100))
    {
        return SENSOR_TPS_FAIL_SAFE;
        log_error("tps out of range!");
    }
    return result;
}
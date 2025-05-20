#include "tps.h"
#include "error_handling.h"
#include <math.h>
#include "utils.h"

percent_t sensor_tps_get(sensor_tps_s *sensor)
{
    if (sensor == NULL)
    {
        log_error("tps is null");
        return SENSOR_TPS_FAIL_SAFE;
    }
     if (sensor->wide_open_throttle_adc_value == 0 && sensor->closed_throttle_adc_value == 0)
    {
        log_error("tps wrong sensor calib");
        return SENSOR_TPS_FAIL_SAFE;
    }
    percent_t result = 0;
    uint16_t raw_data = analog_inputs_get_data(sensor->analog_channel);
    if (sensor->is_inverted)
    {
        result = mapf((float)raw_data, (float)sensor->wide_open_throttle_adc_value, (float)sensor->closed_throttle_adc_value, (float)0, (float)100);
    }
    else
    {
        result = mapf((float)raw_data, (float)sensor->closed_throttle_adc_value, (float)sensor->wide_open_throttle_adc_value, (float)0, (float)100);
    }
    if (!IS_IN_RANGE(result, (percent_t)0, (percent_t)100))
    {
        return SENSOR_TPS_FAIL_SAFE;
        log_error("tps out of range!");
    }
    return result;
}
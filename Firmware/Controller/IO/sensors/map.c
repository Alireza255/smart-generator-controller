#include "map.h"

sensor_map_calib_s map_calib = {0};

void sensor_map_init()
{
    switch (configuration.map_sensor_type)
    {
    case SENSOR_MAP_TYPE_BOSCH_816:
        /**
         * a simple linear sensor
         */
        /**
         * @todo actually calculate these values!
         */
        map_calib.adc_value_0_bar = 100;
        map_calib.adc_value_1_bar = 3000;
        break;
    
    default:
        break;
    }
}

pressure_t sensor_map_get()
{
    if (map_calib.adc_value_0_bar == 0 && map_calib.adc_value_1_bar == 0)
    {
        log_error("map sensor no init!");
        return SENSOR_MAP_FAIL_SAFE;
    }
    
    uint16_t raw_adc_value = 0;
    raw_adc_value = analog_inputs_get_data(ANALOG_INPUT_SENSOR_MAP_PIN);
    
    pressure_t result = SENSOR_MAP_FAIL_SAFE;
    switch (configuration.map_sensor_type)
    {
    case SENSOR_MAP_TYPE_BOSCH_816:
        result = mapf((float)raw_adc_value, map_calib.adc_value_0_bar, map_calib.adc_value_1_bar, (pressure_t)0, (pressure_t)100);
        break;
    
    default:
        result = SENSOR_MAP_FAIL_SAFE;
        break;
    }
    return result;
}

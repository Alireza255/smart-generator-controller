
/**
 * @file sensors.c
 * @brief Implementation of the merged sensor library for CLT, IAT, MAP, OPS, and TPS sensors.
 * @author Alireza Eskandari
 * @date 2025-05-29
 */

#include "sensors.h"
#include <stdbool.h> // For bool type in sensor_ops_get and sensor_tps_s
#include <math.h>    // For NAN in SENSOR_TPS_FAIL_SAFE
#include "utils.h"   // For mapf and IS_IN_RANGE in sensor_tps_get

// Section for Coolant Temperature (CLT) Sensor
static resistor_s sensor_clt_resistor;
static thermistor_s sensor_clt;

void sensor_clt_init(void)
{
    resistor_init(&sensor_clt_resistor, 10000, RESISTOR_PULL_UP, ANALOG_INPUT_SENSOR_CLT_PIN);
    thermistor_init(&sensor_clt, configuration.clt_thermistor_calib);
}

temperature_t sensor_clt_get(void)
{
    temperature_t clt_temp = 0;
    clt_temp = thermistor_get_temp(&sensor_clt);
    if (clt_temp == NAN)
    {
        clt_temp = SENSOR_CLT_FAIL_SAFE;
    }
    return clt_temp;
}

// Section for Intake Air Temperature (IAT) Sensor
static resistor_s sensor_iat_resistor = {0};
static thermistor_s sensor_iat = {0};

void sensor_iat_init(void)
{
    switch (configuration.iat_sensor_type)
    {
        case SENSOR_IAT_TYPE_BOSCH_816:
        resistor_init(&sensor_iat_resistor, 10000, RESISTOR_PULL_UP, ANALOG_INPUT_SENSOR_IAT_PIN);
        thermistor_conf_s bosch_816_iat_conf = 
        {
            .resistance_1 = 9395.0f,
            .resistance_2 = 2056.0f,
            .resistance_3 = 322.5f,
            .tempC_1 = -10.0f,
            .tempC_2 = 25.0f,
            .tempC_3 = 80.0f
        };
        thermistor_init(&sensor_iat, bosch_816_iat_conf);
        break;
    
    default:
        log_error("iat sensor type not configured!");
        break;
    }
}

temperature_t sensor_iat_get(void)
{
    temperature_t temperature = SENSOR_IAT_FAIL_SAFE;
    switch (configuration.iat_sensor_type)
    {
    case SENSOR_IAT_TYPE_BOSCH_816:
        temperature = thermistor_get_temp(&sensor_iat);
        break;
    
    default:
        temperature = SENSOR_IAT_FAIL_SAFE;
        log_warning("iat sensor type not configured!");
        break;
    }
    return temperature;
}

// Section for Manifold Absolute Pressure (MAP) Sensor
static sensor_map_calib_s map_calib = {0};

void sensor_map_init(void)
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

pressure_t sensor_map_get(void)
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

// Section for Oil Pressure Sensor (OPS)
bool sensor_ops_get(void)
{
    static bool state = false;
    static const uint16_t threshold_high = 2500;
    static const uint16_t threshold_low = 1800;

    uint16_t raw_adc_value = 0;
    raw_adc_value = analog_inputs_get_data(ANALOG_INPUT_SENSOR_OIL_PIN);

    if (raw_adc_value > threshold_high)
    {
        state = true;
    }
    else if (raw_adc_value < threshold_low)
    {
        state = false;
    }

    return state;
}

// Section for Throttle Position Sensor (TPS)
/**
 * @todo implement fail safe so that it uses the other sensor input when the main one fails
 */
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
        log_error("tps out of range!");
        return SENSOR_TPS_FAIL_SAFE;
    }
    return result;
}
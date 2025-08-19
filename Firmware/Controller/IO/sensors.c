#include "sensors.h"

static sensor_map_t *map_sensor = NULL;
static thermistor_t *sensor_iat = NULL;
static thermistor_t *sensor_clt = NULL;
static sensor_ops_t *sensor_ops = NULL;

percent_t sensor_tps_get(sensor_tps_t *sensor)
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
    result = CLAMP(result, (percent_t)0, (percent_t)100);
    return result;
}

void sensor_map_init(sensor_map_t *sensor, sensor_map_type_t type)
{
    if (sensor == NULL)
    {
        log_error("map sensor is null");
        return;
    }
    switch (type)
    {
    case SENSOR_MAP_TYPE_BOSCH_816:
        /**
         * a simple linear sensor
         */
        /**
         * @todo actually calculate these values!
         */
        sensor->adc_value_0_bar = 100;
        sensor->adc_value_1_bar = 3000;
        sensor->analog_channel = ANALOG_INPUT_SENSOR_MAP_PIN;
        break;
    case SENSOR_MAP_TYPE_TEST:
        /**
         * a simple linear sensor
         */
        /**
         * @todo actually calculate these values!
         */
        sensor->adc_value_0_bar = 0;
        sensor->adc_value_1_bar = 4095;
        sensor->analog_channel = ANALOG_INPUT_SENSOR_MAP_PIN;
        break;
    
    default:
        log_error("map sensor type not configured!");
        break;
    }
    map_sensor = sensor;
}


pressure_t sensor_map_get()
{
    if (map_sensor->adc_value_0_bar == 0 && map_sensor->adc_value_1_bar == 0)
    {
        log_error("map sensor no init!");
        return SENSOR_MAP_FAIL_SAFE;
    }
    
    uint16_t raw_adc_value = 0;
    raw_adc_value = analog_inputs_get_data(map_sensor->analog_channel);

    pressure_t result = SENSOR_MAP_FAIL_SAFE;

    result = mapf((float)raw_adc_value, (float)map_sensor->adc_value_0_bar, (float)map_sensor->adc_value_1_bar, (pressure_t)0, (pressure_t)100);

    if (!IS_IN_RANGE(result, (pressure_t)0, FIRMWARE_LIMIT_MAX_MAP))
    {
        log_error("map sensor out of range!");
        return SENSOR_MAP_FAIL_SAFE;
    }
    return result;
}

void sensor_iat_init(thermistor_t *sensor, sensor_iat_type_t type)
{
    switch (type)
    {
        case SENSOR_IAT_TYPE_BOSCH_816:
            resistor_init(&sensor->resistor, 10000, RESISTOR_PULL_UP, ANALOG_INPUT_SENSOR_IAT_PIN);
            thermistor_conf_t bosch_816_iat_conf = 
            {
                .resistance_1 = 9395.0f,
                .resistance_2 = 2056.0f,
                .resistance_3 = 322.5f,
                .tempC_1 = -10.0f,
                .tempC_2 = 25.0f,
                .tempC_3 = 80.0f
            };
            thermistor_init(sensor, bosch_816_iat_conf);
            sensor_iat = sensor;
            break;
        case SENSOR_IAT_TYPE_TEST:
            resistor_init(&sensor->resistor, 4700, RESISTOR_PULL_UP, ANALOG_INPUT_SENSOR_IAT_PIN);
            thermistor_conf_t genric_5k =
            {
                .resistance_1 = 22.263e3f,
                .resistance_2 = 5e3f,
                .resistance_3 = 588.0f,
                .tempC_1 = -10.0f,
                .tempC_2 = 25.0f,
                .tempC_3 = 90.0f
            };
            thermistor_init(sensor, genric_5k);
            sensor_iat = sensor;
            break;
    default:
        log_error("iat sensor type not configured!");
        break;
    }
}

temperature_t sensor_iat_get()
{
    if (sensor_iat == NULL)
    {
        log_error("iat sensor is null");
        return SENSOR_IAT_FAIL_SAFE;
    }
    temperature_t temperature = thermistor_get_temp(sensor_iat);
    if (!IS_IN_RANGE(temperature, FIRMWARE_LIMIT_MIN_TEMP, FIRMWARE_LIMIT_MAX_TEMP))
    {
        log_error("iat sensor out of range!");
        return SENSOR_IAT_FAIL_SAFE;
    }
    return temperature;
}

void sensor_clt_init(thermistor_t *sensor, sensor_clt_type_t type)
{
    if (sensor == NULL)
    {
        log_error("clt sensor is null");
        return;
    }
    switch (type)
    {
    /* Actually make this a proper thing */
    case SENSOR_CLT_TYPE_NISSAN:
        resistor_init(&sensor->resistor, 10000, RESISTOR_PULL_UP, ANALOG_INPUT_SENSOR_CLT_PIN);
        thermistor_conf_t nissan_clt_conf =
        {
            .resistance_1 = 15700.0f,
            .resistance_2 = 2050.0f,
            .resistance_3 = 230.0f,
            .tempC_1 = -20.0f,
            .tempC_2 = 25.0f,
            .tempC_3 = 90.0f
        };
        thermistor_init(sensor, nissan_clt_conf);
        sensor_clt = sensor;
        break;
    case SENSOR_CLT_TYPE_TEST:
        resistor_init(&sensor->resistor, 4700, RESISTOR_PULL_UP, ANALOG_INPUT_SENSOR_CLT_PIN);
        thermistor_conf_t genric_5k =
        {
            .resistance_1 = 22.263e3f,
            .resistance_2 = 5e3f,
            .resistance_3 = 588.0f,
            .tempC_1 = -10.0f,
            .tempC_2 = 25.0f,
            .tempC_3 = 90.0f
        };
        thermistor_init(sensor, genric_5k);
        sensor_clt = sensor;
        break;
    default:
        log_error("clt sensor type not configured!");
        break;
    }
}

temperature_t sensor_clt_get()
{
    if (sensor_clt == NULL)
    {
        log_error("clt sensor is null");
        return SENSOR_CLT_FAIL_SAFE;
    }
    temperature_t temperature = thermistor_get_temp(sensor_clt);
    if (!IS_IN_RANGE(temperature, FIRMWARE_LIMIT_MIN_TEMP, FIRMWARE_LIMIT_MAX_TEMP))
    {
        log_error("clt sensor out of range!");
        return SENSOR_CLT_FAIL_SAFE;
    }
    return temperature;
}

void sensor_ops_init(sensor_ops_t *sensor)
{
    if (sensor == NULL)
    {
        log_error("ops sensor is null");
        return;
    }
    sensor->adc_value_threshold = 4095 / 2;
    sensor->debounce_time_ms = 200;
    sensor->analog_channel = ANALOG_INPUT_SENSOR_OIL_PIN;
    sensor_ops = sensor;
}

bool sensor_ops_get()
{
    static uint32_t last_change_time = 0;
    static bool last_state = false;
    static bool debounced_state = false;

    if (sensor_ops == NULL)
    {
        log_error("ops sensor is null");
        return false;
    }

    uint16_t adc_value = analog_inputs_get_data(sensor_ops->analog_channel);
    bool current_state = adc_value > sensor_ops->adc_value_threshold;

    uint32_t now = get_time_ms();

    if (current_state != last_state)
    {
        /* State changed, reset debounce timer */
        last_change_time = now;
        last_state = current_state;
    }

    if ((now - last_change_time) >= sensor_ops->debounce_time_ms)
    {
        /* Debounce period passed, accept new state */
        debounced_state = current_state;
    }

    return debounced_state;
}

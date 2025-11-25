#include "sensors.h"

static sensor_map_t *map_sensor = NULL;
static thermistor_t *sensor_iat = NULL;
static thermistor_t *sensor_clt = NULL;


void sensor_tps_init(sensor_tps_t *sensor, status_t status_flag)
{
    if (sensor == NULL)
    {
        log_error("tps sensor is null");
        return;
    }

    sensor_tps_type_t sensor_type = 10;
    if (status_flag == STATUS_TPS1_ERROR)
    {
        sensor_type = config.tps1_type;
    }
    else if (status_flag == STATUS_TPS2_ERROR)
    {
        sensor_type = config.tps2_type;
    }

    switch (sensor_type)
    {
    case SENSOR_TPS_TYPE_SAMAND_ETB:
        /**
         * a simple linear sensor
         */
        if (status_flag == STATUS_TPS1_ERROR)
        {
            sensor->config.analog_channel = ANALOG_INPUT_ETB1_SENSE1;
            sensor->config.analog_channel_backup = ANALOG_INPUT_ETB1_SENSE2;
            sensor->config.is_backup_channel_enabled = true;
            sensor->config.is_inverted = false;
            sensor->config.status_bit = status_flag;
            sensor->config.fully_closed_adc_value = 5;
            sensor->config.fully_open_adc_value = 4090;
            change_bit(&runtime.status, status_flag, false);
        }
        else if (status_flag == STATUS_TPS2_ERROR)
        {
            sensor->config.analog_channel = ANALOG_INPUT_ETB2_SENSE1;
            sensor->config.analog_channel_backup = ANALOG_INPUT_ETB2_SENSE2;
            sensor->config.is_backup_channel_enabled = true;
            sensor->config.is_inverted = false;
            sensor->config.status_bit = status_flag;
            sensor->config.fully_closed_adc_value = 5;
            sensor->config.fully_open_adc_value = 4090;
            change_bit(&runtime.status, status_flag, false);
        }

        break;
    case SENSOR_TPS_CUSTOM:
        /**
         * a simple linear sensor
         */
        if (status_flag == STATUS_TPS1_ERROR)
        {
            sensor->config.analog_channel = ANALOG_INPUT_ETB1_SENSE1;
            sensor->config.analog_channel_backup = ANALOG_INPUT_ETB1_SENSE2;
            sensor->config.is_backup_channel_enabled = false;
            sensor->config.is_inverted = config.tps1_calib_is_inverted;
            sensor->config.status_bit = status_flag;

            sensor->config.fully_closed_adc_value = config.tps1_calib_closed_throttle_adc_value;
            sensor->config.fully_open_adc_value = config.tps1_calib_wide_open_throttle_adc_value;
            change_bit(&runtime.status, status_flag, false);
        }
        else if (status_flag == STATUS_TPS2_ERROR)
        {
            sensor->config.analog_channel = ANALOG_INPUT_ETB2_SENSE1;
            sensor->config.analog_channel_backup = ANALOG_INPUT_ETB2_SENSE2;
            sensor->config.is_backup_channel_enabled = false;
            sensor->config.is_inverted = config.tps2_calib_is_inverted;
            sensor->config.status_bit = status_flag;

            sensor->config.fully_closed_adc_value = config.tps2_calib_closed_throttle_adc_value;
            sensor->config.fully_open_adc_value = config.tps2_calib_wide_open_throttle_adc_value;
            change_bit(&runtime.status, status_flag, false);
        }

        break;

    default:

        break;
    }
}

percent_t sensor_tps_get(sensor_tps_t *sensor)
{
    if (sensor == NULL)
    {
        return SENSOR_TPS_FAIL_SAFE;
    }
    // Disabling this check because it will always fail at the start when prev time is zero
    // time_us_t current_time = get_time_us();

    // if ((int32_t)(current_time - sensor->prev_position_time) > SENSOR_TPS_OLD_DATA_THRESHOLD)
    // {
    //     change_bit(&runtime.status, sensor->config.status_bit, true);
    //     return SENSOR_TPS_FAIL_SAFE;
    // }

    return sensor->current_position;
}

percent_t sensor_tps_rate_of_change_get(sensor_tps_t *sensor)
{
    if (sensor == NULL)
    {
        return SENSOR_TPS_FAIL_SAFE;
    }
    return sensor->current_rate_of_change;
}

void sensor_tps_update(sensor_tps_t *sensor, time_us_t current_time)
{
    if (sensor == NULL)
    {
        log_error("tps is null");
        return;
    }
    if (sensor->config.fully_open_adc_value == 0 && sensor->config.fully_closed_adc_value == 0)
    {
        change_bit(&runtime.status, sensor->config.status_bit, true);
        sensor->current_position = SENSOR_TPS_FAIL_SAFE;
        sensor->current_rate_of_change = SENSOR_TPS_FAIL_SAFE;
        return;
    }
    percent_t result = 0;
    uint16_t raw_data = analog_inputs_get_data(sensor->config.analog_channel);
    if (sensor->config.is_inverted)
    {
        result = mapf((float)raw_data, (float)sensor->config.fully_open_adc_value, (float)sensor->config.fully_closed_adc_value, (float)0, (float)100);
    }
    else
    {
        result = mapf((float)raw_data, (float)sensor->config.fully_closed_adc_value, (float)sensor->config.fully_open_adc_value, (float)0, (float)100);
    }
    sensor->current_position = result;

    time_us_t delta_time = current_time - sensor->prev_position_time;
    percent_t delta_position = sensor->current_position - sensor->prev_position;
    sensor->prev_position_time = current_time;
    sensor->prev_position = sensor->current_position;

    if (delta_time == 0)
    {
        sensor->current_rate_of_change = 0;
    }
    else
    {
        sensor->current_rate_of_change = delta_position / (float)delta_time * (float)CONVERSION_FACTOR_SECONDS_TO_MICROSECONDS;
    }

    change_bit(&runtime.status, sensor->config.status_bit, false);
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
        sensor->config.adc_value_0_bar = 100;
        sensor->config.adc_value_1_bar = 3000;
        sensor->config.analog_channel = ANALOG_INPUT_SENSOR_MAP_PIN;
        change_bit(&runtime.status, STATUS_MAP_ERROR, false);
        break;
    case SENSOR_MAP_TYPE_TEST:
        /**
         * a simple linear sensor
         */
        /**
         * @todo actually calculate these values!
         */
        sensor->config.adc_value_0_bar = 0;
        sensor->config.adc_value_1_bar = 4095;
        sensor->config.analog_channel = ANALOG_INPUT_SENSOR_MAP_PIN;
        change_bit(&runtime.status, STATUS_MAP_ERROR, false);
        break;

    default:
        // log_error("map sensor type not configured!");
        change_bit(&runtime.status, STATUS_MAP_ERROR, true);
        break;
    }

    map_sensor = sensor;
}

void sensor_map_update(time_us_t time)
{
    if (map_sensor == NULL)
    {
        log_error("map sensor no init!");
        return;
    }

    if (map_sensor->config.adc_value_0_bar == 0 && map_sensor->config.adc_value_1_bar == 0)
    {
        change_bit(&runtime.status, STATUS_MAP_ERROR, true);
        log_error("map sensor no init!");
        map_sensor->current_map = SENSOR_MAP_FAIL_SAFE;
        map_sensor->current_map_rate_of_change = SENSOR_MAP_FAIL_SAFE;
        return;
    }

    uint16_t raw_adc_value = 0;
    raw_adc_value = analog_inputs_get_data(map_sensor->config.analog_channel);

    pressure_t read_map = SENSOR_MAP_FAIL_SAFE;

    read_map = mapf((float)raw_adc_value, (float)map_sensor->config.adc_value_0_bar, (float)map_sensor->config.adc_value_1_bar, (pressure_t)0, (pressure_t)100);

    pressure_t rate_of_change = rate_of_change_per_sec(read_map, map_sensor->prev_map, time, map_sensor->prev_time);

    map_sensor->current_map = read_map;
    map_sensor->current_map_rate_of_change = rate_of_change;

    map_sensor->prev_map = read_map;
    map_sensor->prev_time = time;

    if (!IS_IN_RANGE(read_map, (pressure_t)0, FIRMWARE_LIMIT_MAX_MAP))
    {
        change_bit(&runtime.status, STATUS_MAP_ERROR, true);
    }
    else
    {
        change_bit(&runtime.status, STATUS_MAP_ERROR, false);
    }
}

pressure_t sensor_map_get()
{
    if (map_sensor == NULL)
    {
        change_bit(&runtime.status, STATUS_MAP_ERROR, true);
        log_error("map sensor no init!");
        return SENSOR_MAP_FAIL_SAFE;
    }
    return map_sensor->current_map;
}
pressure_t sensor_map_rate_of_change_get()
{
    if (map_sensor == NULL)
    {
        change_bit(&runtime.status, STATUS_MAP_ERROR, true);
        log_error("map sensor no init!");
        return SENSOR_MAP_FAIL_SAFE;
    }
    return map_sensor->current_map_rate_of_change;
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
                .tempC_3 = 80.0f};
        thermistor_init(sensor, bosch_816_iat_conf);
        sensor_iat = sensor;
        change_bit(&runtime.status, STATUS_IAT_ERROR, false);

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
                .tempC_3 = 90.0f};
        thermistor_init(sensor, genric_5k);
        sensor_iat = sensor;
        change_bit(&runtime.status, STATUS_IAT_ERROR, false);

        break;
    default:
        // log_error("iat sensor type not configured!");
        change_bit(&runtime.status, STATUS_IAT_ERROR, true);
        break;
    }
}
void sensor_iat_update()
{
    if (sensor_iat == NULL)
    {
        log_error("iat sensor is null");
        change_bit(&runtime.status, STATUS_IAT_ERROR, true);
        runtime.iat_degc = SENSOR_IAT_FAIL_SAFE;
        return;
    }

    temperature_t temperature = thermistor_get_temp(sensor_iat);
    if (!IS_IN_RANGE(temperature, FIRMWARE_LIMIT_MIN_TEMP, FIRMWARE_LIMIT_MAX_TEMP))
    {
        // log_error("iat sensor out of range!");
        change_bit(&runtime.status, STATUS_IAT_ERROR, true);

        runtime.iat_degc = SENSOR_IAT_FAIL_SAFE;
        return;
    }
    change_bit(&runtime.status, STATUS_IAT_ERROR, false);
    runtime.iat_degc = temperature;
}

temperature_t sensor_iat_get()
{
    if (sensor_iat == NULL)
    {
        log_error("iat sensor is null");
        change_bit(&runtime.status, STATUS_IAT_ERROR, true);
        return SENSOR_IAT_FAIL_SAFE;
    }
    return runtime.iat_degc;
}

void sensor_clt_init(thermistor_t *sensor, sensor_clt_type_t type)
{
    if (sensor == NULL)
    {
        log_error("clt sensor is null");
        change_bit(&runtime.status, STATUS_CLT_ERROR, true);
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
                .tempC_3 = 90.0f};
        thermistor_init(sensor, nissan_clt_conf);
        sensor_clt = sensor;
        change_bit(&runtime.status, STATUS_CLT_ERROR, false);
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
                .tempC_3 = 90.0f};
        thermistor_init(sensor, genric_5k);
        sensor_clt = sensor;
        change_bit(&runtime.status, STATUS_CLT_ERROR, false);
        break;
    default:
        // log_error("clt sensor type not configured!");
        change_bit(&runtime.status, STATUS_CLT_ERROR, true);
        break;
    }
}
void sensor_clt_update()
{
    if (sensor_clt == NULL)
    {
        change_bit(&runtime.status, STATUS_CLT_ERROR, true);
        log_error("clt sensor is null");
        runtime.clt_degc = SENSOR_CLT_FAIL_SAFE;
        return;
    }

    temperature_t temperature = thermistor_get_temp(sensor_clt);
    if (!IS_IN_RANGE(temperature, FIRMWARE_LIMIT_MIN_TEMP, FIRMWARE_LIMIT_MAX_TEMP))
    {
        change_bit(&runtime.status, STATUS_CLT_ERROR, true);
        // log_error("clt sensor out of range!");
        runtime.clt_degc = SENSOR_CLT_FAIL_SAFE;
        return;
    }
    change_bit(&runtime.status, STATUS_CLT_ERROR, false);
    runtime.clt_degc = temperature;
}
temperature_t sensor_clt_get()
{
    if (sensor_clt == NULL)
    {
        change_bit(&runtime.status, STATUS_CLT_ERROR, true);
        log_error("clt sensor is null");
        return SENSOR_CLT_FAIL_SAFE;
    }
    return runtime.clt_degc;
}

void sensor_ops_update(time_us_t time)
{
    const uint16_t debounce_time_ms = 200;
    static uint32_t last_change_time = 0;
    static bool last_state = false;
    static bool debounced_state = false;

    uint16_t adc_value = analog_inputs_get_data(ANALOG_INPUT_SENSOR_OIL_PIN);
    bool current_state = adc_value > 2047;

    uint32_t now = time;

    if (current_state != last_state)
    {
        /* State changed, reset debounce timer */
        last_change_time = now;
        last_state = current_state;
    }

    if ((now - last_change_time) >= debounce_time_ms)
    {
        /* Debounce period passed, accept new state */
        debounced_state = current_state;
    }

    change_bit(&runtime.status, STATUS_OIL_PRESSURE_LOW, !debounced_state);
}

bool sensor_ops_get()
{
    return !get_bit(runtime.status, STATUS_OIL_PRESSURE_LOW);
}

void sensor_egt_update()
{
    uint16_t adc_value = analog_inputs_get_data(ANALOG_INPUT_SENSOR_EGT);
    /*
     * Type K thermocouple: ~41uV/°C
     * With gain 100: 4.1mV/°C
     * ADC: 12-bit, 3.3V ref => 3.3V/4095 ≈ 0.000805V/LSB
     * So, each °C = 4.1mV / 0.000805V ≈ 5.09 ADC counts/°C
     * Temperature (°C) = (adc_value * 3.3 / 4095) / 0.0041
     */
    voltage_t voltage = ((float)adc_value * ADC_REF_VOLTAGE) / ADC_MAX_VALUE;
    temperature_t temperature = (temperature_t)(voltage / SENSOR_EGT_THERMOCOUPLE_CONSTANT);
    runtime.egt_degc = temperature;
}
temperature_t sensor_egt_get()
{
    return runtime.egt_degc;
}

void vbat_update()
{
    uint16_t adc_value = analog_inputs_get_data(ANALOG_INPUT_VBAT_SENSE_PIN);
    voltage_t voltage = ((float)adc_value * ADC_REF_VOLTAGE) / ADC_MAX_VALUE * VBAT_DIVIDER_RATIO;
    runtime.vbatt_volts = voltage;
}
voltage_t vbat_get()
{
    return runtime.vbatt_volts;
}

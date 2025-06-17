#ifndef SENSORS_H
#define SENSORS_H

#include "math.h"
#include "analog_inputs.h"
#include "resistor.h"
#include "thermistor.h"
#include "error_handling.h"
#include "utils.h"
#include "constants.h"
#include "timing.h"
#include "types.h"

#define SENSOR_TPS_FAIL_SAFE (percent_t) NAN
#define SENSOR_MAP_FAIL_SAFE (pressure_t)0
#define SENSOR_IAT_FAIL_SAFE (temperature_t)40
#define SENSOR_CLT_FAIL_SAFE (temperature_t)30


typedef struct
{
    uint16_t wide_open_throttle_adc_value;
    uint16_t closed_throttle_adc_value;
    analog_input_channel_t analog_channel;
    bool is_inverted;
} sensor_tps_t;

typedef struct
{
    uint16_t adc_value_0_bar;
    uint16_t adc_value_1_bar;
    analog_input_channel_t analog_channel;
} sensor_map_t;

typedef struct
{
    uint16_t adc_value_threshold;
    uint16_t debounce_time_ms;
    analog_input_channel_t analog_channel;
} sensor_ops_t;


percent_t sensor_tps_get(sensor_tps_t *sensor);

void sensor_map_init(sensor_map_t *sensor, sensor_map_type_t type);
pressure_t sensor_map_get();

void sensor_iat_init(thermistor_t *sensor, sensor_iat_type_t type);
temperature_t sensor_iat_get();

void sensor_clt_init(thermistor_t *sensor, sensor_clt_type_t type);
temperature_t sensor_clt_get();

void sensor_ops_init(sensor_ops_t *sensor);
bool sensor_ops_get();


#endif // SENSORS_H
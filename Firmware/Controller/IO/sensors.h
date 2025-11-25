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
#include "config_and_runtime.h"

#define SENSOR_TPS_FAIL_SAFE (percent_t) NAN
#define SENSOR_TPS_OLD_DATA_THRESHOLD (time_us_t)10000
#define SENSOR_MAP_FAIL_SAFE (pressure_t)0
#define SENSOR_IAT_FAIL_SAFE (temperature_t)40
#define SENSOR_CLT_FAIL_SAFE (temperature_t)60
#define SENSOR_EGT_THERMOCOUPLE_CONSTANT (float)41e-6

typedef struct
{
    uint16_t fully_open_adc_value;
    uint16_t fully_closed_adc_value;
    analog_input_channel_t analog_channel;
    analog_input_channel_t analog_channel_backup;
    status_t status_bit;
    bool is_backup_channel_enabled;
    bool is_inverted;
} sensor_tps_config_t;

typedef struct
{
    percent_t current_position;
    percent_t current_rate_of_change;
    percent_t prev_position;
    time_us_t prev_position_time;
    sensor_tps_config_t config;
} sensor_tps_t;

typedef struct
{
        uint16_t adc_value_0_bar;
    uint16_t adc_value_1_bar;
    analog_input_channel_t analog_channel;
} sensor_map_config_t;


typedef struct
{
    pressure_t current_map;
    pressure_t current_map_rate_of_change;
    pressure_t prev_map;
    time_us_t  prev_time;
    sensor_map_config_t config;
} sensor_map_t;




void sensor_tps_init(sensor_tps_t *sensor, status_t status_flag);
void sensor_tps_update(sensor_tps_t *sensor, time_us_t current_time);


percent_t sensor_tps_get(sensor_tps_t *sensor);
percent_t sensor_tps_rate_of_change_get(sensor_tps_t *sensor);

void sensor_map_init(sensor_map_t *sensor, sensor_map_type_t type);
void sensor_map_update(time_us_t time);
pressure_t sensor_map_get();
pressure_t sensor_map_rate_of_change_get();

void sensor_iat_init(thermistor_t *sensor, sensor_iat_type_t type);
void sensor_iat_update();
temperature_t sensor_iat_get();

void sensor_clt_init(thermistor_t *sensor, sensor_clt_type_t type);
void sensor_clt_update();
temperature_t sensor_clt_get();

void sensor_ops_update(time_us_t time);
bool sensor_ops_get();

void sensor_egt_update();
temperature_t sensor_egt_get();

void vbat_update();
voltage_t vbat_get();


#endif // SENSORS_H
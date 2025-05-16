/**
 * @file tps.h
 * @brief Header file for the Throttle Position Sensor (TPS) interface.
 *
 * This file contains the declarations and definitions required to interface
 * with the Throttle Position Sensor (TPS) in the system.
 *
 * @author Alireza Eskandari
 */
#ifndef TPS_H
#define TPS_H

/**
 * @todo implement fail safe so that it usues the other sensor input when the main one fails
 */

#include "analog_inputs.h"
#include "types.h"

#define SENSOR_TPS_FAIL_SAFE (percent_t) NAN

typedef struct
{
    uint16_t wide_open_throttle_adc_value;
    uint16_t closed_throttle_adc_value;
    analog_input_adc_channel_mapping_e analog_channel;
    bool is_inverted;
} tps_sensor_calib_s;

typedef struct
{
    tps_sensor_calib_s *calib;
    bool is_initialized;
} tps_sensor_s;



void sensor_tps_init(tps_sensor_s *sensor, tps_sensor_calib_s *sensor_calib);

percent_t sensor_tps_get(tps_sensor_s *sensor);


#endif // TPS_H
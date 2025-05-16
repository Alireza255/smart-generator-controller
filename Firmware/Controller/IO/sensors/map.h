/*
 * File:        map.h
 * Author:      Alireza Eskandari
 * Created:     2025-05-11
 * Description: Header file for Manifold Absolute Pressure (MAP) sensor interface.
 *              Provides functions and definitions for reading and processing data
 *              from the MAP sensor.
 */

#ifndef MAP_H
#define MAP_H

#include "types.h"
#include "controller.h"
#include "error_handling.h"


typedef struct
{
    uint16_t adc_value_0_bar;
    uint16_t adc_value_1_bar;
} sensor_map_calib_s;

#define SENSOR_MAP_FAIL_SAFE (pressure_t)0

void sensor_map_init();

pressure_t sensor_map_get();

#endif // MAP_H

/*
 * File:        clt.h
 * Author:      Alireza Eskandari
 * Created:     2025-05-11
 * Description: Header file for Coolant Temperature (CLT) sensor interface.
 *              Provides functions and definitions for reading and processing data
 *              from the CLT sensor.
 */
#ifndef CLT_H
#define CLT_H

#include "thermistor.h"
#include "controller.h"

#define SENSOR_CLT_FAIL_SAFE (temperature_t)30


void sensor_clt_init();

temperature_t sensor_clt_get();

#endif // CLT_H

/*
 * File:        iat.h
 * Author:      Alireza Eskandari
 * Created:     2025-05-11
 * Description: Header file for Intake Air Temperature (IAT) sensor interface.
 *              Provides functions and definitions for reading and processing data
 *              from the IAT sensor.
 */
#ifndef IAT_H
#define IAT_H

#include "thermistor.h"
#include "controller.h"

#define SENSOR_IAT_FAIL_SAFE (temperature_t)40


void sensor_iat_init();

temperature_t sensor_iat_get();



#endif // IAT_H

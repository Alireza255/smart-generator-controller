/**
 * @file tps.h
 * @brief Header file for the Gas valve position sensor (GVPS) interface.
 *
 * This file contains the declarations and definitions required to interface
 * with the Gas valve position sensor(GVPS) in the system.
 *
 * @author Alireza Eskandari
 */
#ifndef GVPS_H
#define GVPS_H

#include "controller.h"
#include "tps.h"

#define SENSOR_GVPS_FAIL_SAFE (percent_t) NAN

void sensor_gvps_init();

percent_t sensor_gvps_get();


#endif // GVPS_H
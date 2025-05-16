/*
 * File:        ops.h
 * Author:      Alireza Eskandari
 * Created:     2025-05-11
 * Description: Header file for oil pressure sensor interface.
 */
#ifndef OPS_H
#define OPS_H

#include "analog_inputs.h"

/**
 * @brief returns the oil pressure sensor result  true if good and false if no good!
 * @return true if oil pressure is good and false if no good!
 */
bool sensor_ops_get();

#endif // OPS_H

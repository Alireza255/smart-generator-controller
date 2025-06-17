#ifndef SENSORS_H
#define SENSORS_H

// Merged sensor library declarations

// Dependencies for all sensors
#include "thermistor.h"
//#include "controller.h"
#include "types.h"
#include "error_handling.h"
#include "analog_inputs.h"


void sensor_clt_init(void);
temperature_t sensor_clt_get(void);


void sensor_iat_init(void);
temperature_t sensor_iat_get(void);



void sensor_map_init(void);
pressure_t sensor_map_get(void);

// Section for Oil Pressure Sensor (OPS)
/**
 * @brief returns the oil pressure sensor result  true if good and false if no good!
 * @return true if oil pressure is good and false if no good!
 */
bool sensor_ops_get(void);


percent_t sensor_tps_get(sensor_tps_s *sensor);

// Section for additional sensor declarations
// (To be populated once additional sensor .h files are provided)

#endif // SENSORS_H
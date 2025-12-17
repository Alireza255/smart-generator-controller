#ifndef CONTROLLER_H
#define CONTROLLER_H



#include "timing.h"
#include "types.h"
#include "comms.h"
#include "trigger.h"
#include "dc_motors.h"
#include "tables.h"
#include "sensors.h"
#include "ignition.h"
#include "thermistor.h"
#include "pid.h"
#include "electronic_throttle.h"
#include "adc.h"
#include "governer.h"
#include "fan_control.h"
#include "trigger_simulator.h"
#include "config_and_runtime.h"
#include "ee.h"
#include "gas_injection.h"
#include "fuel_math.h"
#include "injection.h"
#include "outputs.h"
#include "fuel_math.h"
#include "timing_light.h"
#include "trigger_logger.h"
#include "protection.h"

extern config_t config;
extern runtime_t runtime;

extern sensor_tps_t tps1;
extern sensor_tps_t tps2;



void controller_init();

void controller_load_configuration();

bool controller_save_configuration();

void controller_configuration_load_default();

#endif // CONTROLLER_H

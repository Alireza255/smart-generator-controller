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

extern trigger_t trigger1;

void controller_init();

void controller_load_configuration();

bool controller_save_configuration();

void controller_load_test_configuration();

#endif // CONTROLLER_H

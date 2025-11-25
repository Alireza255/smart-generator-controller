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

extern config_t config;
extern runtime_t runtime;

extern sensor_tps_t tps1;
extern sensor_tps_t tps2;

static electronic_throttle_t etb1;
static dc_motor_t etb1_motor;
static pid_t etb1_pid;

static electronic_throttle_t etb2;
static dc_motor_t etb2_motor;
static pid_t etb2_pid;


static thermistor_t sensor_clt;
static thermistor_t sensor_iat;


void controller_init();

void controller_load_configuration();

bool controller_save_configuration();

void controller_load_test_configuration();


#endif // CONTROLLER_H

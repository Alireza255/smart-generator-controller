#ifndef IGNITION_H
#define IGNITION_H

#include "controller.h"
#include "main.h"
#include "utils.h"
#include "stm32f4xx_hal.h"
#include "iat.h"
#include "map.h"
#include "clt.h"

#define TEST_MODE

#define IGNITION_MAX_OUTPUTS 4
#define IGNITION_MIN_DWELL_TIME_MS 0.5f
#define IGNITION_MAX_DWELL_TIME_MS 4.0f
#define IGNITION_MULTI_SPARK_MAX_SPARKS 8

#define IGNITION_MIN_ADVANCE (angle_t)0
#define IGNITION_MAX_ADVANCE (angle_t)40


typedef struct
{
    GPIO_TypeDef    *gpio;
    uint32_t         pin;
} ignition_output_pin_s;

typedef struct
{
    ignition_output_pin_s output[IGNITION_MAX_OUTPUTS];
} ignition_output_conf_s;

typedef enum
{
    IGNITION_COIL_STATE_NOT_CHARGING = 0,
    IGNITION_COIL_STATE_CHARGING = 1,
} ignition_coil_state_e;

void ignition_init(ignition_output_conf_s *output_conf);

/**
 * @brief Handles an ignition trigger event based on the crankshaft angle, RPM, and current time.
 * 
 * @param crankshaft_angle The current angle of the crankshaft.
 * @param rpm The current revolutions per minute of the engine.
 * @param current_time_us The current time in microseconds.
 */
void ignition_trigger_event_handle(angle_t crankshaft_angle, rpm_t rpm, time_us_t current_time_us);

/**
 * @brief Gets the number of sparks per coil based on the ignition mode.
 * 
 * @param ignition_mode The mode of ignition, specified as an enumeration of type ignition_mode_e.
 * @return The number of sparks per coil.
 */
uint8_t ignition_get_number_of_sparks_per_coil(ignition_mode_e ignition_mode);

/**
 * @brief Charges the ignition coil at the specified index.
 * 
 * @param coil_index The index of the coil to be charged.
 */
void ignition_coil_begin_charge(void *arg);

/**
 * @brief Fires a spark from the ignition coil at the specified index.
 * 
 * @param coil_index The index of the coil to fire the spark from.
 */
void ignition_coil_fire_spark(void *arg);

/**
 * @brief Gets the duty cycle of the ignition coil as a percentage.
 * 
 * @return The duty cycle of the ignition coil.
 */
percent_t ignition_get_coil_duty_cycle();

angle_t ignition_get_advance();

#endif // IGNITION_H

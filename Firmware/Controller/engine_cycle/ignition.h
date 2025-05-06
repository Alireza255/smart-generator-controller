#ifndef IGNITION_H
#define IGNITION_H

#include "controller.h"
#include "main.h"
#include "utils.h"

#include "configuration.h"

#define IGNITION_MAX_ALLOWED_OUTPUTS 6
#define IGNITION_MIN_DWELL_TIME_MS 0.5f
#define IGNITION_MAX_DWELL_TIME_MS 4.0f
#define IGNITION_MULTI_SPARK_MAX_SPARKS 8

typedef struct
{
    /**
     * this means that all ignition outputs have to be from the same GPIO
     * this might change in the future
     */
    GPIO_TypeDef    *gpio;
    uint32_t        output[IGNITION_MAX_ALLOWED_OUTPUTS];
} ignition_output_pins_s;

/**
 * @brief Initializes the ignition system with the specified parameters.
 * 
 * @param htim Pointer to a TIM_HandleTypeDef structure that contains
 *             the configuration information for the TIM peripheral.
 * @param ignition_mode The mode of ignition to be used.
 * @param pin Pointer to a structure of type ignition_output_pins_s
 *            that defines the output pins for the ignition system.
 * @note a 32 bit timer is preferd
 * @note interrupts must be enabled
 */
void ignition_init(TIM_HandleTypeDef *htim, ignition_mode_e ignition_mode, ignition_output_pins_s *pin);

/**
 * @brief Handles an ignition trigger event based on the crankshaft angle, RPM, and current time.
 * 
 * @param crankshaft_angle The current angle of the crankshaft.
 * @param rpm The current revolutions per minute of the engine.
 * @param current_time_us The current time in microseconds.
 */
void ignition_trigger_event_handle(angle_t crankshaft_angle, rpm_t rpm, time_us_t current_time_us);

/**
 * @brief Schedules the next spark event for the ignition system.
 */
void ignition_schedule_spark();

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
void ignition_coil_charge(uint8_t coil_index);

/**
 * @brief Fires a spark from the ignition coil at the specified index.
 * 
 * @param coil_index The index of the coil to fire the spark from.
 */
void ignition_coil_fire_spark(uint8_t coil_index);

/**
 * @brief Gets the duty cycle of the ignition coil as a percentage.
 * 
 * @return The duty cycle of the ignition coil.
 */
percent_t ignition_get_coil_duty_cycle();


#endif // IGNITION_H

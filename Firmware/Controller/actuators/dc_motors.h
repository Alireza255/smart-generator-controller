#ifndef DC_MOTORS_H
#define DC_MOTORS_H

#include "types.h"
#include "main.h"

#define DC_MOTOR_MAX_FREQUENCY 10000U

typedef enum
{
    MS_NORMAL,
    MS_FAULT,
    MS_DISABLED,
} dc_motor_status_e;

typedef enum
{
    MD_FORWARD = 0,
    MD_REVERSE = 1,
} dc_motor_direction_e;

typedef struct
{
    TIM_HandleTypeDef *timer;
    uint32_t timer_channel_output_1;
    uint32_t timer_channel_output_2;
    dc_motor_direction_e current_direction;
    uint8_t current_duty_cycle;
    dc_motor_status_e status;

} dc_motor_s;


/**
 * @brief inits the dc motor
 * @todo complete documentation
 * @param motor pointer to the dc_motor_s
 * @param timer pointer to timer handle - the timer connected to the motor
 * @param timer_channel_output_1 which timer channel is connected to the first input of the driver
 * @param timer_channel_output_2 which timer channel is connected to the second input of the driver
 * @param frequency the desired pwm frequency
 */
void dc_motor_init(dc_motor_s *motor, TIM_HandleTypeDef *timer, uint32_t timer_channel_output_1, uint32_t timer_channel_output_2, pwm_freq_t frequency);

/**
 * @brief sets the pwm duty cycle of the motor
 * @param timer pointer to timer handle - the timer connected to the motor
 * @param dir the desired direction of the motor
 * @param duty_cycle a number between 0 and 255 not a percentage
 */
void dc_motor_set(dc_motor_s *motor,dc_motor_direction_e dir, uint8_t duty_cycle);

/**
 * @brief sets the pwm frequency
 * @note this changes the frequency of the entire timer, this includes all four channels
 */
void dc_motor_set_timer_freq(dc_motor_s *motor, pwm_freq_t frequency);

/**
 * @brief disables the motor.
 */
void dc_motor_disable(dc_motor_s *motor);

/**
 * @brief enables the motor.
 * @note sets the duty cycle to 0 for safety
 */
void dc_motor_enable(dc_motor_s *motor);

#endif // DC_MOTORS_H
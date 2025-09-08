#ifndef DC_MOTORS_H
#define DC_MOTORS_H

#include "types.h"
#include "main.h"
#include "utils.h"

#define DC_MOTOR_MAX_FREQUENCY 20000U
#define DC_MOTOR_MIN_FREQUENCY 1000U

/**
 * @todo add a code to produce a sound using the motor coils when
 * init happens succesfully
 */
typedef enum
{
    MOTOR_STATE_NORMAL,
    MOTOR_STATE_FAULT,
    MOTOR_STATE_DISABLED,
} dc_motor_status_t;

typedef enum
{
    MOTOR_DIRECTION_FORWARD = 0,
    MOTOR_DIRECTION_REVERSE = 1,
} dc_motor_direction_t;

typedef struct
{
    TIM_HandleTypeDef *timer;
    uint32_t timer_channel_output_1;
    uint32_t timer_channel_output_2;
    uint32_t timer_channel_output_3;
    uint32_t timer_channel_output_4;
    dc_motor_direction_t current_direction;
    uint8_t current_duty_cycle;
    dc_motor_status_t status;
} dc_motor_t;


/**
 * @brief inits the dc motor
 * @todo complete documentation
 * @param motor pointer to the dc_motor_t
 * @param timer pointer to timer handle - the timer connected to the motor
 * @param timer_channel_output_1 which timer channel is connected to the first input of the driver
 * @param timer_channel_output_2 which timer channel is connected to the second input of the driver
 * @param frequency the desired pwm frequency
 */
bool dc_motor_init(dc_motor_t *motor, TIM_HandleTypeDef *timer, uint32_t timer_channel_output_1, uint32_t timer_channel_output_2, pwm_freq_t frequency);
bool dc_motor_init_simple_Hbridge(dc_motor_t *motor, TIM_HandleTypeDef *timer, uint32_t timer_channel_output_1, uint32_t timer_channel_output_2, uint32_t timer_channel_output_3, uint32_t timer_channel_output_4, pwm_freq_t frequency);

/**
 * @brief sets the pwm duty cycle of the motor
 * @param timer pointer to timer handle - the timer connected to the motor
 * @param dir the desired direction of the motor
 * @param duty_cycle a number between 0 and 255 not a percentage
 */
bool dc_motor_set(dc_motor_t *motor, dc_motor_direction_t dir, uint8_t duty_cycle);

/**
 * @brief sets the pwm frequency
 * @note this changes the frequency of the entire timer, this includes all four channels
 */
bool dc_motor_set_timer_freq(dc_motor_t *motor, pwm_freq_t frequency);

/**
 * @brief disables the motor.
 */
bool dc_motor_disable(dc_motor_t *motor);

/**
 * @brief enables the motor.
 * @note sets the duty cycle to 0 for safety
 */
bool dc_motor_enable(dc_motor_t *motor);

#endif // DC_MOTORS_H
#include "dc_motors.h"

/**
 * @brief inits the dc motor
 * @todo complete documentation
 * @param motor pointer to the dc_motor_s
 * @param timer pointer to timer handle - the timer connected to the motor
 * @param timer_channel_output_1 which timer channel is connected to the first input of the driver
 * @param timer_channel_output_2 which timer channel is connected to the second input of the driver
 * @param frequency the desired pwm frequency
 */
void dc_motor_init(dc_motor_t *motor, TIM_HandleTypeDef *timer, uint32_t timer_channel_output_1, uint32_t timer_channel_output_2, pwm_freq_t frequency)
{
    if (motor == NULL || timer == NULL)
    {
        return;
        /**
         * @todo throw an error
         */
    }

    motor->timer = timer;
    motor->timer_channel_output_1 = timer_channel_output_1;
    motor->timer_channel_output_2 = timer_channel_output_2;
	motor->timer_channel_output_3 = 0;
    motor->timer_channel_output_4 = 0;
	
	motor->current_direction = MOTOR_DIRECTION_FORWARD;
	motor->status = MOTOR_STATE_NORMAL;

	/**
	 * Enable the timer using HAL
	 */
	HAL_TIM_Base_Start((TIM_HandleTypeDef *)motor->timer);
	dc_motor_set_timer_freq(motor, frequency);
	// Set initial duty cycle to 0 for both channels
	__HAL_TIM_SET_COMPARE((TIM_HandleTypeDef *)motor->timer, motor->timer_channel_output_1, 0);
	__HAL_TIM_SET_COMPARE((TIM_HandleTypeDef *)motor->timer, motor->timer_channel_output_2, 0);
	HAL_TIM_PWM_Start((TIM_HandleTypeDef *)motor->timer, motor->timer_channel_output_1);
	HAL_TIM_PWM_Start((TIM_HandleTypeDef *)motor->timer, motor->timer_channel_output_2);
	motor->current_duty_cycle = 0;
}

void dc_motor_init_simple_Hbridge(dc_motor_t *motor, TIM_HandleTypeDef *timer, uint32_t timer_channel_output_1, uint32_t timer_channel_output_2, uint32_t timer_channel_output_3, uint32_t timer_channel_output_4, pwm_freq_t frequency)
{
    if (motor == NULL || timer == NULL)
    {
        return;
        /**
         * @todo throw an error
         */
    }

    motor->timer = timer;
    motor->timer_channel_output_1 = timer_channel_output_1;
    motor->timer_channel_output_2 = timer_channel_output_2;
	motor->timer_channel_output_3 = timer_channel_output_3;
    motor->timer_channel_output_4 = timer_channel_output_4;
	motor->current_direction = MOTOR_DIRECTION_FORWARD;
	motor->status = MOTOR_STATE_NORMAL;

	/**
	 * Enable the timer using HAL
	 */
	HAL_TIM_Base_Start((TIM_HandleTypeDef *)motor->timer);
	dc_motor_set_timer_freq(motor, frequency);
	// Set initial duty cycle to 0 for both channels
	__HAL_TIM_SET_COMPARE((TIM_HandleTypeDef *)motor->timer, motor->timer_channel_output_1, 0);
	__HAL_TIM_SET_COMPARE((TIM_HandleTypeDef *)motor->timer, motor->timer_channel_output_2, 0);
	__HAL_TIM_SET_COMPARE((TIM_HandleTypeDef *)motor->timer, motor->timer_channel_output_3, 0);
	__HAL_TIM_SET_COMPARE((TIM_HandleTypeDef *)motor->timer, motor->timer_channel_output_4, 0);
	
	HAL_TIM_PWM_Start((TIM_HandleTypeDef *)motor->timer, motor->timer_channel_output_1);
	HAL_TIM_PWM_Start((TIM_HandleTypeDef *)motor->timer, motor->timer_channel_output_2);
	HAL_TIM_PWM_Start((TIM_HandleTypeDef *)motor->timer, motor->timer_channel_output_3);
	HAL_TIM_PWM_Start((TIM_HandleTypeDef *)motor->timer, motor->timer_channel_output_4);

	motor->current_duty_cycle = 0;
}
/**
 * @brief sets the pwm duty cycle of the motor
 * @param timer pointer to timer handle - the timer connected to the motor
 * @param dir the desired direction of the motor
 * @param duty_cycle a number between 0 and 255 not a percentage
 */
void dc_motor_set(dc_motor_t *motor, dc_motor_direction_t dir, uint8_t duty_cycle)
{
	if (motor == NULL || motor->timer == NULL)
	{
		return;
		/**
		 * @todo throw an error
		 */
	}

	if (duty_cycle < 0 || duty_cycle > 255)
	{
		return;
	}
	// Map duty cycle (0-255) to timer compare value
	uint32_t compare_value = (uint32_t)((duty_cycle / 255.0) * __HAL_TIM_GET_AUTORELOAD((TIM_HandleTypeDef *)motor->timer));
	
	switch (dir)
	{
	case MOTOR_DIRECTION_FORWARD:
		// Check to see if we are using a simple H bridge
		if (motor->timer_channel_output_3 != 0 && motor->timer_channel_output_4 != 0)
		{
			__HAL_TIM_SET_COMPARE((TIM_HandleTypeDef *)motor->timer, motor->timer_channel_output_3, 0);
			__HAL_TIM_SET_COMPARE((TIM_HandleTypeDef *)motor->timer, motor->timer_channel_output_4, __HAL_TIM_GET_AUTORELOAD((TIM_HandleTypeDef *)motor->timer));
		}
		__HAL_TIM_SET_COMPARE((TIM_HandleTypeDef *)motor->timer, motor->timer_channel_output_2, 0);
		__HAL_TIM_SET_COMPARE((TIM_HandleTypeDef *)motor->timer, motor->timer_channel_output_1, compare_value);
		break;
	case MOTOR_DIRECTION_REVERSE:
		if (motor->timer_channel_output_3 != 0 && motor->timer_channel_output_4 != 0)
		{
			__HAL_TIM_SET_COMPARE((TIM_HandleTypeDef *)motor->timer, motor->timer_channel_output_4, 0);
			__HAL_TIM_SET_COMPARE((TIM_HandleTypeDef *)motor->timer, motor->timer_channel_output_3, __HAL_TIM_GET_AUTORELOAD((TIM_HandleTypeDef *)motor->timer));
		}
		__HAL_TIM_SET_COMPARE((TIM_HandleTypeDef *)motor->timer, motor->timer_channel_output_1, 0);
		__HAL_TIM_SET_COMPARE((TIM_HandleTypeDef *)motor->timer, motor->timer_channel_output_2, compare_value);
		break;
	default:
		__HAL_TIM_SET_COMPARE((TIM_HandleTypeDef *)motor->timer, motor->timer_channel_output_1, 0);
		__HAL_TIM_SET_COMPARE((TIM_HandleTypeDef *)motor->timer, motor->timer_channel_output_2, 0);
		__HAL_TIM_SET_COMPARE((TIM_HandleTypeDef *)motor->timer, motor->timer_channel_output_3, 0);
		__HAL_TIM_SET_COMPARE((TIM_HandleTypeDef *)motor->timer, motor->timer_channel_output_4, 0);
		/**
		 * @todo throw an error
		 */
		break;
	}
	motor->current_duty_cycle = duty_cycle;
	motor->current_direction = dir;
}

/**
 * @brief sets the pwm frequency
 * @note this changes the frequency of the entire timer, this includes all four channels
 */
void dc_motor_set_timer_freq(dc_motor_t *motor, pwm_freq_t frequency)
{
	if (frequency == 0)
	{
		// Invalid frequency, return or handle error
		dc_motor_disable(motor);
		return;
	}
	if (frequency > DC_MOTOR_MAX_FREQUENCY)
	{
		frequency = DC_MOTOR_MAX_FREQUENCY;
	}
	
	uint32_t timer_clock = HAL_RCC_GetSysClockFreq(); // Get the timer clock frequency
	uint16_t prescaler = 0;
	uint32_t auto_reload = 0;

	// Calculate prescaler and auto-reload values
	for (prescaler = 0; prescaler <= 0xFFFF; prescaler++)
	{
		auto_reload = (timer_clock / ((prescaler + 1) * frequency)) - 1;
		
		if (auto_reload <= 0xFFFF)
		{
			break;

		}
	}

	if (prescaler > 0xFFFF || auto_reload > 0xFFFF)
	{
		// Frequency is too low, cannot configure timer
		return;
	}
	
	// Set the prescaler and auto-reload values
	__HAL_TIM_SET_PRESCALER((TIM_HandleTypeDef *)motor->timer, prescaler);
	__HAL_TIM_SET_AUTORELOAD((TIM_HandleTypeDef *)motor->timer, auto_reload);

	dc_motor_set(motor, motor->current_direction, motor->current_duty_cycle);
}

/**
 * @brief disables the motor.
 */
void dc_motor_disable(dc_motor_t *motor)
{
	if (motor == NULL || motor->timer == NULL)
	{
		return;
		/**
		 * @todo throw an error
		 */
	}

	// Set duty cycle to 0 for both channels
	__HAL_TIM_SET_COMPARE((TIM_HandleTypeDef *)motor->timer, motor->timer_channel_output_1, 0);
	__HAL_TIM_SET_COMPARE((TIM_HandleTypeDef *)motor->timer, motor->timer_channel_output_2, 0);
	motor->current_duty_cycle = 0;
	motor->status = MOTOR_STATE_DISABLED;
}

/**
 * @brief enables the motor.
 * @note sets the duty cycle to 0 for safety
 */
void dc_motor_enable(dc_motor_t *motor)
{
	if (motor == NULL || motor->timer == NULL)
	{
		return;
		/**
		 * @todo throw an error
		 */
	}
	if (motor->status == MOTOR_STATE_NORMAL || motor->status == MOTOR_STATE_FAULT || motor == NULL)
	{
		return;
		/**
		 * @todo throw an error
		 */
	}

	// Set duty cycle to 0 for both channels
	__HAL_TIM_SET_COMPARE((TIM_HandleTypeDef *)motor->timer, motor->timer_channel_output_1, 0);
	__HAL_TIM_SET_COMPARE((TIM_HandleTypeDef *)motor->timer, motor->timer_channel_output_2, 0);
	motor->current_duty_cycle = 0;
	motor->status = MOTOR_STATE_NORMAL;
}

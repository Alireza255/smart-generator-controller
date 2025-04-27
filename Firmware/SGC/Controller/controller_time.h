#ifndef CONTROLLER_TIME_H
#define CONTROLLER_TIME_H

#include "types.h"
#include "tim.h"
#include "main.h"

/**
 * @brief starts the timer used for timing int the controller
 */
void controller_time_start(TIM_HandleTypeDef *htim);

/**
 * integer time is microseconds since MCU reset, used for precise timing
 * @note must use "delta = (current - start)" to handle overflows properly.
*/
controller_time_us_t get_time_us(void);

/**
* @brief time in milliseconds (1/1_000 of a second)
* @note must use "delta = (current - start)" to handle overflows properly.
*/
controller_time_ms_t get_time_ms(void);

#endif // CONTROLLER_TIME_H

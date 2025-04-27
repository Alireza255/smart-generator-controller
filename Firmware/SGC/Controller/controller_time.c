#include "controller_time.h"

TIM_HandleTypeDef *timer = NULL;

void controller_time_start(TIM_HandleTypeDef *htim)
{
    if (htim == NULL)
    {
        /**
         * @todo throw an error
         */
        return;
    }
    timer = htim;
    /**
     * This assumes that sys clock is the same as timer clock
     */
    timer->Instance->PSC = HAL_RCC_GetSysClockFreq() / 1000000UL;
    HAL_TIM_Base_Start(timer);
}


controller_time_us_t get_time_us(void)
{
    return timer->Instance->CNT;
}

controller_time_ms_t get_time_ms(void)
{
    return timer->Instance->CNT / 1000U;
}

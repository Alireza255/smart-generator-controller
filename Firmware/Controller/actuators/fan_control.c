#include "fan_control.h"

void fan_control_update(fan_control_t *fan)
{
    if (fan->temp_off == fan->temp_on)
    {
        HAL_GPIO_WritePin(fan->pin.gpio, fan->pin.pin, GPIO_PIN_RESET);
        osEventFlagsClear(engine.flags, ENGINE_FLAG_FAN_ON);
        log_error("Fan off or on temperature is not set");
    }

    temperature_t current_temp = sensor_clt_get();
    // Fan control logic here
    if (current_temp > configuration.fan1_on_temp)
    {
        HAL_GPIO_WritePin(FAN1_GPIO_Port, FAN1_Pin, GPIO_PIN_SET);
        osEventFlagsSet(engine.flags, fan->on_flag);
    }
    else if (current_temp < configuration.fan1_off_temp)
    {
        HAL_GPIO_WritePin(FAN1_GPIO_Port, FAN1_Pin, GPIO_PIN_RESET);
        osEventFlagsClear(engine.flags, fan->on_flag);
    }


}
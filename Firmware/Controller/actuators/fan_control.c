#include "fan_control.h"

static bool thermostat(temperature_t temp, temperature_t on_temp, temperature_t off_temp)
{
    if (temp > on_temp)
    {
        return true;
    }
    else if (temp < off_temp)
    {
        return false;
    }
}
void fan_control_update()
{
    temperature_t current_temp = sensor_clt_get();

    bool fan1_command = false;
    bool fan2_command = false;
    
    if (config.fan1_enabled)
    {
        if (current_temp > config.fan1_on_temp)
        {
            fan1_command = true;
        }
        else if (current_temp < config.fan1_off_temp)
        {
            fan1_command = false;
        }
    }
    else
    {
        fan1_command = false;
    }
    
    if (config.fan2_enabled)
    {
        if (current_temp > config.fan2_on_temp)
        {
            fan2_command = true;
        }
        else if (current_temp < config.fan2_off_temp)
        {
            fan2_command = false;
        }
    }
    else
    {
       fan2_command = false;
    }
    
    HAL_GPIO_WritePin(FAN_1_GPIO_Port, FAN_1_Pin, fan1_command);
    change_bit(&runtime.status, STATUS_FAN1_ON, fan1_command);
    
    HAL_GPIO_WritePin(FAN_2_GPIO_Port, FAN_2_Pin, fan2_command);
    change_bit(&runtime.status, STATUS_FAN2_ON, fan2_command);

}
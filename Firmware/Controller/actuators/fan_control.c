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
    if (get_bit(runtime.status, STATUS_CLT_ERROR))
    {
        fan1_command = true;
        fan2_command = true;
    }
    
    output_set(&fan1_output, fan1_command);
    change_bit(&runtime.status, STATUS_FAN1_ON, fan1_command);
        
    output_set(&fan2_output, fan2_command);
    change_bit(&runtime.status, STATUS_FAN2_ON, fan2_command);

}
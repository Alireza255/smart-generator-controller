#include "outputs.h"

controller_output_pin_t main_relay_output =
{
    .gpio = PWM_OUT_3_GPIO_Port,
    .pin = PWM_OUT_3_Pin,
    .override_active = false,
    .last_normal_state = false
};

controller_output_pin_t gas_solenoid_output =
{
    .gpio = PWM_OUT_2_GPIO_Port,
    .pin = PWM_OUT_2_Pin,
    .override_active = false,
    .last_normal_state = false
};
controller_output_pin_t fuel_pump_output =
{
    .gpio = FUEL_PUMP_RELAY_GPIO_Port,
    .pin = FUEL_PUMP_RELAY_Pin,
    .override_active = false,
    .last_normal_state = false
};
controller_output_pin_t fan1_output =
{
    .gpio = PWM_OUT_1_GPIO_Port,
    .pin = PWM_OUT_1_Pin,
    .override_active = false,
    .last_normal_state = false
};
controller_output_pin_t fan2_output =
{
    .gpio = FAN_2_GPIO_Port,
    .pin = FAN_2_Pin,
    .override_active = false,
    .last_normal_state = false
};
controller_output_pin_t injector_output[FIRMWARE_INJECTOR_OUTPUT_COUNT] = {
    {.gpio = INJECTOR_OUTPUT_1_GPIO_Port, .pin = INJECTOR_OUTPUT_1_Pin, .override_active = false, .last_normal_state = false},
    {.gpio = INJECTOR_OUTPUT_2_GPIO_Port, .pin = INJECTOR_OUTPUT_2_Pin, .override_active = false, .last_normal_state = false},
    {.gpio = INJECTOR_OUTPUT_3_GPIO_Port, .pin = INJECTOR_OUTPUT_3_Pin, .override_active = false, .last_normal_state = false},
    {.gpio = INJECTOR_OUTPUT_4_GPIO_Port, .pin = INJECTOR_OUTPUT_4_Pin, .override_active = false, .last_normal_state = false}
};
controller_output_pin_t ignition_output[FIRMWARE_IGNITION_OUTPUT_COUNT] = {
    {.gpio = IGNITION_OUTPUT_1_GPIO_Port, .pin = IGNITION_OUTPUT_1_Pin, .override_active = false, .last_normal_state = false},
    {.gpio = IGNITION_OUTPUT_2_GPIO_Port, .pin = IGNITION_OUTPUT_2_Pin, .override_active = false, .last_normal_state = false},
    {.gpio = IGNITION_OUTPUT_3_GPIO_Port, .pin = IGNITION_OUTPUT_3_Pin, .override_active = false, .last_normal_state = false},
    {.gpio = IGNITION_OUTPUT_4_GPIO_Port, .pin = IGNITION_OUTPUT_4_Pin, .override_active = false, .last_normal_state = false}
};


void output_set(controller_output_pin_t *pin, bool state)
{
    if (pin == NULL)
    {
        return;
    }

    __disable_irq();
    if (!pin->override_active)
    {
        HAL_GPIO_WritePin(pin->gpio, pin->pin, state);
    }
    __enable_irq();
}

void output_override(controller_output_pin_t *pin, bool forced_state)
{
    if (pin == NULL)
    {
        return;
    }

    __disable_irq();
    pin->last_normal_state = HAL_GPIO_ReadPin(pin->gpio, pin->pin);
    pin->override_active = true;      // renamed for clarity
    HAL_GPIO_WritePin(pin->gpio, pin->pin, forced_state);
    __enable_irq();
}

void output_override_clear(controller_output_pin_t *pin)
{
    if (pin == NULL)
    {
        return;
    }

    __disable_irq();
    pin->override_active = false;
    HAL_GPIO_WritePin(pin->gpio, pin->pin, pin->last_normal_state);
    __enable_irq();
}


void output_main_relay_set(bool state)
{
    output_set(&main_relay_output, state);
}

void output_gas_solenoid_set(bool state)
{
    output_set(&gas_solenoid_output, state);
}

void output_fuel_pump_set(bool state)
{
    output_set(&fuel_pump_output, state);
}

void output_fan1_set(bool state)
{
    output_set(&fan1_output, state);
}

void output_fan2_set(bool state)
{
    output_set(&fan2_output, state);
}

void output_fire_ignition(controller_output_pin_t *pin)
{
    if (pin == NULL)
    {
        return;
    }
    output_override(pin, true);
    osDelay(3);
    output_override_clear(pin);
    output_set(pin, false);
}
void output_fire_injection(controller_output_pin_t *pin)
{
    if (pin == NULL)
    {
        return;
    }
    output_override(pin, true);
    osDelay(10);
    output_override_clear(pin);
    output_set(pin, false);

}
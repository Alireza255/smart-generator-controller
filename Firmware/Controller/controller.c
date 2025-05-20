#include "controller.h"

configuration_s configuration = {0};
engine_s engine = {0};


void controller_init()
{
    engine.total_revolutions = 0;
    engine.crankshaft_angle = 0;
    engine.camshaft_angle = 0;
    engine.rpm = 0;
    engine.cylinder_count = 4;
    engine.spinning_state = SS_STOPPED;
    engine.firing_interval = 180.0f;
    // engine.trigger = .....;

    configuration.ignition_dwell = (float_time_ms_t)3;
    configuration.ignition_mode = IM_WASTED_SPARK;

    configuration.trigger.filtering = TF_FILTERING_NONE;
    configuration.trigger.full_teeth = 60;
    configuration.trigger.missing_teeth = 2;
    configuration.cranking_rpm_threshold = 400;
    
    ignition_output_conf_s ignition_output_conf =
    {
        .output[0] = {.gpio = IGNITION_OUTPUT1_GPIO_Port, .pin = IGNITION_OUTPUT1_Pin},
        .output[1] = {.gpio = IGNITION_OUTPUT2_GPIO_Port, .pin = IGNITION_OUTPUT2_Pin},
        .output[2] = {.gpio = IGNITION_OUTPUT3_GPIO_Port, .pin = IGNITION_OUTPUT3_Pin},
        .output[3] = {.gpio = IGNITION_OUTPUT4_GPIO_Port, .pin = IGNITION_OUTPUT4_Pin}
    };

    controller_timing_start(&htim2);
    trigger_init(&engine.trigger);
    ignition_init(&ignition_output_conf);

    //HAL_FLASH_Unlock();
    //EE_Init();
}

void controller_load_configuration()
{
    uint16_t status = HAL_OK;
    uint32_t read_size = sizeof(configuration) / sizeof(uint16_t);
    uint16_t *starting_point = (uint16_t *) &configuration;
    for (size_t i = 0; i < read_size; i++)
    {
        status = EE_ReadVariable(i, starting_point + i);
        if (status != HAL_OK)
        {
            log_error("Configuration load failed!");
            break;
        }
    }
}

void controller_save_configuration()
{
    uint16_t status = HAL_OK;
    uint32_t write_size = sizeof(configuration) / sizeof(uint16_t);
    uint16_t *starting_point = (uint16_t *) &configuration;
    for (size_t i = 0; i < write_size; i++)
    {
        status = EE_WriteVariable(i, *(starting_point + i));
        if (status != HAL_OK)
        {
            log_error("Configuration save failed!");
        }
    }
}

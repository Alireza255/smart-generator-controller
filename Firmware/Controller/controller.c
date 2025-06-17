#include "controller.h"

configuration_t configuration = {0};
engine_t engine = {0};

void controller_init_with_defaults()
{   
    /* Initialize engine flags */
    static const osEventFlagsAttr_t engine_flags_attributes =
    {
        .name = "engine_flags"
    };
    engine.flags = osEventFlagsNew(&engine_flags_attributes);

    /* Initialize engine stats to 0 */
    engine.total_revolutions = 0;
    engine.crankshaft_angle = 0;
    engine.camshaft_angle = 0;
    engine.rpm = 0;
    engine.cylinder_count = 4;
    engine.spinning_state = SS_STOPPED;
    engine.trigger.initialized = false;

    /* Start controller timing */
    controller_timing_start(&htim2);

    /* Init analog inputs*/
    analog_inputs_init(&hadc1);

    
    configuration.engine_displacment = 2.4f;
    configuration.firing_order = FO_1342;
    configuration.fuel_type = FUEL_TYPE_GAS;
    
    
    configuration.cranking_rpm_threshold = 400;
    configuration.cranking_advance = 10.0f;
    configuration.cranking_throttle = 10;
    
    configuration.ignition_mode = IM_WASTED_SPARK;
    configuration.ignition_dwell = 3.0f;
    configuration.ignition_is_multi_spark = false;
    configuration.ignition_multi_spark_number_of_sparks = 0;
    configuration.ignition_multi_spark_rpm_threshold = 0;
    configuration.ignition_multi_spark_rest_time = 0;
    
    
    /* Initialize trigger */
    configuration.trigger.filtering = TRIGGER_FILTERING_NONE;
    configuration.trigger.full_teeth = 60;
    configuration.trigger.missing_teeth = 2;
    trigger_init(&engine.trigger, &configuration.trigger);
    

    // comms_init();

    /* Initialize analog sensors BEGIN*/
    /* TPS */
    static sensor_tps_t tps1 = {
        .analog_channel = ANALOG_INPUT_ETB1_SENSE1,
        .closed_throttle_adc_value = 0,
        .wide_open_throttle_adc_value = 4095,
        .is_inverted = false
    };
    
    /* CLT */
    configuration.clt_sensor_type = SENSOR_CLT_TYPE_TEST;
    static thermistor_t sensor_clt = {0};
    sensor_clt_init(&sensor_clt, configuration.clt_sensor_type);

    /* IAT */
    configuration.iat_sensor_type = SENSOR_IAT_TYPE_TEST;
    static thermistor_t sensor_iat = {0};
    sensor_iat_init(&sensor_iat, configuration.iat_sensor_type);

    /* MAP */
    configuration.map_sensor_type = SENSOR_MAP_TYPE_TEST;
    static sensor_map_t sensor_map = {0};
    sensor_map_init(&sensor_map, configuration.map_sensor_type);

    /* OPS */
    static sensor_ops_t sensor_ops = {0};
    sensor_ops_init(&sensor_ops);

    /* Initialize analog sensors END*/

    /* Initialize fan*/
    //static fan_control_t fan1 = {.on_flag = ENGINE_FLAG_FAN_ON, .pin = {FAN1_GPIO_Port, FAN1_Pin}, .temp_off = 80.0f, .temp_on = 85.0f};
    //fan_control_update(&fan1);


    /* Initialize ignition */
    ignition_output_conf_t ignition_output_conf =
    {
        .output[0] = {.gpio = IGNITION_OUTPUT1_GPIO_Port, .pin = IGNITION_OUTPUT1_Pin},
        .output[1] = {.gpio = IGNITION_OUTPUT2_GPIO_Port, .pin = IGNITION_OUTPUT2_Pin},
        .output[2] = {.gpio = IGNITION_OUTPUT3_GPIO_Port, .pin = IGNITION_OUTPUT3_Pin},
        .output[3] = {.gpio = IGNITION_OUTPUT4_GPIO_Port, .pin = IGNITION_OUTPUT4_Pin}
    };
    ignition_init(&ignition_output_conf);

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

#include "controller.h"

configuration_t configuration = {0};
engine_t engine = {0};

void controller_test_task(void *arg);

sensor_tps_t tps1 = {
      .analog_channel = ANALOG_INPUT_ETB1_SENSE1,
      .closed_throttle_adc_value = 1700,
      .wide_open_throttle_adc_value = 3000,
      .is_inverted = false};

  static electronic_throttle_t etb1 = {0};
  static dc_motor_t etb1_motor = {0};
  static pid_t etb1_pid = {.derivative_filter_tau = 0.1f, .Kp = 20.0f, .Ki = 1, .Kd = 0, .limit_output_min = -100, .limit_output_max = 100, .limit_integrator_min = -255, .limit_integrator_max = 255, .setpoint = 50};


void controller_init_with_defaults()
{
  /* Initialize engine flags */
  static const osEventFlagsAttr_t engine_flags_attributes =
      {
          .name = "engine_flags"};
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
  dc_motor_init_simple_Hbridge(&etb1_motor, &htim3, TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3, TIM_CHANNEL_4, 25000);
  pid_init(&etb1_pid);
  electronic_throttle_init(&etb1, &etb1_pid, &tps1, &etb1_motor);
  electronic_throttle_set(&etb1, 70);

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
  // static fan_control_t fan1 = {.on_flag = ENGINE_FLAG_FAN_ON, .pin = {FAN1_GPIO_Port, FAN1_Pin}, .temp_off = 80.0f, .temp_on = 85.0f};
  // fan_control_update(&fan1);

  /* Initialize ignition */
  ignition_output_conf_t ignition_output_conf =
      {
          .output[0] = {.gpio = IGNITION_OUTPUT1_GPIO_Port, .pin = IGNITION_OUTPUT1_Pin},
          .output[1] = {.gpio = IGNITION_OUTPUT2_GPIO_Port, .pin = IGNITION_OUTPUT2_Pin},
          .output[2] = {.gpio = IGNITION_OUTPUT3_GPIO_Port, .pin = IGNITION_OUTPUT3_Pin},
          .output[3] = {.gpio = IGNITION_OUTPUT4_GPIO_Port, .pin = IGNITION_OUTPUT4_Pin}};
  ignition_init(&ignition_output_conf);
  
    osTimerId_t timer_id = osTimerNew(
        controller_update_stats,
        osTimerPeriodic,
        NULL,
        NULL
    );
    osTimerStart(timer_id, 10); // every 10 tick = every 10ms = 100Hz
    

    comms_init();

    
    const osThreadAttr_t controller_test_attr = {
        .name = "test_task",
        .stack_size = 1024,
        .priority = osPriorityNormal,
    };
    osThreadNew(controller_test_task, NULL, &controller_test_attr);
}

void controller_load_configuration()
{
  
}

void controller_save_configuration()
{
}

void controller_update_stats(void *arg)
{
  // this must not run before the controller is initialized
  engine.tps1 = sensor_tps_get(&tps1);

}

void controller_test_task(void *arg)
{
  for (;;)
  {
    
    percent_t etb_test_target_pos = 20;
    const percent_t etb_test_min = 0;
    const percent_t etb_test_max = 95;
    const percent_t etb_test_resolution = 0.02;
    const float_time_ms_t etb_test_period = 10000;
    time_ms_t etb_update_period = etb_test_period / ((float)2 * (etb_test_max - etb_test_min)) * etb_test_resolution;
    for (etb_test_target_pos = 20; etb_test_target_pos < 80; etb_test_target_pos += etb_test_resolution)
    {
      osDelay((uint32_t)etb_update_period);
      electronic_throttle_set(&etb1, etb_test_target_pos);
    }
    for (etb_test_target_pos = 80; etb_test_target_pos > 20; etb_test_target_pos -= etb_test_resolution)
    {
      osDelay((uint32_t)etb_update_period);
      electronic_throttle_set(&etb1, etb_test_target_pos);
    }
  }
  

}
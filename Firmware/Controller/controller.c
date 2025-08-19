#include "controller.h"

config_t config = {0};
runtime_t runtime = {0};

void controller_sensor_task(void *arg);
void controller_test_task(void *arg);
void controller_long_routines_task(void *arg);

static sensor_tps_t tps1 = {
    .analog_channel = ANALOG_INPUT_ETB1_SENSE1,
    .closed_throttle_adc_value = 1700,
    .wide_open_throttle_adc_value = 3000,
    .is_inverted = false};
static sensor_tps_t tps2 = {
    .analog_channel = ANALOG_INPUT_ETB2_SENSE1,
    .closed_throttle_adc_value = 1700,
    .wide_open_throttle_adc_value = 3000,
    .is_inverted = false};

static electronic_throttle_t etb1 = {0};
static dc_motor_t etb1_motor = {0};
static pid_t etb1_pid = {.derivative_filter_tau = 0.1f, .Kp = 20.0f, .Ki = 1, .Kd = 0, .limit_output_min = -100, .limit_output_max = 100, .limit_integrator_min = -255, .limit_integrator_max = 255, .setpoint = 50};

static electronic_throttle_t etb2 = {0};
static dc_motor_t etb2_motor = {0};
static pid_t etb2_pid = {0};

static thermistor_t sensor_clt = {0};
static thermistor_t sensor_iat = {0};
static sensor_map_t sensor_map = {0};
static sensor_ops_t sensor_ops = {0};

static controller_output_pin_t injector_ouput_pins[FIRMWARE_INJECTOR_OUPUT_COUNT] = {
    {.gpio = INJECTOR_OUTPUT_1_GPIO_Port, .pin = INJECTOR_OUTPUT_1_Pin},
    {.gpio = INJECTOR_OUTPUT_2_GPIO_Port, .pin = INJECTOR_OUTPUT_2_Pin},
    {.gpio = INJECTOR_OUTPUT_3_GPIO_Port, .pin = INJECTOR_OUTPUT_3_Pin},
    {.gpio = INJECTOR_OUTPUT_4_GPIO_Port, .pin = INJECTOR_OUTPUT_4_Pin}};
static controller_output_pin_t ignition_output_pins[FIRMWARE_IGNITION_OUTPUT_COUNT] = {
    {.gpio = IGNITION_OUTPUT_1_GPIO_Port, .pin = IGNITION_OUTPUT_1_Pin},
    {.gpio = IGNITION_OUTPUT_2_GPIO_Port, .pin = IGNITION_OUTPUT_2_Pin},
    {.gpio = IGNITION_OUTPUT_3_GPIO_Port, .pin = IGNITION_OUTPUT_3_Pin},
    {.gpio = IGNITION_OUTPUT_4_GPIO_Port, .pin = IGNITION_OUTPUT_4_Pin}};

trigger_t trigger1;



void controller_load_test_configuration()
{
  // VE and ignition tables: zeroed by default, or set up as needed elsewhere

  // Cranking & ignition floats
  config.cranking_rpm_threshold = 400.0f;
  config.cranking_advance = 8.0f;
  config.cranking_throttle = 10.0f;

  config.ignition_dwell = 2.0f;
  config.multi_spark_rpm_threshold = 3500.0f;
  config.multi_spark_rest_time_ms = 1.0f;
  config.multi_spark_max_trailing_angle = 15.0f;

  // Stoichiometric AFRs
  config.stoich_afr_gas = 14.7f;
  config.stoich_afr_petrol = 14.7f;

  // Governer (PID) floats
  config.governer_target_rpm = 1500.0f;
  config.governer_idle_rpm = 850.0f;
  config.governer_pid_Kp = 10.0f;
  config.governer_pid_Ki = 0.5f;
  config.governer_pid_Kd = 0.1f;
  config.governer_pid_limit_integrator_min = -100.0f;
  config.governer_pid_limit_integrator_max = 100.0f;
  config.governer_pid_derivative_filter_tau = 0.1f;

  // ETB1 PID floats
  config.etb1_pid_Kp = 20.0f;
  config.etb1_pid_Ki = 1.0f;
  config.etb1_pid_Kd = 0.0f;
  config.etb1_pid_limit_integrator_min = 0.0f;
  config.etb1_pid_limit_integrator_max = 255.0f;
  config.etb1_pid_derivative_filter_tau = 0.1f;

  // ETB2 PID floats
  config.etb2_pid_Kp = 20.0f;
  config.etb2_pid_Ki = 1.0f;
  config.etb2_pid_Kd = 0.0f;
  config.etb2_pid_limit_integrator_min = 0.0f;
  config.etb2_pid_limit_integrator_max = 255.0f;
  config.etb2_pid_derivative_filter_tau = 0.1f;

  // Protection & fan floats
  config.protection_clt_shutdown_temprature = 110.0f;
  config.protection_clt_load_disconnect_temprature = 100.0f;

  config.fan1_on_temp = 95.0f;
  config.fan1_off_temp = 90.0f;
  config.fan2_on_temp = 100.0f;
  config.fan2_off_temp = 95.0f;

  // RPM/load/ignition bins: can be set up elsewhere or left zeroed for now

  config.rpm_limiter = 4000;
  config.engine_displacement_cc = 2400;

  config.tps1_calib_wide_open_throttle_adc_value = 3000;
  config.tps1_calib_closed_throttle_adc_value = 1700;
  config.tps2_calib_wide_open_throttle_adc_value = 3000;
  config.tps2_calib_closed_throttle_adc_value = 1700;

  // Firing order, fuel type, triggers
  config.firing_order = FO_1342;
  config.fuel_type = FUEL_TYPE_GAS;
  config.trigger1_type = TW_58_TOOTH_2_MISSING; // e.g. TRIGGER_TYPE_60_2
  config.trigger1_filtering = TRIGGER_FILTERING_LITE;
  config.trigger2_type = 0;
  config.trigger2_filtering = 0;
  config.trigger2_enabled = 0;

  config.number_of_injectors = 4;
  config.injection_mode = IM_SIMULTANEOUS;

  config.ignition_mode = IM_WASTED_SPARK;
  config.multi_spark_enabled = 1;
  config.multi_spark_number_of_sparks = 5;

  config.tps1_calib_is_inverted = 0;
  config.tps2_calib_is_inverted = 0;

  config.sensor_clt_type = SENSOR_CLT_TYPE_TEST;
  config.sensor_iat_type = SENSOR_IAT_TYPE_TEST;
  config.sensor_map_type = SENSOR_MAP_TYPE_TEST;

  config.protection_oil_pressure_time = 3;
  config.protection_oil_pressure_enabled = 0;
  config.protection_clt_enabled = 0;
  config.protection_clt_load_disconnect_enabled = 0;

  config.etb1_enabled = 1;
  config.etb2_enabled = 0;
  config.etb1_motor_inverted = 0;
  config.etb2_motor_inverted = 0;

  config.fan1_enabled = 1;
  config.fan2_enabled = 0;

  for (size_t i = 0; i < TABLE_PRIMARY_SIZE_X; i++)
  {
    config.ve_table_1.x_bins[i] = i * FIRMWARE_LIMIT_MAX_RPM / TABLE_PRIMARY_SIZE_X;
    config.ve_table_2.x_bins[i] = i * FIRMWARE_LIMIT_MAX_RPM / TABLE_PRIMARY_SIZE_X;
    config.ign_table_1.x_bins[i] = i * FIRMWARE_LIMIT_MAX_RPM / TABLE_PRIMARY_SIZE_X;
    config.ign_table_2.x_bins[i] = i * FIRMWARE_LIMIT_MAX_RPM / TABLE_PRIMARY_SIZE_X;
  }
  
  for (size_t i = 0; i < TABLE_PRIMARY_SIZE_Y; i++)
  {
    config.ve_table_1.y_bins[i] = i * FIRMWARE_LIMIT_MAX_MAP / TABLE_PRIMARY_SIZE_Y;
    config.ve_table_2.y_bins[i] = i * FIRMWARE_LIMIT_MAX_MAP / TABLE_PRIMARY_SIZE_Y;
    config.ign_table_1.y_bins[i] = i * FIRMWARE_LIMIT_MAX_MAP / TABLE_PRIMARY_SIZE_Y;
    config.ign_table_2.y_bins[i] = i * FIRMWARE_LIMIT_MAX_MAP / TABLE_PRIMARY_SIZE_Y;
  }
  const float table_init_data = 16;

  for (size_t i = 0; i < TABLE_PRIMARY_SIZE_X; i++)
  {
    for (size_t j = 0; j < TABLE_PRIMARY_SIZE_Y; j++)
    {
      config.ve_table_1.data[i][j] = table_init_data;
      config.ve_table_2.data[i][j] = table_init_data;
      config.ign_table_1.data[i][j] = table_init_data;
      config.ign_table_2.data[i][j] = table_init_data;
      
    }
    
  }
  
  
  
}

void controller_init_with_defaults()
{

  /* Initialize engine stats to 0 */
  memset(&runtime, 0, sizeof(runtime));
  
  /* Start controller timing */
  controller_timing_start(&htim2);

  /* Init analog inputs*/
  analog_inputs_init(&hadc1);

  controller_load_test_configuration();

  trigger_init(&trigger1, config.trigger1_type, &config.trigger1_filtering, STATUS_TRIGGER1_SYNCED, 1);

  /* Initialize analog sensors BEGIN*/
  /* TPS */
  dc_motor_init_simple_Hbridge(&etb1_motor, &htim3, TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3, TIM_CHANNEL_4, 25000);
  pid_init(&etb1_pid);
  electronic_throttle_init(&etb1, &etb1_pid, &tps1, &etb1_motor);
  electronic_throttle_set(&etb1, 70);

  /* CLT */
  sensor_clt_init(&sensor_clt, config.sensor_clt_type);

  /* IAT */
  sensor_iat_init(&sensor_iat, config.sensor_iat_type);

  /* MAP */
  sensor_map_init(&sensor_map, config.sensor_map_type);

  /* OPS */
  //sensor_ops_init(&sensor_ops);

  /* Initialize analog sensors END*/

  /* Initialize fan*/
  // static fan_control_t fan1 = {.on_flag = ENGINE_FLAG_FAN_ON, .pin = {FAN1_GPIO_Port, FAN1_Pin}, .temp_off = 80.0f, .temp_on = 85.0f};
  // fan_control_update(&fan1);

  /* Initialize ignition */

  ignition_init(ignition_output_pins);

  comms_init();

  const osThreadAttr_t controller_test_attr = {
      .name = "test_task",
      .stack_size = 1024,
      .priority = osPriorityRealtime,
  };
  const osThreadAttr_t controller_sensor_task_attr = {
      .name = "sensor_task",
      .stack_size = 1024,
      .priority = osPriorityAboveNormal,
  };
  
  const osThreadAttr_t controller_long_routines_attr = {
      .name = "sensor_task",
      .stack_size = 128,
      .priority = osPriorityNormal,
  };
  
  
  osThreadNew(controller_test_task, NULL, &controller_test_attr);
  osThreadNew(controller_sensor_task, NULL, &controller_sensor_task_attr);
  osThreadNew(controller_long_routines_task, NULL, &controller_long_routines_attr);
  
}

void controller_load_configuration()
{
}

void controller_save_configuration()
{
}

void controller_long_routines_task(void *arg)
{
  //uint32_t next_routine_time_ticks = 1000;
  for (;;)
  {
    runtime.seconds = get_time_ms() / 1000;





    osDelay(1000);
    //next_routine_time_ticks += 1000;
    //osDelayUntil(next_routine_time_ticks);
  }
  
}

void controller_sensor_task(void *arg)
{
  for (;;)
  {
    runtime.tps1 = sensor_tps_get(&tps1);
    runtime.clt_degc = sensor_clt_get();
    runtime.iat_degc = sensor_iat_get();
    //runtime.map_kpa = sensor_map_get();
    //runtime.oil_pressure_ok = sensor_ops_get();
    osDelay(1);
  }
  
}
void controller_test_task(void *arg)
{
  osDelay(100);
  trigger_simulator_init(60, 2, trigger_tooth_handle, &trigger1);
  static rpm_t simulated_rpm = 1000;
  for (;;)
  {
    //simulated_rpm = (rpm_t)mapf((float)analog_inputs_get_data(ANALOG_INPUT_ETB2_SENSE2), 0.0f, 4095.0f, 10.0f, 1000.0f);
    simulated_rpm = 500;
    trigger_simulator_update(simulated_rpm);
    osDelay(1);

    /*
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
    */
  }
}


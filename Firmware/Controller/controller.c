#include "controller.h"

config_t config = {0};
runtime_t runtime = {0};

void controller_sensor_task(void *arg);
void controller_test_task(void *arg);
void controller_long_routines_task(void *arg);
void controller_test2_task(void *arg);
static sensor_tps_t tps1 = {
    .analog_channel = ANALOG_INPUT_ETB1_SENSE1,
    .closed_throttle_adc_value = 1613,
    .wide_open_throttle_adc_value = 3170,
    .is_inverted = false};
static sensor_tps_t tps2 = {
    .analog_channel = ANALOG_INPUT_ETB2_SENSE1,
    .closed_throttle_adc_value = 1613,
    .wide_open_throttle_adc_value = 3170,
    .is_inverted = false};

static electronic_throttle_t etb1 = {0};
static dc_motor_t etb1_motor = {0};
static pid_t etb1_pid = {.derivative_filter_tau = 0.01f, .Kp = 15.0f, .Ki = 5, .Kd = 0.001, .limit_output_min = -100, .limit_output_max = 100, .limit_integrator_min = -50, .limit_integrator_max = 50, .setpoint = 0};

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

  // ---------- VE & ignition tables (realistic torque curve + RPM/load) ----------
  for (size_t i = 0; i < TABLE_PRIMARY_SIZE_X; i++)
  {
    float rpm_factor = (float)i / (TABLE_PRIMARY_SIZE_X - 1); // 0..1 across RPM

    // Approximate torque curve: peak around mid-RPM
    float ve_rpm_scale = 0.8f * sinf(rpm_factor * 3.14159f) + 0.2f; // 0.2..1.0

    for (size_t j = 0; j < TABLE_PRIMARY_SIZE_Y; j++)
    {
      float map_factor = (float)j / (TABLE_PRIMARY_SIZE_Y - 1); // 0..1 across MAP/load

      // VE table: scales with torque curve and load
      config.ve_table_1.data[i][j] = 20.0f + 70.0f * ve_rpm_scale * map_factor;
      config.ve_table_2.data[i][j] = 20.0f + 70.0f * ve_rpm_scale * map_factor;

      // Ignition table: base 10° at idle, +20° with RPM, -10° with load
      config.ign_table_1.data[i][j] = 10.0f + 20.0f * rpm_factor - 10.0f * map_factor;
      config.ign_table_2.data[i][j] = 10.0f + 20.0f * rpm_factor - 10.0f * map_factor;
    }
  }

  // x_bins: RPM axis
  for (size_t i = 0; i < TABLE_PRIMARY_SIZE_X; i++)
  {
    config.ve_table_1.x_bins[i] = i * FIRMWARE_LIMIT_MAX_RPM / TABLE_PRIMARY_SIZE_X;
    config.ve_table_2.x_bins[i] = i * FIRMWARE_LIMIT_MAX_RPM / TABLE_PRIMARY_SIZE_X;
    config.ign_table_1.x_bins[i] = i * FIRMWARE_LIMIT_MAX_RPM / TABLE_PRIMARY_SIZE_X;
    config.ign_table_2.x_bins[i] = i * FIRMWARE_LIMIT_MAX_RPM / TABLE_PRIMARY_SIZE_X;
  }

  // y_bins: MAP/load axis
  for (size_t i = 0; i < TABLE_PRIMARY_SIZE_Y; i++)
  {
    config.ve_table_1.y_bins[i] = i * FIRMWARE_LIMIT_MAX_MAP / TABLE_PRIMARY_SIZE_Y;
    config.ve_table_2.y_bins[i] = i * FIRMWARE_LIMIT_MAX_MAP / TABLE_PRIMARY_SIZE_Y;
    config.ign_table_1.y_bins[i] = i * FIRMWARE_LIMIT_MAX_MAP / TABLE_PRIMARY_SIZE_Y;
    config.ign_table_2.y_bins[i] = i * FIRMWARE_LIMIT_MAX_MAP / TABLE_PRIMARY_SIZE_Y;
  }

  // ---------- Cranking & ignition floats ----------
  config.cranking_rpm_threshold = 400.0f;
  config.cranking_advance = 8.0f;
  config.cranking_throttle = 10.0f;

  config.ignition_dwell = 2.5f;
  config.multi_spark_rpm_threshold = 3500.0f;
  config.multi_spark_rest_time_ms = 0.5f;
  config.multi_spark_max_trailing_angle = 15.0f;

  // ---------- Stoichiometric AFRs ----------
  config.stoich_afr_gas = 14.7f;
  config.stoich_afr_petrol = 14.7f;

  // ---------- Governer (PID) floats ----------
  config.governer_target_rpm = 1500.0f;
  config.governer_idle_rpm = 850.0f;
  config.governer_pid_Kp = 10.0f;
  config.governer_pid_Ki = 0.5f;
  config.governer_pid_Kd = 0.1f;
  config.governer_pid_limit_integrator_min = -100.0f;
  config.governer_pid_limit_integrator_max = 100.0f;
  config.governer_pid_derivative_filter_tau = 0.1f;

  // ---------- ETB1 PID floats ----------
  config.etb1_pid_Kp = 20.0f;
  config.etb1_pid_Ki = 1.0f;
  config.etb1_pid_Kd = 0.0f;
  config.etb1_pid_limit_integrator_min = 0.0f;
  config.etb1_pid_limit_integrator_max = 255.0f;
  config.etb1_pid_derivative_filter_tau = 0.1f;

  // ---------- ETB2 PID floats ----------
  config.etb2_pid_Kp = 20.0f;
  config.etb2_pid_Ki = 1.0f;
  config.etb2_pid_Kd = 0.0f;
  config.etb2_pid_limit_integrator_min = 0.0f;
  config.etb2_pid_limit_integrator_max = 255.0f;
  config.etb2_pid_derivative_filter_tau = 0.1f;

  // ---------- Protection & fan floats ----------
  config.protection_clt_shutdown_temprature = 110.0f;
  config.protection_clt_load_disconnect_temprature = 100.0f;

  config.fan1_on_temp = 95.0f;
  config.fan1_off_temp = 90.0f;
  config.fan2_on_temp = 100.0f;
  config.fan2_off_temp = 95.0f;

  // ---------- uint16_t scalars ----------
  config.rpm_limiter = 4000;
  config.engine_displacement_cc = 2400;

  config.tps1_calib_wide_open_throttle_adc_value = 3000;
  config.tps1_calib_closed_throttle_adc_value = 1700;
  config.tps2_calib_wide_open_throttle_adc_value = 3000;
  config.tps2_calib_closed_throttle_adc_value = 1700;

  // ---------- uint8_t scalars and flags ----------
  config.firing_order = FO_1342;
  config.fuel_type = FUEL_TYPE_GAS;
  config.trigger1_type = TW_58_TOOTH_2_MISSING;
  config.trigger1_filtering = TRIGGER_FILTERING_LITE;
  config.trigger2_type = 0;
  config.trigger2_filtering = 0;
  config.trigger2_enabled = 0;

  config.number_of_injectors = 4;
  config.injection_mode = IM_SIMULTANEOUS;

  config.ignition_mode = IM_WASTED_SPARK;
  config.multi_spark_enabled = 0;
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

  config.fan1_enabled = 0;
  config.fan2_enabled = 0;

  config.tps1_type = 0;
  config.tps2_type = 0;

  // ---------- Padding ----------
  config._padding[0] = 0;
  config._padding[1] = 0;
  config._padding[2] = 0;

  // ---------- Checksum ----------
  config.checksum = 0;
}

void controller_init()
{

  /* Initialize engine stats to 0 */
  memset(&runtime, 0, sizeof(runtime));
  memset(&config, 0, sizeof(config));
  
  if (EE_Init(&config, sizeof(config_t)) != true)
  {
    controller_load_test_configuration();
    log_error("Not able to init eeprom");
  }
  controller_load_configuration();
  if (config.checksum != CONFIG_CHECKSUM) // check the data and if it is wrong, throw an error and load defaults-- add a better check
  {
    controller_load_test_configuration();
    log_error("EEPROM data is invalid");
  }
  
  //controller_load_test_configuration();
  /* Start controller timing */
  controller_timing_start(&htim2);

  /* Init analog inputs*/
  analog_inputs_init(&hadc1);

  trigger_init(&trigger1, config.trigger1_type, &config.trigger1_filtering, STATUS_TRIGGER1_SYNCED, 1);

  /* Initialize analog sensors BEGIN*/

  /* CLT */
  sensor_clt_init(&sensor_clt, config.sensor_clt_type);

  /* IAT */
  sensor_iat_init(&sensor_iat, config.sensor_iat_type);

  /* MAP */
  sensor_map_init(&sensor_map, config.sensor_map_type);

  /* OPS */
  // sensor_ops_init(&sensor_ops);

  /* Initialize analog sensors END*/

  /* Initialize fan*/

  /* Initialize ignition */

  ignition_init(ignition_output_pins);

  // Electronic Throttle Body 1
  if (config.etb1_enabled)
  {
    etb1.etb_number = ETB_NUMBER_1;
    dc_motor_init(&etb1_motor, &htim3, TIM_CHANNEL_4, TIM_CHANNEL_3, 20000);
    pid_init(&etb1_pid);
    electronic_throttle_init(&etb1, &etb1_pid, &tps1, &etb1_motor);
    electronic_throttle_set(&etb1, 0);

    static osTimerId_t etb1_software_timer;
    etb1_software_timer = osTimerNew(electronic_throttle_update, osTimerPeriodic, &etb1, NULL);
    if (etb1_software_timer != NULL)
    {
      osTimerStart(etb1_software_timer, 1);
    }
    
  }
  if (config.etb2_enabled)
  {
    /*
    etb2.etb_number = ETB_NUMBER_2;
    dc_motor_init_simple_Hbridge(&etb2_motor, &htim3, TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3, TIM_CHANNEL_4, 25000);
    pid_init(&etb2_pid);
    electronic_throttle_init(&etb2, &etb2_pid, &tps2, &etb2_motor);
    electronic_throttle_set(&etb2, 0);
    
    static osTimerId_t etb2_software_timer;
    etb2_software_timer = osTimerNew(electronic_throttle_update, osTimerPeriodic, &etb2, NULL);
    if (etb2_software_timer != NULL)
    {
      osTimerStart(etb2_software_timer, 1);
    }
    */
  }

  comms_init();


  const osThreadAttr_t controller_test_attr = {
      .name = "test_task",
      .stack_size = 1024,
      .priority = osPriorityRealtime,
  };
  const osThreadAttr_t controller_test2_attr = {
      .name = "test2_task",
      .stack_size = 1024,
      .priority = osPriorityNormal,
  };
  const osThreadAttr_t controller_sensor_task_attr = {
      .name = "sensor_task",
      .stack_size = 1024,
      .priority = osPriorityAboveNormal,
  };

  const osThreadAttr_t controller_long_routines_attr = {
      .name = "long_routines",
      .stack_size = 1024,
      .priority = osPriorityNormal,
  };


  osThreadNew(controller_sensor_task, NULL, &controller_sensor_task_attr);
  osThreadNew(controller_long_routines_task, NULL, &controller_long_routines_attr);
  osThreadNew(controller_test_task, NULL, &controller_test_attr);
  osThreadNew(controller_test2_task, NULL, &controller_test2_attr);


}

void controller_load_configuration()
{
  EE_Read();
  // Validate loaded data (checksum)
}

bool controller_save_configuration()
{
  bool state = false;
  config.checksum = CONFIG_CHECKSUM; // implement a proper checksum later
  state = EE_Write();
  return state;
}


void controller_long_routines_task(void *arg)
{
  // uint32_t next_routine_time_ticks = 1000;
  for (;;)
  {
    runtime.seconds = get_time_ms() / 1000;
    fan_control_update();
    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
    osDelay(1000);
    // next_routine_time_ticks += 1000;
    // osDelayUntil(next_routine_time_ticks);
  }
}

void controller_sensor_task(void *arg)
{
  for (;;)
  {
    runtime.tps1 = sensor_tps_get(&tps1);
    runtime.clt_degc = sensor_clt_get();
    runtime.iat_degc = sensor_iat_get();
    // runtime.map_kpa = sensor_map_get();
    // runtime.oil_pressure_ok = sensor_ops_get();
    runtime.egt_degc = sensor_egt_get();
    runtime.vbatt_volts = vbat_get();
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
    simulated_rpm = 500;
    trigger_simulator_update(simulated_rpm);
    osDelay(1);
  }
}

void controller_test2_task(void *arg)
{
  osDelay(100);
  static rpm_t simulated_rpm = 1000;
  for (;;)
  {
    percent_t etb_test_target_pos = 20;
    const percent_t etb_test_min = 0;
    const percent_t etb_test_max = 100;
    const percent_t etb_test_resolution = 0.1;
    for (etb_test_target_pos = etb_test_min; etb_test_target_pos < etb_test_max; etb_test_target_pos += etb_test_resolution)
    {
      osDelay(1);
      electronic_throttle_set(&etb1, etb_test_target_pos);
    }
    for (etb_test_target_pos = etb_test_max; etb_test_target_pos > etb_test_min; etb_test_target_pos -= etb_test_resolution)
    {
      osDelay(1);
      electronic_throttle_set(&etb1, etb_test_target_pos);
    }
    
  }
}
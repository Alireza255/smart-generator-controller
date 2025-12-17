#include "controller.h"

/**


 * @bug make composite trigger logger work
 * @bug etb 1 watchdog seems to not work!
 * @bug add all the curves to ts and make sure they are correct
 * @bug gas valve stays on after sync loss
 */

static bool controller_initialized = false;

config_t config = {0};
runtime_t runtime = {0};
diagnostics_config_t diagnostics_config = {0};

spinning_state_t diagnostic_spinning_state = 0;
governer_status_t diagnostic_governer_status = 0;

osMessageQueueId_t trigger_signal_queue_id = NULL;
static uint8_t trigger_queue_mem[FIRMWARE_TRIGGER_EVENT_QUEUE_DEPTH * sizeof(trigger_event_t)];
static uint8_t trigger_queue_cb[sizeof(StaticQueue_t)];
static const osMessageQueueAttr_t trigger_signal_queue_attr = {
    .name = "trigger queue",
    .cb_mem = trigger_queue_cb,
    .cb_size = sizeof(trigger_queue_cb),
    .mq_mem = trigger_queue_mem,
    .mq_size = sizeof(trigger_queue_mem)};

static osMessageQueueId_t controller_flags = NULL;
static uint8_t eventflags_controller_flags_cb[sizeof(StaticEventGroup_t)];
static const osEventFlagsAttr_t controller_flags_attr = {
    .name = "controller flags",
    .cb_mem = eventflags_controller_flags_cb,
    .cb_size = sizeof(eventflags_controller_flags_cb)};

static uint8_t task_trigger_driven_mem[4 * 1024];
static uint8_t task_trigger_driven_cb[sizeof(StaticTask_t)];
static const osThreadAttr_t task_trigger_driven_attr = {
    .name = "trigger driven",
    .cb_mem = task_trigger_driven_cb,
    .cb_size = sizeof(task_trigger_driven_cb),
    .stack_mem = task_trigger_driven_mem,
    .stack_size = sizeof(task_trigger_driven_mem),
    .priority = osPriorityHigh6};
void task_trigger_driven(void *arg);

static uint8_t task_controller_test_mem[512];
static uint8_t task_controller_test_cb[sizeof(StaticTask_t)];
static const osThreadAttr_t task_controller_test_attr = {
    .name = "controller test",
    .cb_mem = task_controller_test_cb,
    .cb_size = sizeof(task_controller_test_cb),
    .stack_mem = task_controller_test_mem,
    .stack_size = sizeof(task_controller_test_mem),
    .priority = osPriorityHigh7,
};
void task_controller_test(void *arg);

static uint8_t task_sensor_mem[1024];
static uint8_t task_sensor_cb[sizeof(StaticTask_t)];
static const osThreadAttr_t task_sensor_attr = {
    .name = "sensor_task",
    .cb_mem = task_sensor_cb,
    .cb_size = sizeof(task_sensor_cb),
    .stack_mem = task_sensor_mem,
    .stack_size = sizeof(task_sensor_mem),
    .priority = osPriorityHigh,
};
void task_sensor(void *arg);

static uint8_t task_long_routines_mem[512];
static uint8_t task_long_routines_cb[sizeof(StaticTask_t)];
static const osThreadAttr_t task_long_routines_attr = {
    .name = "long_routines",
    .cb_mem = task_long_routines_cb,
    .cb_size = sizeof(task_long_routines_cb),
    .stack_mem = task_long_routines_mem,
    .stack_size = sizeof(task_long_routines_mem),
    .priority = osPriorityNormal,
};
void task_long_routines(void *arg);

static uint8_t task_updater_mem[1024];
static uint8_t task_updater_cb[sizeof(StaticTask_t)];
static const osThreadAttr_t task_updater_attr = {
    .name = "updater",
    .cb_mem = task_updater_cb,
    .cb_size = sizeof(task_updater_cb),
    .stack_mem = task_updater_mem,
    .stack_size = sizeof(task_updater_mem),
    .priority = osPriorityAboveNormal,
};
void task_updater(void *arg);

sensor_tps_t tps1 = {
    .config.status_bit = STATUS_TPS1_ERROR,
    .config.analog_channel = ANALOG_INPUT_ETB1_SENSE1,
    .config.fully_closed_adc_value = 1313,
    .config.fully_open_adc_value = 3017,
    .config.is_inverted = false};
sensor_tps_t tps2 = {
    .config.status_bit = STATUS_TPS2_ERROR,
    .config.analog_channel = ANALOG_INPUT_ETB2_SENSE1,
    .config.fully_closed_adc_value = 1313,
    .config.fully_open_adc_value = 3017,
    .config.is_inverted = false};

static electronic_throttle_t etb1 = {0};
static dc_motor_t etb1_motor = {0};
static pid_t etb1_pid = {0};

static electronic_throttle_t etb2 = {0};
static dc_motor_t etb2_motor = {0};
static pid_t etb2_pid = {0};

static thermistor_t sensor_clt = {0};
static thermistor_t sensor_iat = {0};
static sensor_map_t sensor_map = {0};

void controller_configuration_load_default()
{
  const float ve_table_fill_value = 80;
  const float ign_table_fill_value = 25;
  const float fuel_blend_table_fill_value = 100;

  // x_bins: RPM axis
  for (size_t i = 0; i < TABLE_PRIMARY_SIZE_X; i++)
  {
    config.ve_table_1.x_bins[i] = i * FIRMWARE_LIMIT_MAX_RPM / TABLE_PRIMARY_SIZE_X;
    config.ve_table_2.x_bins[i] = i * FIRMWARE_LIMIT_MAX_RPM / TABLE_PRIMARY_SIZE_X;
    config.ign_table_1.x_bins[i] = i * FIRMWARE_LIMIT_MAX_RPM / TABLE_PRIMARY_SIZE_X;
    config.ign_table_2.x_bins[i] = i * FIRMWARE_LIMIT_MAX_RPM / TABLE_PRIMARY_SIZE_X;
    config.fuel_blend_table.x_bins[i] = i * FIRMWARE_LIMIT_MAX_RPM / TABLE_PRIMARY_SIZE_X;
  }

  // y_bins: MAP/load axis
  for (size_t i = 0; i < TABLE_PRIMARY_SIZE_Y; i++)
  {
    config.ve_table_1.y_bins[i] = i * FIRMWARE_LIMIT_MAX_MAP / TABLE_PRIMARY_SIZE_Y;
    config.ve_table_2.y_bins[i] = i * FIRMWARE_LIMIT_MAX_MAP / TABLE_PRIMARY_SIZE_Y;
    config.ign_table_1.y_bins[i] = i * FIRMWARE_LIMIT_MAX_MAP / TABLE_PRIMARY_SIZE_Y;
    config.ign_table_2.y_bins[i] = i * FIRMWARE_LIMIT_MAX_MAP / TABLE_PRIMARY_SIZE_Y;
    config.fuel_blend_table.y_bins[i] = i * FIRMWARE_LIMIT_MAX_MAP / TABLE_PRIMARY_SIZE_Y;
  }

  for (size_t i = 0; i < TABLE_PRIMARY_SIZE_X; i++)
  {
    for (size_t j = 0; j < TABLE_PRIMARY_SIZE_Y; j++)
    {
      config.ve_table_1.data[i][j] = ve_table_fill_value;
      config.ve_table_2.data[i][j] = ve_table_fill_value;
      config.ign_table_1.data[i][j] = ign_table_fill_value;
      config.ign_table_2.data[i][j] = ign_table_fill_value;
      config.fuel_blend_table.data[i][j] = fuel_blend_table_fill_value;
    }
  }

  for (size_t i = 0; i < TABLE_PRIMARY_SIZE_X; i++)
  {
    config.accel_enrichment_tps_table.data[i] = 0;
    config.accel_enrichment_tps_table.x_bins[i] = (float)((i - (TABLE_PRIMARY_SIZE_X / 2)) / (TABLE_PRIMARY_SIZE_X / 2) * 2000);
    config.accel_enrichment_map_table.data[i] = 0;
    config.accel_enrichment_map_table.x_bins[i] = (float)((i - (TABLE_PRIMARY_SIZE_X / 2)) / (TABLE_PRIMARY_SIZE_X / 2) * 2000);
  }

  for (size_t i = 0; i < TABLE_PRIMARY_SIZE_X; i++)
  {
    config.clt_based_advance_correction_table.data[i] = 0;
    config.clt_based_fuel_correction_table_gas.data[i] = 0;
    config.clt_based_fuel_correction_table_petrol.data[i] = 0;
    float x = mapf((float)i, 0, TABLE_PRIMARY_SIZE_X - 1, FIRMWARE_LIMIT_MIN_TEMP, FIRMWARE_LIMIT_MAX_TEMP);
    config.clt_based_advance_correction_table.x_bins[i] = x;
    config.clt_based_fuel_correction_table_gas.x_bins[i] = x;
    config.clt_based_fuel_correction_table_petrol.x_bins[i] = x;
  }

  config.cranking_rpm_threshold = 400.0f;
  config.cranking_advance = 10.0f;
  config.cranking_throttle = 15.0f;
  config.trigger_offset_deg = 114.0f;

  config.ignition_dwell = 4.0f;
  config.ignition_fixed_timing_advance = 25.0f;
  config.multi_spark_rpm_threshold = 2000.0;
  config.multi_spark_rest_time_ms = 0.2f;
  config.multi_spark_max_trailing_angle = 15.0f;

  config.stoich_afr_gas = 12.0;
  config.stoich_afr_petrol = 14.0;

  config.gas_control_etb_flowrate_grams_per_sec = 8.6f;
  config.gas_reference_pressure = 2.0f;
  config.gas_priming_time_sec = 2.0f;
  config.gas_control_etb_priming_position_percent = 20.0f;
  config.petrol_priming_time_sec = 5.0f;

  config.injection_end_of_injection_angle = 180.0f;
  config.injector_flow_rate_cc_per_min = 250.0f;

  config.governer_target_rpm = 1500.0f;
  config.governer_idle_rpm = 850.0f;
  config.governer_pid_parameters.Kp = 0.01f;
  config.governer_pid_parameters.Ki = 0.005f;
  config.governer_pid_parameters.Kd = 0.001f;
  config.governer_pid_parameters.limit_integrator_max = 100.0f;
  config.governer_pid_parameters.limit_integrator_min = -100.0f;
  config.governer_pid_parameters.derivative_filter_tau = 0.01f;

  
  config.etb1_pid_parameters.Kp = 7.5f;
  config.etb1_pid_parameters.Ki = 5.0f;
  config.etb1_pid_parameters.Kd = 0.01f;
  config.etb1_pid_parameters.limit_integrator_min = -100.0f;
  config.etb1_pid_parameters.limit_integrator_max = 100.0f;
  config.etb1_pid_parameters.derivative_filter_tau = 0.01f;
  config.etb1_end_of_travel_duty_cycle_limit_upper = 70.0f;
  config.etb1_end_of_travel_duty_cycle_limit_lower = 70.0f;

  config.etb2_pid_parameters.Kp = 7.5f;
  config.etb2_pid_parameters.Ki = 5.0f;
  config.etb2_pid_parameters.Kd = 0.01f;
  config.etb2_pid_parameters.limit_integrator_min = -100.0f;
  config.etb2_pid_parameters.limit_integrator_max = 100.0f;
  config.etb2_pid_parameters.derivative_filter_tau = 0.01f;
  config.etb2_end_of_travel_duty_cycle_limit_upper = 70.0f;
  config.etb2_end_of_travel_duty_cycle_limit_lower = 70.0f;

  config.protection_clt_shutdown_temprature = 110.0f;
  config.protection_clt_load_disconnect_enabled = 105.0f;
  config.fan1_on_temp = 84.0f;
  config.fan1_off_temp = 80.0f;
  config.fan2_on_temp = 88.0f;
  config.fan2_off_temp = 84.0f;

  config.rev_limit = 2500;
  config.rev_limit_hystersis = 500;
  config.engine_displacement_cc = 2400;

  config.tps1_calib_wide_open_throttle_adc_value = 3817;
  config.tps1_calib_closed_throttle_adc_value = 417;
  config.tps2_calib_wide_open_throttle_adc_value = 3817;
  config.tps2_calib_closed_throttle_adc_value = 417;

  config.firing_order = FO_1342;
  config.fuel_type = FUEL_TYPE_GAS;
  config.trigger_crank_type = TRIGGER_WHEEL_TYPE_CRANK_58_TOOTH_2_MISSING;
  config.trigger_crank_filtering = TRIGGER_FILTERING_NONE;
  config.trigger_cam_type = TRIGGER_WHEEL_TYPE_CAM_NO_WHEEL;
  config.trigger_cam_filtering = TRIGGER_FILTERING_NONE;
  config.trigger_cam_enabled = false;

  config.injection_mode = INJECTION_MODE_NO_FUEL_INJECTION;
  config.ignition_mode = IGNITION_MODE_WASTED_SPARK;
  config.ignition_fixed_timing_enabled = false;

  config.multi_spark_enabled = false;
  config.multi_spark_number_of_sparks = 3;

  config.tps1_calib_is_inverted = false;
  config.tps2_calib_is_inverted = false;
  config.tps1_type = SENSOR_TPS_TYPE_SAMAND_ETB;
  config.tps2_type = SENSOR_TPS_TYPE_SAMAND_ETB;

  config.sensor_clt_type = SENSOR_CLT_TYPE_NISSAN;
  config.sensor_iat_type = SENSOR_IAT_TYPE_BOSCH_816;
  config.sensor_map_type = SENSOR_MAP_TYPE_BOSCH_816;

  config.protection_oil_pressure_time = 3;
  config.protection_oil_pressure_enabled = false;
  config.protection_clt_enabled = false;
  config.protection_clt_load_disconnect_enabled = false;

  config.etb1_enabled = true;
  config.etb2_enabled = true;
  config.etb1_motor_inverted = false;
  config.etb2_motor_inverted = false;
  config.etb1_end_of_travel_duty_cycle_limit_enabled = false;
  config.etb2_end_of_travel_duty_cycle_limit_enabled = false;

  config.fan1_enabled = true;
  config.fan2_enabled = true;

  diagnostics_config.timing_light_enabled = false;
  diagnostics_config.trigger_light_enabled = false;
}

void controller_init()
{

  /* Initialize engine stats to 0 */
  memset(&runtime, 0, sizeof(runtime));
  memset(&config, 0, sizeof(config));

  if (EE_Init(&config, sizeof(config_t)) != true)
  {
    controller_configuration_load_default();
    log_error("Not able to init eeprom");
  }
  controller_load_configuration();

  /* Start controller timing */
  controller_timing_start(&htim2);

  controller_flags = osEventFlagsNew(&controller_flags_attr);
  if (controller_flags == NULL)
  {
    /**
     * @todo add proper handling here
     */
    __NOP();
  }

  /* Init analog inputs*/
  analog_inputs_init(&hadc1, &htim5);

  if (config.trigger_cam_enabled)
    trigger_camshaft_init(config.trigger_cam_type, &config.trigger_cam_filtering);

  trigger_crankshaft_init(config.trigger_crank_type);
  trigger_signal_queue_id = osMessageQueueNew(FIRMWARE_TRIGGER_EVENT_QUEUE_DEPTH, sizeof(trigger_event_t), &trigger_signal_queue_attr);
  if (trigger_signal_queue_id == NULL)
  {
    /**
     * @todo add proper handling here
     */
  }
  osThreadNew(task_trigger_driven, NULL, &task_trigger_driven_attr);

  /* Initialize analog sensors BEGIN*/

  sensor_tps_init(&tps1, STATUS_TPS1_ERROR);
  sensor_tps_init(&tps2, STATUS_TPS2_ERROR);

  /* CLT */
  sensor_clt_init(&sensor_clt, config.sensor_clt_type);

  /* IAT */
  sensor_iat_init(&sensor_iat, config.sensor_iat_type);

  /* MAP */
  sensor_map_init(&sensor_map, config.sensor_map_type);

  /* Initialize analog sensors END*/

  fuel_math_use_tps_map_sensors(&tps1, &sensor_map);

  /* Initialize ignition */

  ignition_init();
  injection_init();

  osDelay(500);
  // Electronic Throttle Body 1
  if (config.etb1_enabled)
  {
    if (config.etb1_motor_inverted)
    {
      dc_motor_init(&etb1_motor, &htim3, TIM_CHANNEL_4, TIM_CHANNEL_3, 20000);
    }
    else
    {
      dc_motor_init(&etb1_motor, &htim3, TIM_CHANNEL_3, TIM_CHANNEL_4, 20000);
    }

    pid_init(&etb1_pid, &config.etb1_pid_parameters);
    electronic_throttle_init(&etb1, &etb1_pid, &tps1, &etb1_motor, &config.etb1_feedforward_table, STATUS_ETB1_OK);

    if (config.etb1_end_of_travel_duty_cycle_limit_enabled)
    {
      electronic_throttle_enable_end_of_travel_protection(&etb1, (percent_t)config.etb1_end_of_travel_duty_cycle_limit_lower, (percent_t)config.etb1_end_of_travel_duty_cycle_limit_upper);
    }
    electronic_throttle_set(&etb1, config.cranking_throttle);
    /* Init governer */
    if (config.etb1_enabled)
    {
      governer_init(&etb1, &config.governer_pid_parameters);
    }
  }
  if (config.etb2_enabled)
  {

    if (config.etb2_motor_inverted)
    {
      dc_motor_init(&etb2_motor, &htim3, TIM_CHANNEL_2, TIM_CHANNEL_1, 20000);
    }
    else
    {
      dc_motor_init(&etb2_motor, &htim3, TIM_CHANNEL_1, TIM_CHANNEL_2, 20000);
    }

    pid_init(&etb2_pid, &config.etb2_pid_parameters);
    electronic_throttle_init(&etb2, &etb2_pid, &tps2, &etb2_motor, &config.etb2_feedforward_table, STATUS_ETB2_OK);
    if (config.etb2_end_of_travel_duty_cycle_limit_enabled)
    {
      electronic_throttle_enable_end_of_travel_protection(&etb2, (percent_t)config.etb2_end_of_travel_duty_cycle_limit_lower, (percent_t)config.etb2_end_of_travel_duty_cycle_limit_upper);
    }
    electronic_throttle_set(&etb2, 0);

    gas_injection_init(&etb2);
  }
  protections_init(&etb1);
  comms_init();

  osThreadNew(task_updater, NULL, &task_updater_attr);
  osThreadNew(task_controller_test, NULL, &task_controller_test_attr);
  osThreadNew(task_sensor, NULL, &task_sensor_attr);
  osThreadNew(task_long_routines, NULL, &task_long_routines_attr);

  osEventFlagsSet(controller_flags, CONTROLLER_FLAG_INIT_DONE);
}

void controller_load_configuration()
{
  EE_Read();
  // Validate loaded data (checksum)
}

bool controller_save_configuration()
{
  bool state = false;
  state = EE_Write();
  return state;
}

void task_long_routines(void *arg)
{
  // uint32_t next_routine_time_ticks = 1000;
  for (;;)
  {
    runtime.seconds = get_time_ms() / 1000;
    fan_control_update();
    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
    osDelay(1000);
  }
}

void controller_analog_inputs_read_callback()
{
  osEventFlagsSet(controller_flags, CONTROLLER_FLAG_ANALOG_DATA_AVAILABLE);
}

void task_sensor(void *arg)
{
  for (;;)
  {
    osEventFlagsWait(controller_flags, CONTROLLER_FLAG_ANALOG_DATA_AVAILABLE ^ CONTROLLER_FLAG_INIT_DONE, osFlagsWaitAny, osWaitForever);
    osEventFlagsClear(controller_flags, CONTROLLER_FLAG_ANALOG_DATA_AVAILABLE);

    time_us_t timestamp = get_time_us();
    sensor_tps_update(&tps1, timestamp);
    sensor_tps_update(&tps2, timestamp);
    sensor_map_update(timestamp);
    sensor_iat_update();
    sensor_clt_update();
    sensor_ops_update(timestamp);
    vbat_update();

    runtime.tps1 = sensor_tps_get(&tps1);
    runtime.tps2 = sensor_tps_get(&tps2);
    runtime.tps1_dot = sensor_tps_rate_of_change_get(&tps1);
    runtime.map_dot = sensor_map_rate_of_change_get(&sensor_map);
    runtime.lambda = 1;
    runtime.dwell_actual = ignition_dwell_get();
    runtime.clt_degc = sensor_clt_get();
    runtime.iat_degc = sensor_iat_get();
    runtime.map_kpa = sensor_map_get();
    runtime.egt_degc = sensor_egt_get();
    runtime.vbatt_volts = vbat_get();
    runtime.ignition_advance_deg = ignition_get_advance();
    runtime.etb1_target = etb1.target_position;
    runtime.etb1_motor_effort = etb1.motor->current_direction ? (float)-1 * (float)(etb1.motor->current_duty_cycle) : (float)(etb1.motor->current_duty_cycle);
    runtime.etb2_target = etb2.target_position;
    runtime.etb2_motor_effort = etb2.motor->current_direction ? (float)-1 * (float)(etb2.motor->current_duty_cycle) : (float)(etb2.motor->current_duty_cycle);

    change_bit(&runtime.status, STATUS_GAS_SOLENOID_ON, output_get_state(&gas_solenoid_output));
    
    diagnostic_spinning_state = trigger_spinning_state_get();
    diagnostic_governer_status = governer_get_status();
  }
}
void task_controller_test(void *arg)
{
  osDelay(100);
  trigger_simulator_crank_init(TRIGGER_SIMULATOR_WHEEL_TYPE_60_2, trigger_signal_queue_id);
  for (;;)
  {
    trigger_simulator_update();
  }
}
void task_updater(void *arg)
{
  for (;;)
  {
    time_us_t timestamp = get_time_us();
    governer_update(timestamp);
    gas_injection_update(timestamp);
    electronic_throttle_update(&etb1, timestamp);
    electronic_throttle_update(&etb2, timestamp);
    protections_update(timestamp);
    osDelay(1);
  }
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  trigger_event_t event = {0};
  event.timestamp = get_time_us();
  bool crank_trigger_input_state = HAL_GPIO_ReadPin(SENSOR_VR1_GPIO_Port, SENSOR_VR1_Pin);
  bool cam_trigger_input_state = HAL_GPIO_ReadPin(SENSOR_VR2_GPIO_Port, SENSOR_VR2_Pin);
  if (GPIO_Pin == SENSOR_VR1_Pin && !crank_trigger_input_state)
  {
    event.type = TRIGGER_EVENT_TYPE_CRANKSHAFT;
    event.edge = crank_trigger_input_state;
  }
  else if (GPIO_Pin == SENSOR_VR2_Pin)
  {
    event.type = TRIGGER_EVENT_TYPE_CAMSHAFT;
    event.edge = cam_trigger_input_state;
  }
  osStatus_t status = osMessageQueuePut(trigger_signal_queue_id, &event, 0, 0);
  if (status == osErrorResource)
  {
    /* means queue was full and processing has totally fallen behind queu */
    /**
     * @todo handle this properly
     */
  }
}

void task_trigger_driven(void *arg)
{
  trigger_event_t event;
  osStatus_t status;
  for (;;)
  {
    status = osMessageQueueGet(trigger_signal_queue_id, &event, NULL, FTRMWARE_TRIGGER_EVENT_TIMEOUT_MS);
    if (status == osErrorTimeout)
    {
      event.edge = TRIGGER_EVENT_EDGE_NONE;
      event.type = TRIGGER_EVENT_TYPE_TIMEOUT;
      event.timestamp = get_time_us();
    }

    switch (event.type)
    {
    case TRIGGER_EVENT_TYPE_CRANKSHAFT:
      if (event.edge == TRIGGER_EVENT_EDGE_FALLING)
        trigger_crank_handle(&event);
      trigger_logger_entry_add_tooth(event.timestamp);

    case TRIGGER_EVENT_TYPE_CAMSHAFT:
      /**
       * @todo implement cam handling
       */
      break;
    case TRIGGER_EVENT_TYPE_TIMEOUT:
      trigger_crank_handle(&event);
      break;
    default:
      break;
    }

    bool is_crankshaft_synced = get_bit(runtime.status, STATUS_TRIGGER_CRANKSHAFT_SYNCED);
    if (is_crankshaft_synced)
    {
      rpm_t rpm = crankshaft_get_rpm();
      angle_t crank_angle = crankshaft_get_angle();
      time_us_t timestamp = event.timestamp;
      ignition_trigger_event_handle(crank_angle, rpm, timestamp);
      injection_trigger_event_handle(crank_angle, rpm, timestamp);
      /**
       * @remark gas injection update should be here ?
       * @remark governer update should be here ?
       */
    }
  }
}
void task_controller_cmds(void *arg)
{
  for (;;)
  {
    uint32_t command = osEventFlagsWait(controller_command_flag, 0x0000FFFF, osFlagsWaitAny, osWaitForever);
    switch (command)
    {
    case TS_CONTROLLER_COMMAND_REBOOT:
      NVIC_SystemReset();
      break;
    case TS_CONTROLLER_COMMAND_AUTO_CALIB_ETB1:
      if (config.etb1_enabled)
        electronic_throttle_auto_tune(&etb1);
      break;
    case TS_CONTROLLER_COMMAND_AUTO_CALIB_ETB2:
      if (config.etb2_enabled)
        electronic_throttle_auto_tune(&etb2);
      break;
    case TS_CONTROLLER_COMMAND_TPS1_CALIB_SAVE_FROM_AUTO_CALIB:
      config.tps1_calib_closed_throttle_adc_value = tps1.config.fully_closed_adc_value;
      config.tps1_calib_wide_open_throttle_adc_value = tps1.config.fully_open_adc_value;
      config.tps1_calib_is_inverted = tps1.config.is_inverted;
      break;
    case TS_CONTROLLER_COMMAND_TPS2_CALIB_SAVE_FROM_AUTO_CALIB:
      config.tps2_calib_closed_throttle_adc_value = tps2.config.fully_closed_adc_value;
      config.tps2_calib_wide_open_throttle_adc_value = tps2.config.fully_open_adc_value;
      config.tps2_calib_is_inverted = tps2.config.is_inverted;
      break;

    case TS_CONTROLLER_COMMAND_TRIGGER_SIMULATOR_STOP:
      trigger_simulator_stop();
      break;
    case TS_CONTROLLER_COMMAND_TRIGGER_SIMULATOR_RPM_100:
      trigger_simulator_set_rpm_and_start(100);
      break;
    case TS_CONTROLLER_COMMAND_TRIGGER_SIMULATOR_RPM_250:
      trigger_simulator_set_rpm_and_start(250);
      break;
    case TS_CONTROLLER_COMMAND_TRIGGER_SIMULATOR_RPM_500:
      trigger_simulator_set_rpm_and_start(500);
      break;

    case TS_CONTROLLER_COMMAND_IGNITION_FIRE_COIL_1:
      output_fire_ignition(&ignition_output[0]);
      break;

    case TS_CONTROLLER_COMMAND_IGNITION_FIRE_COIL_2:
      output_fire_ignition(&ignition_output[1]);
      break;

    case TS_CONTROLLER_COMMAND_IGNITION_FIRE_COIL_3:
      output_fire_ignition(&ignition_output[2]);
      break;

    case TS_CONTROLLER_COMMAND_IGNITION_FIRE_COIL_4:
      output_fire_ignition(&ignition_output[3]);
      break;

    case TS_CONTROLLER_COMMAND_INJECTION_FIRE_INJECTOR_1:
      output_fire_injection(&injector_output[0]);
      break;

    case TS_CONTROLLER_COMMAND_INJECTION_FIRE_INJECTOR_2:
      output_fire_injection(&injector_output[1]);
      break;

    case TS_CONTROLLER_COMMAND_INJECTION_FIRE_INJECTOR_3:
      output_fire_injection(&injector_output[2]);
      break;

    case TS_CONTROLLER_COMMAND_INJECTION_FIRE_INJECTOR_4:
      output_fire_injection(&injector_output[3]);
      break;

    case TS_CONTROLLER_COMMAND_FAN1_ON:
      output_override(&fan1_output, true);
      break;

    case TS_CONTROLLER_COMMAND_FAN1_OFF:
      output_override_clear(&fan1_output);
      break;

    case TS_CONTROLLER_COMMAND_FAN2_ON:
      output_override(&fan2_output, true);
      break;

    case TS_CONTROLLER_COMMAND_FAN2_OFF:
      output_override_clear(&fan2_output);
      break;

    case TS_CONTROLLER_COMMAND_MAIN_RELAY_ON:
      output_override(&main_relay_output, true);
      break;

    case TS_CONTROLLER_COMMAND_MAIN_RELAY_OFF:
      output_override_clear(&main_relay_output);
      break;

    case TS_CONTROLLER_COMMAND_GAS_SOLENOID_ON:
      output_override(&gas_solenoid_output, true);
      break;

    case TS_CONTROLLER_COMMAND_GAS_SOLENOID_OFF:
      output_override_clear(&gas_solenoid_output);
      break;
    case TS_CONTROLLER_COMMAND_TIMING_LIGHT_ON:
      diagnostics_config.timing_light_enabled = true;
      trigger_logger_composite_start();
      break;
    case TS_CONTROLLER_COMMAND_TIMING_LIGHT_OFF:
      diagnostics_config.timing_light_enabled = false;
      break;
    case TS_CONTROLLER_COMMAND_TRIGGER_LIGHT_ON:
      diagnostics_config.trigger_light_enabled = true;
      break;
    case TS_CONTROLLER_COMMAND_TRIGGER_LIGHT_OFF:
      diagnostics_config.trigger_light_enabled = false;
      break;
    case TS_CONTROLLER_COMMAND_LOAD_DEFAULT_CONFIG:
      controller_configuration_load_default();
      break;
    default:
      break;
    }
    osEventFlagsClear(controller_command_flag, 0x0000FFFF);
  }
}
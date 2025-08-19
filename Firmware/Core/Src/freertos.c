/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "controller.h"
#include "trigger_simulator.h"
#include "utils.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

table_2d_t test_table = {0};
float table_value = 0.0f;
/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityRealtime,
};
/* Definitions for trig_sim */
osThreadId_t trig_simHandle;
const osThreadAttr_t trig_sim_attributes = {
  .name = "trig_sim",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for fuel_task */
osThreadId_t fuel_taskHandle;
const osThreadAttr_t fuel_task_attributes = {
  .name = "fuel_task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for init_task */
osThreadId_t init_taskHandle;
const osThreadAttr_t init_task_attributes = {
  .name = "init_task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityRealtime,
};
/* Definitions for sensor_task */
osThreadId_t sensor_taskHandle;
const osThreadAttr_t sensor_task_attributes = {
  .name = "sensor_task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for engine_flags */
osEventFlagsId_t engine_flagsHandle;
const osEventFlagsAttr_t engine_flags_attributes = {
  .name = "engine_flags"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

void toggle_led();

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void trigger_simulator_task(void *argument);
void start_fuel_task(void *argument);
void controller_init_task(void *argument);
void sensors_task(void *argument);

extern void MX_USB_DEVICE_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of trig_sim */
  trig_simHandle = osThreadNew(trigger_simulator_task, NULL, &trig_sim_attributes);

  /* creation of fuel_task */
  fuel_taskHandle = osThreadNew(start_fuel_task, NULL, &fuel_task_attributes);

  /* creation of init_task */
  init_taskHandle = osThreadNew(controller_init_task, NULL, &init_task_attributes);

  /* creation of sensor_task */
  sensor_taskHandle = osThreadNew(sensors_task, NULL, &sensor_task_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Create the event(s) */
  /* creation of engine_flags */
  engine_flagsHandle = osEventFlagsNew(&engine_flags_attributes);

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN StartDefaultTask */

  osDelay(100);
  
  /* Infinite loop */
  for(;;)
  {

    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_trigger_simulator_task */
/**
* @brief Function implementing the trig_sim thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_trigger_simulator_task */
void trigger_simulator_task(void *argument)
{
  /* USER CODE BEGIN trigger_simulator_task */

  /* Infinite loop */
  for(;;)
  {
    osDelay(1);

  }
  /* USER CODE END trigger_simulator_task */
}

/* USER CODE BEGIN Header_start_fuel_task */
/**
* @brief Function implementing the fuel_task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_start_fuel_task */
void start_fuel_task(void *argument)
{
  /* USER CODE BEGIN start_fuel_task */


  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END start_fuel_task */
}

/* USER CODE BEGIN Header_controller_init_task */
/**
* @brief Function implementing the init_task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_controller_init_task */
void controller_init_task(void *argument)
{
  /* USER CODE BEGIN controller_init_task */

  controller_init_with_defaults();


  for (size_t i = 0; i < (sizeof(test_table.x_bins) / sizeof(test_table.x_bins[0])); i++)
  {
    test_table.x_bins[i] = i * 100;  // Example values
  }
  for (size_t i = 0; i < (sizeof(test_table.y_bins) / sizeof(test_table.y_bins[0])); i++)
  {
    test_table.y_bins[i] = i * 10;  // Example values
  }
  for (size_t i = 0; i < (sizeof(test_table.data) / sizeof(test_table.data[0])); i++)
  {
    for (size_t j = 0; j < (sizeof(test_table.data[0]) / sizeof(test_table.data[0][0])); j++)
    {
      test_table.data[i][j] = (i + 1) * (j + 1);  // Example values
    }
  }

  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
    static float rpm = 0;
    static float load = 0;
    rpm += 10;  // Simulate RPM increase
    load = 11.25f;  // Simulate load increase
    if (rpm > 5000)
    {
      rpm = -1000;
    }
    table_value = table_2d_get_value(&test_table, rpm, load);

    
  }
  /* USER CODE END controller_init_task */
}

/* USER CODE BEGIN Header_sensors_task */
/**
* @brief Function implementing the sensor_task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_sensors_task */
void sensors_task(void *argument)
{
  /* USER CODE BEGIN sensors_task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);

    //runtime.map_kpa = sensor_map_get();
  }
  /* USER CODE END sensors_task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void toggle_led()
{
  HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
}
/* USER CODE END Application */


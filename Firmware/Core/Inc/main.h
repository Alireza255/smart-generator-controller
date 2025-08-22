/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define SENSOR_EGT_PIN_Pin GPIO_PIN_0
#define SENSOR_EGT_PIN_GPIO_Port GPIOC
#define ETB1_SENSE1_Pin GPIO_PIN_1
#define ETB1_SENSE1_GPIO_Port GPIOC
#define ETB1_SENSE2_Pin GPIO_PIN_2
#define ETB1_SENSE2_GPIO_Port GPIOC
#define ETB2_SENSE1_Pin GPIO_PIN_3
#define ETB2_SENSE1_GPIO_Port GPIOC
#define ETB2_SENSE2_Pin GPIO_PIN_0
#define ETB2_SENSE2_GPIO_Port GPIOA
#define SENSOR_MAP_PIN_Pin GPIO_PIN_1
#define SENSOR_MAP_PIN_GPIO_Port GPIOA
#define SENSOR_IAT_PIN_Pin GPIO_PIN_2
#define SENSOR_IAT_PIN_GPIO_Port GPIOA
#define SENSOR_CLT_PIN_Pin GPIO_PIN_3
#define SENSOR_CLT_PIN_GPIO_Port GPIOA
#define SENSOR_OIL_PIN_Pin GPIO_PIN_4
#define SENSOR_OIL_PIN_GPIO_Port GPIOA
#define VBAT_SENSE_PIN_Pin GPIO_PIN_5
#define VBAT_SENSE_PIN_GPIO_Port GPIOA
#define AD_INPUT_1_Pin GPIO_PIN_6
#define AD_INPUT_1_GPIO_Port GPIOA
#define AD_INPUT_2_Pin GPIO_PIN_7
#define AD_INPUT_2_GPIO_Port GPIOA
#define AD_INPUT_3_Pin GPIO_PIN_4
#define AD_INPUT_3_GPIO_Port GPIOC
#define LOW_SIDE_1_Pin GPIO_PIN_0
#define LOW_SIDE_1_GPIO_Port GPIOB
#define LOW_SIDE_2_Pin GPIO_PIN_1
#define LOW_SIDE_2_GPIO_Port GPIOB
#define FAN_1_Pin GPIO_PIN_2
#define FAN_1_GPIO_Port GPIOB
#define FAN_2_Pin GPIO_PIN_10
#define FAN_2_GPIO_Port GPIOB
#define AUX_RELAY_Pin GPIO_PIN_12
#define AUX_RELAY_GPIO_Port GPIOB
#define GAS_SOLENOID_RELAY_Pin GPIO_PIN_13
#define GAS_SOLENOID_RELAY_GPIO_Port GPIOB
#define FUEL_PUMP_RELAY_Pin GPIO_PIN_14
#define FUEL_PUMP_RELAY_GPIO_Port GPIOB
#define MAIN_RELAY_Pin GPIO_PIN_15
#define MAIN_RELAY_GPIO_Port GPIOB
#define DC_MOTOR2_1_Pin GPIO_PIN_6
#define DC_MOTOR2_1_GPIO_Port GPIOC
#define DC_MOTOR2_2_Pin GPIO_PIN_7
#define DC_MOTOR2_2_GPIO_Port GPIOC
#define DC_MOTOR1_1_Pin GPIO_PIN_8
#define DC_MOTOR1_1_GPIO_Port GPIOC
#define DC_MOTOR1_2_Pin GPIO_PIN_9
#define DC_MOTOR1_2_GPIO_Port GPIOC
#define PWM_OUT_1_Pin GPIO_PIN_8
#define PWM_OUT_1_GPIO_Port GPIOA
#define PWM_OUT_2_Pin GPIO_PIN_9
#define PWM_OUT_2_GPIO_Port GPIOA
#define PWM_OUT_3_Pin GPIO_PIN_10
#define PWM_OUT_3_GPIO_Port GPIOA
#define INJECTOR_OUTPUT_4_Pin GPIO_PIN_15
#define INJECTOR_OUTPUT_4_GPIO_Port GPIOA
#define INJECTOR_OUTPUT_3_Pin GPIO_PIN_10
#define INJECTOR_OUTPUT_3_GPIO_Port GPIOC
#define INJECTOR_OUTPUT_2_Pin GPIO_PIN_11
#define INJECTOR_OUTPUT_2_GPIO_Port GPIOC
#define INJECTOR_OUTPUT_1_Pin GPIO_PIN_12
#define INJECTOR_OUTPUT_1_GPIO_Port GPIOC
#define IGNITION_OUTPUT_4_Pin GPIO_PIN_2
#define IGNITION_OUTPUT_4_GPIO_Port GPIOD
#define IGNITION_OUTPUT_3_Pin GPIO_PIN_3
#define IGNITION_OUTPUT_3_GPIO_Port GPIOB
#define IGNITION_OUTPUT_2_Pin GPIO_PIN_4
#define IGNITION_OUTPUT_2_GPIO_Port GPIOB
#define IGNITION_OUTPUT_1_Pin GPIO_PIN_5
#define IGNITION_OUTPUT_1_GPIO_Port GPIOB
#define SENSOR_VR_1_Pin GPIO_PIN_8
#define SENSOR_VR_1_GPIO_Port GPIOB
#define SENSOR_VR2_Pin GPIO_PIN_9
#define SENSOR_VR2_GPIO_Port GPIOB
#define SENSOR_VR2_EXTI_IRQn EXTI9_5_IRQn

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

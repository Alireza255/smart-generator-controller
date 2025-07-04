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
#define AD_INPUT1_PIN_Pin GPIO_PIN_0
#define AD_INPUT1_PIN_GPIO_Port GPIOC
#define AD_INPUT2_PIN_Pin GPIO_PIN_1
#define AD_INPUT2_PIN_GPIO_Port GPIOC
#define AD_INPUT3_PIN_Pin GPIO_PIN_2
#define AD_INPUT3_PIN_GPIO_Port GPIOC
#define AD_INPUT4_PIN_Pin GPIO_PIN_3
#define AD_INPUT4_PIN_GPIO_Port GPIOC
#define ETB1_SENSE1_Pin GPIO_PIN_0
#define ETB1_SENSE1_GPIO_Port GPIOA
#define ETB1_SENSE2_Pin GPIO_PIN_1
#define ETB1_SENSE2_GPIO_Port GPIOA
#define ETB2_SENSE1_Pin GPIO_PIN_2
#define ETB2_SENSE1_GPIO_Port GPIOA
#define ETB2_SENSE2_Pin GPIO_PIN_3
#define ETB2_SENSE2_GPIO_Port GPIOA
#define SENSOR_MAP_PIN_Pin GPIO_PIN_4
#define SENSOR_MAP_PIN_GPIO_Port GPIOA
#define SENSOR_IAT_PIN_Pin GPIO_PIN_5
#define SENSOR_IAT_PIN_GPIO_Port GPIOA
#define SENSOR_CLT_PIN_Pin GPIO_PIN_6
#define SENSOR_CLT_PIN_GPIO_Port GPIOA
#define SENSOR_OIL_PIN_Pin GPIO_PIN_7
#define SENSOR_OIL_PIN_GPIO_Port GPIOA
#define VBAT_SENSE_PIN_Pin GPIO_PIN_4
#define VBAT_SENSE_PIN_GPIO_Port GPIOC
#define PWM_OUT3_Pin GPIO_PIN_0
#define PWM_OUT3_GPIO_Port GPIOB
#define PWM_OUT4_Pin GPIO_PIN_1
#define PWM_OUT4_GPIO_Port GPIOB
#define FAN1_Pin GPIO_PIN_2
#define FAN1_GPIO_Port GPIOB
#define FAN2_Pin GPIO_PIN_10
#define FAN2_GPIO_Port GPIOB
#define IGNITION_OUTPUT4_Pin GPIO_PIN_12
#define IGNITION_OUTPUT4_GPIO_Port GPIOB
#define IGNITION_OUTPUT3_Pin GPIO_PIN_13
#define IGNITION_OUTPUT3_GPIO_Port GPIOB
#define IGNITION_OUTPUT2_Pin GPIO_PIN_14
#define IGNITION_OUTPUT2_GPIO_Port GPIOB
#define IGNITION_OUTPUT1_Pin GPIO_PIN_15
#define IGNITION_OUTPUT1_GPIO_Port GPIOB
#define ETB1_MOTOR1_Pin GPIO_PIN_8
#define ETB1_MOTOR1_GPIO_Port GPIOA
#define ETB1_MOTOR2_Pin GPIO_PIN_9
#define ETB1_MOTOR2_GPIO_Port GPIOA
#define PWM_OUT1_Pin GPIO_PIN_4
#define PWM_OUT1_GPIO_Port GPIOB
#define PWM_OUT2_Pin GPIO_PIN_5
#define PWM_OUT2_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

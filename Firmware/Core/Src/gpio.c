/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LED_Pin|INJECTOR_OUTPUT_3_Pin|INJECTOR_OUTPUT_2_Pin|INJECTOR_OUTPUT_1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LOW_SIDE_1_Pin|LOW_SIDE_2_Pin|FAN_1_Pin|FAN_2_Pin
                          |AUX_RELAY_Pin|GAS_SOLENOID_RELAY_Pin|FUEL_PUMP_RELAY_Pin|MAIN_RELAY_Pin
                          |IGNITION_OUTPUT_3_Pin|IGNITION_OUTPUT_2_Pin|IGNITION_OUTPUT_1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(INJECTOR_OUTPUT_4_GPIO_Port, INJECTOR_OUTPUT_4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(IGNITION_OUTPUT_4_GPIO_Port, IGNITION_OUTPUT_4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LED_Pin INJECTOR_OUTPUT_3_Pin INJECTOR_OUTPUT_2_Pin INJECTOR_OUTPUT_1_Pin */
  GPIO_InitStruct.Pin = LED_Pin|INJECTOR_OUTPUT_3_Pin|INJECTOR_OUTPUT_2_Pin|INJECTOR_OUTPUT_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : LOW_SIDE_1_Pin LOW_SIDE_2_Pin FAN_1_Pin FAN_2_Pin
                           AUX_RELAY_Pin GAS_SOLENOID_RELAY_Pin FUEL_PUMP_RELAY_Pin MAIN_RELAY_Pin
                           IGNITION_OUTPUT_3_Pin IGNITION_OUTPUT_2_Pin IGNITION_OUTPUT_1_Pin */
  GPIO_InitStruct.Pin = LOW_SIDE_1_Pin|LOW_SIDE_2_Pin|FAN_1_Pin|FAN_2_Pin
                          |AUX_RELAY_Pin|GAS_SOLENOID_RELAY_Pin|FUEL_PUMP_RELAY_Pin|MAIN_RELAY_Pin
                          |IGNITION_OUTPUT_3_Pin|IGNITION_OUTPUT_2_Pin|IGNITION_OUTPUT_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : INJECTOR_OUTPUT_4_Pin */
  GPIO_InitStruct.Pin = INJECTOR_OUTPUT_4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(INJECTOR_OUTPUT_4_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : IGNITION_OUTPUT_4_Pin */
  GPIO_InitStruct.Pin = IGNITION_OUTPUT_4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(IGNITION_OUTPUT_4_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SENSOR_VR_1_Pin */
  GPIO_InitStruct.Pin = SENSOR_VR_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(SENSOR_VR_1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SENSOR_VR2_Pin */
  GPIO_InitStruct.Pin = SENSOR_VR2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(SENSOR_VR2_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */

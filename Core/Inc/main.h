/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "stm32h7xx_hal.h"

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
#define EN_D1_Pin GPIO_PIN_12
#define EN_D1_GPIO_Port GPIOB
#define EN_D2_Pin GPIO_PIN_13
#define EN_D2_GPIO_Port GPIOB
#define EN_D3_Pin GPIO_PIN_14
#define EN_D3_GPIO_Port GPIOB
#define LED3_Pin GPIO_PIN_15
#define LED3_GPIO_Port GPIOB
#define LED2_Pin GPIO_PIN_8
#define LED2_GPIO_Port GPIOD
#define LED1_Pin GPIO_PIN_9
#define LED1_GPIO_Port GPIOD
#define NO_AUDIO2_Pin GPIO_PIN_15
#define NO_AUDIO2_GPIO_Port GPIOD
#define ERROR2_Pin GPIO_PIN_8
#define ERROR2_GPIO_Port GPIOC
#define RESET2_Pin GPIO_PIN_8
#define RESET2_GPIO_Port GPIOA
#define RESET1_Pin GPIO_PIN_5
#define RESET1_GPIO_Port GPIOB
#define ERROR1_Pin GPIO_PIN_6
#define ERROR1_GPIO_Port GPIOB
#define NO_AUDIO1_Pin GPIO_PIN_9
#define NO_AUDIO1_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

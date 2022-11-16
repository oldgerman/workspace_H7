/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
#include "bsp_qspi_w25qxx.h"
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
#define QSPI_FLASH_SIZE_2N_OFFSET 0
#define LRGB_B_Pin GPIO_PIN_1
#define LRGB_B_GPIO_Port GPIOC
#define LRGB_G_Pin GPIO_PIN_3
#define LRGB_G_GPIO_Port GPIOC
#define KEY_A_Pin GPIO_PIN_0
#define KEY_A_GPIO_Port GPIOA
#define LRGB_R_Pin GPIO_PIN_1
#define LRGB_R_GPIO_Port GPIOA
#define KEY_B_Pin GPIO_PIN_2
#define KEY_B_GPIO_Port GPIOA
#define I2C1_INT_Pin GPIO_PIN_5
#define I2C1_INT_GPIO_Port GPIOB
#define I2C1_INT_EXTI_IRQn EXTI9_5_IRQn
/* USER CODE BEGIN Private defines */
#define QSPI_FLASH_SIZE_OFFSET 1
#ifdef QSPI_FLASH_SIZE_2N_OFFSET
#undef QSPI_FLASH_SIZE_2N_OFFSET
#define QSPI_FLASH_SIZE_2N_OFFSET (QSPI_FLASH_SIZE_2N - QSPI_FLASH_SIZE_OFFSET)
#else
#error "QSPI_FLASH_SIZE_2N_OFFSET undefined"
#endif
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

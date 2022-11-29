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

#include "stm32h7xx_ll_dma.h"
#include "stm32h7xx_ll_rcc.h"
#include "stm32h7xx_ll_crs.h"
#include "stm32h7xx_ll_bus.h"
#include "stm32h7xx_ll_system.h"
#include "stm32h7xx_ll_exti.h"
#include "stm32h7xx_ll_cortex.h"
#include "stm32h7xx_ll_utils.h"
#include "stm32h7xx_ll_pwr.h"
#include "stm32h7xx_ll_spi.h"
#include "stm32h7xx_ll_usart.h"
#include "stm32h7xx_ll_gpio.h"

#if defined(USE_FULL_ASSERT)
#include "stm32_assert.h"
#endif /* USE_FULL_ASSERT */

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
#define LRGB_B_Pin LL_GPIO_PIN_1
#define LRGB_B_GPIO_Port GPIOC
#define LRGB_G_Pin LL_GPIO_PIN_3
#define LRGB_G_GPIO_Port GPIOC
#define KEY_A_Pin LL_GPIO_PIN_0
#define KEY_A_GPIO_Port GPIOA
#define LRGB_R_Pin LL_GPIO_PIN_1
#define LRGB_R_GPIO_Port GPIOA
#define KEY_B_Pin LL_GPIO_PIN_2
#define KEY_B_GPIO_Port GPIOA
#define GPIO_HANDSHAKE_Pin LL_GPIO_PIN_6
#define GPIO_HANDSHAKE_GPIO_Port GPIOC
#define GPIO_HANDSHAKE_EXTI_IRQn EXTI9_5_IRQn
#define GPIO_DATA_READY_Pin LL_GPIO_PIN_7
#define GPIO_DATA_READY_GPIO_Port GPIOC
#define GPIO_DATA_READY_EXTI_IRQn EXTI9_5_IRQn
#define USR_SPI_CS_Pin LL_GPIO_PIN_8
#define USR_SPI_CS_GPIO_Port GPIOC
#define GPIO_RESET_Pin LL_GPIO_PIN_9
#define GPIO_RESET_GPIO_Port GPIOC
#define LPTIM_DMA_PWM_Pin LL_GPIO_PIN_0
#define LPTIM_DMA_PWM_GPIO_Port GPIOE
#ifndef NVIC_PRIORITYGROUP_0
#define NVIC_PRIORITYGROUP_0         ((uint32_t)0x00000007) /*!< 0 bit  for pre-emption priority,
                                                                 4 bits for subpriority */
#define NVIC_PRIORITYGROUP_1         ((uint32_t)0x00000006) /*!< 1 bit  for pre-emption priority,
                                                                 3 bits for subpriority */
#define NVIC_PRIORITYGROUP_2         ((uint32_t)0x00000005) /*!< 2 bits for pre-emption priority,
                                                                 2 bits for subpriority */
#define NVIC_PRIORITYGROUP_3         ((uint32_t)0x00000004) /*!< 3 bits for pre-emption priority,
                                                                 1 bit  for subpriority */
#define NVIC_PRIORITYGROUP_4         ((uint32_t)0x00000003) /*!< 4 bits for pre-emption priority,
                                                                 0 bit  for subpriority */
#endif
/* USER CODE BEGIN Private defines */
extern void SystemClock_Config(void);

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
void _Error_Handler(const char * file, uint32_t line);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LCD_T_VPW 6
#define LCD_T_VFP 10
#define SDRAM_BANK2_ADDR 0xD0000000
#define LCD_T_HBP 20
#define LCD_T_VBP 10
#define SDRAM_BANK1_ADDR 0xC0000000
#define LCD_T_HFP 10
#define QSPI_FLASH_SIZE_2N_OFFSET 0
#define LCD_T_HPW 10
#define LCD_T_VD 480
#define LCD_T_HD 480
#define LCD_MEM_ADDR 0x24000000
#define SPI2_MOSI_Pin GPIO_PIN_3
#define SPI2_MOSI_GPIO_Port GPIOI
#define LCD_RST_Pin GPIO_PIN_8
#define LCD_RST_GPIO_Port GPIOI
#define SPI2_SCK_Pin GPIO_PIN_3
#define SPI2_SCK_GPIO_Port GPIOD
#define SPI2_CS_Pin GPIO_PIN_3
#define SPI2_CS_GPIO_Port GPIOG

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

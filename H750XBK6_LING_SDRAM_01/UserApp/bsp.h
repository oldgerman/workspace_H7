/**
  ******************************************************************************
  * @file           : bsp.h
  ******************************************************************************
  * @Created on        : Jan 14, 2023
  * @Author         : OldGerman
  * @attention        :
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef BSP_BSP_H_
#define BSP_BSP_H_

#ifdef __cplusplus
extern "C" {
#endif
/* USER CODE BEGIN C SCOPE ---------------------------------------------------*/
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "stdbool.h"
#include "bsp_config.h"

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/
#define ENABLE_INT()    __set_PRIMASK(0)    /* 使能全局中断 */
#define DISABLE_INT()    __set_PRIMASK(1)    /* 禁止全局中断 */
#define bsp_GetRunTime HAL_GetTick

/* Exported functions prototypes ---------------------------------------------*/
void bsp_Init();
bool waitTime(uint32_t *timeOld, uint32_t wait);
void i2c_scaner(I2C_HandleTypeDef *hi2c, uint8_t i2cBusNum);
/* Private defines -----------------------------------------------------------*/

/* USER CODE END C SCOPE -----------------------------------------------------*/
#ifdef __cplusplus
}
/* USER CODE BEGIN C++ SCOPE -------------------------------------------------*/
/* Includes ------------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
/* Private defines -----------------------------------------------------------*/

/* USER CODE END C++ SCOPE ---------------------------------------------------*/
#endif
#endif    /* BSP_BSP_H_ */

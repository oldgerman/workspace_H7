/**
  ******************************************************************************
  * @file           : COMMON_INC_H.h
  * @brief          :
  ******************************************************************************
  * @Created on		: Jan 7, 2023
  * @Author 		: OldGerman
  * @attention		:
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __COMMON_INC_H
#define __COMMON_INC_H

#ifdef __cplusplus
extern "C" {
#endif
/* USER CODE BEGIN C SCOPE ---------------------------------------------------*/
/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "bsp.h"
#include "cmsis_os.h"
#include "freertos_inc.h"

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
void Main(void);

/* Private defines -----------------------------------------------------------*/
#define PROJECT_FW_VERSION 1.0

/* USER CODE END C SCOPE -----------------------------------------------------*/
#ifdef __cplusplus
}
/* USER CODE BEGIN C++ SCOPE -------------------------------------------------*/
/* Includes ------------------------------------------------------------------*/
#include "communication.hpp"

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/

/* Private defines -----------------------------------------------------------*/

/* USER CODE END C++ SCOPE ---------------------------------------------------*/
#endif
#endif	/* __COMMON_INC_H */

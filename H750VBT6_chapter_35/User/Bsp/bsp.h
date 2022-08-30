 /*
 * BSP.h
 *
 *  Created on: May 16, 2022
 *      Author: OldGerman
 */

/*
 * BSP.h -- Board Support
 *
 * This exposes functions that are expected to be implemented to add support for different hardware
 */

#ifndef BSP_BSP_H_
#define BSP_BSP_H_
#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "stdbool.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bsp_config.h"
#include "bsp_redirect_callback.h"
#include "bsp_button.h"
#include "bsp_uart_fifo.h"
#include "bsp_tim6.h"
#include "tim.h"
#include "bsp_timx_pwm.h"
#include "bsp_timer.h"
//#include "bsp_qspi_w25qxx.h"
//#include "demo_spi_flash.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
/* 开关全局中断的宏 */
#define ENABLE_INT()	__set_PRIMASK(0)	/* 使能全局中断 */
#define DISABLE_INT()	__set_PRIMASK(1)	/* 禁止全局中断 */
#define bsp_GetRunTime HAL_GetTick

/* 用于BSP调试阶段排错 */
#ifndef BSP_Printf
	#if 0  //< Change 0 to 1 to open debug macro and check program debug information
		#define BSP_Printf usb_printf
	#elif 1
		#define BSP_Printf printf
	#else
		#define BSP_Printf(...)
	#endif
#endif

//#define BSP_Printf(...)

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
/* USER CODE BEGIN EFP */
void bsp_Init();
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */


#ifdef __cplusplus

bool waitTime(uint32_t *timeOld, uint32_t wait);
}
#endif
#endif /* BSP_BSP_H_ */

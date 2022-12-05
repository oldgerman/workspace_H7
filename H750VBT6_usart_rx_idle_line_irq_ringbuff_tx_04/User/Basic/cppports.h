 /*
 * cppports.h
 *
 *  Created on: May 16, 2022
 *      Author: OldGerman
 */

#ifndef INC_CPPPORTS_H_
#define INC_CPPPORTS_H_

#ifdef __cplusplus
extern "C" {
#endif

/* 私有包含 Private includes -------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
//#include "bsp.h"
//#include "ButtonEvent.h"
//#include "FreeRTOS.h"
/* USER CODE END Includes */

/* 导出类型 Exported types ---------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* 导出常量 Exported constants -----------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* 导出的宏 Exported macro ---------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* 导出函数原型 Exported functions prototypes --------------------------------*/

/* USER CODE BEGIN EFP */
void uart_thread();
void led_thread();

void setup();
void loop();
void btA_CLICKED_func();
void btB_CLICKED_func();
void btA_LONG_PRESSED_func();
void btB_LONG_PRESSED_func();

void re_DMA1_Stream0_IRQHandler(void);
void re_DMA1_Stream1_IRQHandler(void);
void re_USART1_IRQHandler(void);
/* USER CODE END EFP */

/* 私有定义 Private defines --------------------------------------------------*/
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* INC_CPPPORTS_H_ */

/*
 * irq.h
 *
 *  Created on: 2022年12月26日
 *      @Author: OldGerman (过气德国佬)
 */
#ifndef IRQ_H_
#define IRQ_H_

#ifdef __cplusplus
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "bsp.h"

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi);
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi);
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi);

/* Private defines -----------------------------------------------------------*/

#ifdef __cplusplus

}		/* extern "C" */
#endif	/* __cplusplus */
#endif /* BSP_H_ */

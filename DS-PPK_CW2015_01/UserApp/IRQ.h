/*
 * Irqs.h
 *
 *  Created on: 30 May 2020
 *      Author: Ralim
 *      Modify:OldGerman
 */

#ifndef BSP_MINIWARE_IRQ_H_
#define BSP_MINIWARE_IRQ_H_

#include "I2C_Wrapper.h"
#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

//以下对HAL的__weak回调函数重写
/*非阻塞模式下的回调函数：
 * 也就是说只有调用FRToI2C的非阻塞API才会调用这些回调函数，使用阻塞API是不会有这些回调函数的
HAL_I2C_ErrorCallback();
HAL_I2C_MasterTxCpltCallback()
HAL_I2C_MasterRxCpltCallback()；
HAL_I2C_MemTxCpltCallback()；
HAL_I2C_MemRxCpltCallback()；
 */

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_AbortCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c);

#ifdef __cplusplus
}
#endif
#endif /* BSP_MINIWARE_IRQ_H_ */

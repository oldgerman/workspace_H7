/*
 * IRQ.c
 *
 *  Created on: 30 May 2020
 *      Author: Ralim
 */

#include "IRQ.h"
#include "I2C_Wrapper.h"

#include <stdio.h>	//提供 __unused 宏

/*
 * 非阻塞模式（中断和DMA）中使用的I2C IRQHandler和回调（对__weak重写）
 * I2C IRQHandler and Callbacks used in non blocking modes (Interrupt and DMA)
 */
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c) { FRToSI2C::CpltCallback(hi2c); }	//主接收完成
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c) { FRToSI2C::CpltCallback(hi2c); }	//主发送完成
void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c) { FRToSI2C::CpltCallback(hi2c); }
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c) { FRToSI2C::CpltCallback(hi2c); }
void HAL_I2C_AbortCpltCallback(I2C_HandleTypeDef *hi2c) { FRToSI2C::CpltCallback(hi2c); }
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c) { FRToSI2C::CpltCallback(hi2c); }


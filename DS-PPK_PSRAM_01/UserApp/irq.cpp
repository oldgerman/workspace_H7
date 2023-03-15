/**
  ******************************************************************************
  * @file        irq.cpp
  * @author      OldGerman
  * @created on  Mar 12, 2023
  * @brief       
  ******************************************************************************
  * @attention
  *
  * Copyright (C) 2022 OldGerman.
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see https://www.gnu.org/licenses/.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "irq.h"
#include "psram_benchmark.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Function implementations --------------------------------------------------*/

/**
 * @brief  TxRx Transfer completed callback.
 * @param  hspi: SPI handle
 * @note   This example shows a simple way to report end of DMA TxRx transfer, and
 *         you can add your own implementation.
 * @retval None
 */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
	SPI2_Base.TxRxCpltCallback(hspi);
}

/**
 * @brief  SPI error callbacks.
 * @param  hspi: SPI handle
 * @note   This example shows a simple way to report transfer error, and you can
 *         add your own implementation.
 * @retval None
 */
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
//	SPI2_Base.ErrorCallback(hspi);
}

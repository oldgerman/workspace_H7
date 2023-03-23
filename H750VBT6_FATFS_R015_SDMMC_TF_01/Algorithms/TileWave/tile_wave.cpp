/**
  ******************************************************************************
  * @file        tile_wave.cpp
  * @author      OldGerman
  * @created on  Mar 20, 2023
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
#include "tile_wave.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
std::function<void*(size_t size)> 	TileWave::malloc;
std::function<void(void* ptr)>		TileWave::free;

__attribute__((section(".RAM_D1_Array"))) ALIGN_32BYTES(uint8_t TxBuffer [64 *1024]);
__attribute__((section(".RAM_D1_Array"))) ALIGN_32BYTES(uint8_t RxBuffer [10 *1024]);

void* TileWave::ucpTxBuffer = TxBuffer;
void* TileWave::ucpRxBuffer = RxBuffer;

std::function<uint32_t (uint32_t addr, uint32_t size, uint8_t* pData)> 		TileWave::write;
std::function<uint32_t (uint32_t addr, uint32_t size, uint8_t* pData)>		TileWave::read;
uint32_t TileWave::ulPeriod = 0;	//周期计数器
uint32_t TileWave::ulTxBufferOffsetOld = 0;
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Function implementations --------------------------------------------------*/


/**
  ******************************************************************************
  * @file        dynamic_ram.cpp
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
#include "dynamic_ram.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/*       Memory Pool        Size [KB  *1024/8]                  Section       */
uint64_t MEMPOOL_D2_SRAM1        [128 *1024/8]  __attribute__((section(".RAM_D2_Array")));
//uint64_t MEMPOOL_D2_SRAM2        [128 *1024/8]  __attribute__((section(".RAM_D2_Array")));
/* Exported variables --------------------------------------------------------*/
/* Create dynamic memory objects */
osRtxMemory DRAM_SRAM1   (MEMPOOL_D2_SRAM1,   sizeof(MEMPOOL_D2_SRAM1));
//osRtxMemory DRAM_SRAM2   (MEMPOOL_D2_SRAM2,   sizeof(MEMPOOL_D2_SRAM2));

/* Private function prototypes -----------------------------------------------*/
/* Function implementations --------------------------------------------------*/
/**
  * @brief  初始化动态内存对象的内存池
  * @param  None
  * @retval 0 - success, ≥1 - failure.
  */
uint32_t DRAM_Init()
{
	uint32_t ret = 0;

	ret += DRAM_SRAM1.init();
//	ret += DRAM_SRAM2.init();

	return ret;
}

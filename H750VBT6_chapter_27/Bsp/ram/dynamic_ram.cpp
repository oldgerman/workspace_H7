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
/* Exported variables --------------------------------------------------------*/
/* RAM 空间分配*/
/* DTCM, 64KB */
mem_head_t *DTCMUsed;
uint64_t AppMallocDTCM[64*1024/8]__attribute__((section(".RAM_DTCM_Array")));

/* AXI SRAM, D1域, 512KB */
mem_head_t *AXISRAMUsed;
uint64_t AppMallocAXISRAM[512*1024/8]__attribute__((section(".RAM_D1_Array")));

/* SRAM1, D2域, 128KB */
mem_head_t *SRAM1Used;
uint64_t AppMallocSRAM1[128*1024/8]__attribute__((section(".RAM_D2_Array")));

/* SRAM2, D2域, 128KB */
mem_head_t *SRAM2Used;
uint64_t AppMallocSRAM2[128*1024/8]__attribute__((section(".RAM_D2_Array")));

/* SRAM3, D2域, 32KB */
mem_head_t *SRAM3Used;
uint64_t AppMallocSRAM3[32*1024/8]__attribute__((section(".RAM_D2_Array")));

/* SRAM3, D3域, 64KB */
mem_head_t *SRAM4Used;
uint64_t AppMallocSRAM4[64*1024/8]__attribute__((section(".RAM_D3_Array")));

/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Function implementations --------------------------------------------------*/
/**
  * @brief  初始化内存池
  * @param  None
  * @retval 0 - success, ≥1 - failure.
  */
uint32_t DRAM_Init()
{
	uint32_t ret = 0;

	ret += osRtxMemoryInit(AppMallocDTCM,    sizeof(AppMallocDTCM));
	ret += osRtxMemoryInit(AppMallocAXISRAM, sizeof(AppMallocAXISRAM));
	ret += osRtxMemoryInit(AppMallocSRAM1,   sizeof(AppMallocSRAM1));
	ret += osRtxMemoryInit(AppMallocSRAM2,   sizeof(AppMallocSRAM2));
	ret += osRtxMemoryInit(AppMallocSRAM3,   sizeof(AppMallocSRAM3));
	ret += osRtxMemoryInit(AppMallocSRAM4,   sizeof(AppMallocSRAM4));

	return ret;
}

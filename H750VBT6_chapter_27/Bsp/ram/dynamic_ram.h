/**
  ******************************************************************************
  * @file        dynamic_ram.h
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef RAM_DYNAMIC_RAM_H_
#define RAM_DYNAMIC_RAM_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "rtx_memory.h"

/* Exported types ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern mem_head_t *DTCMUsed;
extern uint64_t AppMallocDTCM[64*1024/8];

extern mem_head_t *AXISRAMUsed;
extern uint64_t AppMallocAXISRAM[512*1024/8];

extern mem_head_t *SRAM1Used;
extern uint64_t AppMallocSRAM1[128*1024/8];

extern mem_head_t *SRAM2Used;
extern uint64_t AppMallocSRAM2[128*1024/8];

extern mem_head_t *SRAM3Used;
extern uint64_t AppMallocSRAM3[32*1024/8];

extern mem_head_t *SRAM4Used;
extern uint64_t AppMallocSRAM4[64*1024/8];

/* Exported functions --------------------------------------------------------*/
uint32_t DRAM_Init();

#ifdef __cplusplus
}
#endif

#endif /* RAM_DYNAMIC_RAM_H_ */

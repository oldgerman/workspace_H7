/**
  ******************************************************************************
  * @file        sdram.h
  * @author      OldGerman
  * @created on  Dec 12, 2023
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
#ifndef SDRAM_SDRAM_H_
#define SDRAM_SDRAM_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"
//#include "usart.h"

/* Exported types ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/*********************  SDRAM   *********************/
#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)

/* 设置 SDRAM 自刷新频率
 * 安富莱V7_BSP教程_P1038
 * Quadmin SDRAM 使用 W9825G6KH row 地址是 13bit，2^13 = 8192，对应手册：Refresh Time (8K Refresh Cycles) 64ms
 * 和安富莱V7核心板使用的 IS4232800G 4096/ms 不同，因此自刷新率公式要除以8192，这点要注意！
 * 计算公式:
      SDRAM_REFRESH_COUNT = SDRAM_refresh_period / Number_of_rows * SDRAM时钟频率 – 20
      这个数值稍差点，对使用 SDRAM 时基本没影响
 * 计算 Quadmin 的 W9825G6KH
    = 64000(64 ms) / 8192 *100MHz - 20
    = 761.25 取值 762
*/
#define SDRAM_REFRESH_COUNT                      ((uint32_t)762)    // 反客H750XBH6板子代码配置是 918，SDRAM时钟120MHz，
                                                                     // 属于超频行为，918/762=1.205倍，对应 120MHz 是 100MHz 的 1.2倍

//#define SDRAM_TIMEOUT                            ((uint32_t)0xFFFF)
#define SDRAM_TIMEOUT                            ((uint32_t)0x1000) // 反客H750XBH6板子代码配置是 0x1000

#if defined ( __ICCARM__ ) // !< IAR Compiler
#pragma location=0xC0000000
static __no_init uint32_t sdram_data1[100];
#pragma location=0xD0000000
static __no_init uint32_t sdram_data2[100];
#elif defined ( __CC_ARM ) //!< Keil Compiler
uint32_t sdram_data1[100] __attribute__((at(0xC0000000)));
uint32_t sdram_data2[100] __attribute__((at(0xD0000000)));

//#elif defined ( __GNUC__ ) // !< GNU Compiler
//uint32_t sdram_data1[100]__attribute__((section(".sdram1")));//Must add sdram1 script to STM32H743ZITx_FLASH.ld
//uint32_t sdram_data2[100]__attribute__((section(".sdram2")));//Must add sdram2 script to STM32H743ZITx_FLASH.ld
#endif



// SDRAM容量
//#define SDRAM_Size 16*1024*1024  //16M字节 W9812G6KH
#define SDRAM_Size 32*1024*1024  //32M字节 W9825G6KH

/* Exported macro ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
void SDRAM_Initialization_sequence1(SDRAM_HandleTypeDef  *sdramHandle, uint32_t RefreshCount);
void SDRAM_Initialization_sequence2(SDRAM_HandleTypeDef  *sdramHandle, uint32_t RefreshCount);
uint8_t SDRAM_Test(uint32_t SDRAM_BANK_ADDR);

#ifdef __cplusplus
}
#endif

#endif /* SDRAM_SDRAM_H_ */

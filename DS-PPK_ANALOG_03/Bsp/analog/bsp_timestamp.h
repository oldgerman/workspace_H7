/**
  ******************************************************************************
  * @file        bsp_timestamp.h
  * @author      OldGerman
  * @created on  Feb 10, 2023
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
#ifndef BSP_TIMESTAMP_H_
#define BSP_TIMESTAMP_H_

#include "bsp_analog.h"
#include "bsp_auto_sw.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
/* Exported constants --------------------------------------------------------*/
const uint32_t timestamp_auto_sw_buffer_size = adc1_adc3_buffer_size / 2;
//const uint32_t timestamp_auto_sw_buffer_size = 10;

/* Exported types ------------------------------------------------------------*/
/* 标记时间戳当前可访问的时间戳缓冲区范围 */
typedef enum
{
	TS_BUF_1ST = 0,	//前半缓冲区
	TS_BUF_2ND		//后半缓冲区
}timestamp_buffer_select_t;

/* 时间戳双缓冲区结构体 */
typedef struct
{
	/* 当前使用双缓冲区前半还是后半记录时间戳 */
	uint32_t bs;	//buffer select

	/* 双缓冲区：在自动换挡的GPIO外部中断回调函数中记录 */
	struct{
		auto_sw_range_t 	range[timestamp_auto_sw_buffer_size];	//量程
		uint32_t 			time[timestamp_auto_sw_buffer_size];	//时间戳
		uint32_t 			cs;										//缓冲区游标
	}auto_sw[2];

	/* 双缓冲区：记录时间戳 */
	uint32_t dma_adc1[2];		//在ADC1 DMA半传输和传输完成中断回调函数中记录
	uint32_t dma_logic[2];		//在LOGIC GPIO DMA半传输和传输完成中断回调函数中记录

	/* 单个缓冲：记录时间戳 */
	uint32_t trig_adc1;			//在ADC1 模拟看门狗中断回调函数中记录
	uint32_t trig_adc3;			//在ADC3 模拟看门狗中断回调函数中记录
	uint32_t trig_logic;		//在LOGIC GPIO上升或下降沿的外部中断回调函数中记录
}timestamp_t;

/* Exported define -----------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern timestamp_t timestamp;

/* Exported functions --------------------------------------------------------*/
void bsp_timestamp_init();
uint32_t bsp_timestamp_get();

#ifdef __cplusplus
}
#endif

#endif /* BSP_TIMESTAMP_H_ */

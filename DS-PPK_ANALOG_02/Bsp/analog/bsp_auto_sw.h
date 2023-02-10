/**
  ******************************************************************************
  * @file        bsp_auto_sw.h
  * @author      OldGerman
  * @created on  Jan 31, 2023
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
#ifndef BSP_AUTO_SW_H_
#define BSP_AUTO_SW_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* 自动换挡量程结构体 */
typedef union{
		/* 位域成员反映引脚电平，1：高，0：低*/
		struct {
			uint8_t sw1		:1;
			uint8_t sw2		:1;
			uint8_t sw3		:1;
			uint8_t sw4		:1;
			uint8_t unused 	:4;
		};	//匿名结构体成员
		uint8_t swx;
}auto_sw_range_t;

/* Exported define -----------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
void bsp_auto_sw_init();

#ifdef __cplusplus
}
#endif

#endif /* BSP_AUTO_SW_H_ */

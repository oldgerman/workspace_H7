/**
  ******************************************************************************
  * @file        frame_processor.h
  * @author      OldGerman
  * @created on  Feb 7, 2023
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
#ifndef FRAME_PROCESSOR_H_
#define FRAME_PROCESSOR_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
/* Exported types ------------------------------------------------------------*/
/* Slave 向 Host 发送数据帧的格式 */
typedef union{
		struct {
			uint32_t adc1		:12;
			uint32_t swx		:4;
			uint32_t adc3		:8;
			uint32_t logic		:8;
		}format_A;

		struct {
			uint32_t adc		:14;	/*!< bit[0:13] 	ADC数据，测量电流通道ADC的值，0~16384 */
			uint32_t range		:4;		/*!< bit[14:16] 量程，五档量程分别为 000，001，010，011，100 */
			uint32_t counter	:6;		/*!< bit[18:23] 计数器，发送数据帧的序列号，该序列号在每次发送时需要递增1，当计数值大于63，从0开始重新计数 */
			uint32_t logic		:8;		/*!< bit[24:31] 逻辑端口电平 */
		}format_nRF;				/*!< nRF Connect Power Profiler 发送给上位机协议帧格式 */

//		struct {
//			uint32_t adc		:14;	/*!< bit[0:13] 	ADC数据，测量电流通道ADC的值，0~16384 */
//			uint32_t range		:3;		/*!< bit[14:16] 量程，五档量程分别为 000，001，010，011，100 */
//			uint32_t unused 	:1;		/*!< bit[17] 	预留 */
//			uint32_t counter	:6;		/*!< bit[18:23] 计数器，发送数据帧的序列号，该序列号在每次发送时需要递增1，当计数值大于63，从0开始重新计数 */
//			uint32_t logic		:8;		/*!< bit[24:31] 逻辑端口电平 */
//		}format_nRF;					/*!< nRF Connect Power Profiler 发送给上位机协议帧格式 */

		uint32_t ctrl_u32;
		uint8_t ctrl_u8[4];
}frame_format_t;

/* Exported define -----------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern bool frame_processor_debug_print;
extern bool frame_processor_data_print_ppk2;
/* Exported functions --------------------------------------------------------*/
void frame_processor_init();

#ifdef __cplusplus
}
#endif

#endif /* FRAME_PROCESSOR_H_ */

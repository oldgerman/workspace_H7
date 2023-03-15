/**
  ******************************************************************************
  * @file        bsp_timestamp.cpp
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
  * @application note
  *
  * 每秒100次传输，进行 100次 数据帧合成，每次合成帧需要处理 1000 点 ADC 数据，
  * 仪放输出到自动换挡比较器的输入端之间的RC滤波器带宽: 72KHz，
  * 换挡速度仿真在 8us，极限 125KHz，但实际应该百分百小于ADC的采样速度，
  * 因此换挡中断的时间戳双缓冲区点数给 sample_buffer_size 足够，注意大小为 ADC 双缓冲区的大小，不是单缓冲区的大小
  * 这样H7 在合成帧时，会在前一个 和 后一个 半缓冲区来回切换，保证处理一半缓冲区的数据时，另一半还会实时更新，
  * 这个缓冲区切换机制需要在ADC DMA 半传输和传输完成中断中更改时间戳缓冲区的指针指向前半还是后半个缓冲区
  * 在 HAL_ADC_ConvCpltCallback 或 HAL_ADC_ConvHalfCpltCallback 中更改 auto_sw缓冲区 使用前半还是后半
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "bsp_timestamp.h"
#include "tim.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
ALIGN_32BYTES(__attribute__((section (".RAM_D2_Array"))) timestamp_t timestamp);

/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Function implementations --------------------------------------------------*/
/**
  * @brief  初始化时间戳缓冲区和定时器
  * @param  None
  * @retval None
  */
void bsp_timestamp_init()
{
	memset(&timestamp, 0x0, sizeof(timestamp_t));

	HAL_TIM_Base_Start(&htim2);
}

/**
  * @brief  获取时间戳数据
  * @param  None
  * @retval uint32_t  tim2 的 32bit CNT 寄存器值，CNT寄存器每 10us 自增 1，约 11.93 小时后溢出
  */
uint32_t bsp_timestamp_get()
{
	return htim2.Instance->CNT;
}


/**
 ******************************************************************************
 * @file        frame_processor.cpp
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

/* Includes ------------------------------------------------------------------*/
#include "frame_processor.h"
#include "common_inc.h"
#include "interface_usb.hpp"
#include "tile_wave.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
const uint32_t frameProcessorTaskStackSize = 512 * 4;

/* Private constants ---------------------------------------------------------*/
/* Definitions for defaultTask */
const osThreadAttr_t frameProcessorTask_attributes = {
		.name = "frameProcessorTask",
		.stack_size = frameProcessorTaskStackSize,
		.priority = (osPriority_t) osPriorityRealtime,
};

/* Exported variables --------------------------------------------------------*/
ALIGN_32BYTES(__attribute__((section (".RAM_DTCM_Array"))) frame_format_t frame[8000 / 2]);

osThreadId_t frameProcessorTaskHandle;

bool frame_processor_debug_print = false;
bool frame_processor_data_print_ppk2 = false;

extern TileWave xTileWave;
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void frameProcessorTask(void* argument);

/* Function implementations --------------------------------------------------*/
/**
 * @brief  帧处理器任务函数
 * @param  argument Pointer to a void
 * @retval None
 */
static void frameProcessorTask(void* argument)
{

	TickType_t xLastWakeTime;
	uint8_t taskFreq = 1;  /* 每秒调度频率，单位Hz */
	const TickType_t xTaskPeriod = 1000 / taskFreq; /* 调度周期，单位ms */
	xLastWakeTime = xTaskGetTickCount();            /* 获取当前的系统时间 */

	for (;;)
	{

		xTileWave.writeTileBuffer(frame[0].ctrl_u8);

		//上位机格式打印数据
		if(frame_processor_data_print_ppk2)
		{

		}

		// Debug阶段打印数据
		if(frame_processor_debug_print)
		{

		}

		vTaskDelayUntil(&xLastWakeTime, xTaskPeriod);
	}
};

/**
 * @brief  初始化帧处理器
 * @param  None
 * @retval None
 */
void frame_processor_init()
{
	frameProcessorTaskHandle = osThreadNew(frameProcessorTask, NULL, &frameProcessorTask_attributes);
}

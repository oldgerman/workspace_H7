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
#include "demo_sd_fatfs.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
const uint32_t frameProcessorTaskStackSize = 512 * 4;

/* Private constants ---------------------------------------------------------*/
const osThreadAttr_t frameProcessorTask_attributes = {
		.name = "frameProcessorTask",
		.stack_size = frameProcessorTaskStackSize,
		.priority = (osPriority_t) osPriorityRealtime,
};

/* Exported variables --------------------------------------------------------*/
ALIGN_32BYTES(__attribute__((section (".RAM_DTCM_Array"))) frame_format_t frame[8000]);

osThreadId_t frameProcessorTaskHandle;

bool frame_writeTileBuffer = false;
bool frame_initExistingWaveFile= false;
uint32_t sliceButNotWrite = 0;

uint16_t frame_freq = 1; /* 每秒调度频率，单位Hz */

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
	TickType_t xTaskPeriod;
	xLastWakeTime = xTaskGetTickCount();	/* 获取当前的系统时间 */
	/* 帧缓冲区全部归'.'*/
	memset(frame, '.', sizeof(frame));

	/* 切片前复位一些变量 */
	xTileWave.resetVariablesBeforeSlice();

	osStatus_t osStatus;
	TileWave::Event_t msg;
	uint32_t ulPeriodCount = 0;
	uint32_t ulQueueCount = 0;		// 当前消息队列消息数
	uint32_t ulQueueCountHistoryMax = 0;	// 历史消息队列最大消息数
	uint32_t ulMemoryMin = 0;
	uint32_t ulHistoryMemoryMin = 0;
	for (;;)
	{
		if(frame_initExistingWaveFile)
		{
			frame_initExistingWaveFile = 0;
			initExistingWaveFile();
		}

		if(frame_writeTileBuffer == 0)
		{
			/* 切片前复位一些变量 */
			xTileWave.resetVariablesBeforeSlice();
			ulMemoryMin = DRAM_SRAM1.getMemFree();
			ulHistoryMemoryMin = ulMemoryMin;
			ulPeriodCount = 0;
			ulQueueCountHistoryMax = 0;
			/* 通知 fatfsSDTask 停止 */
			msg.type = TileWave::EVENT_STOP;
			osMessageQueuePut(xTileWave.xMsgQueue, &msg, 0U, 0U);
		}
		else
		{
			/* 4096次后停止 */
			if(ulPeriodCount == xTileWave.ulLayerTilesNumMax - 1) {
				frame_writeTileBuffer = 0;
			}

			/* 采样缓冲区第一个元素写入当前切片周期数 */
			sprintf((char*)&(frame[0].ctrl_u8[0]), "%4ld", xTileWave.ulPeriod);

			msg.type = TileWave::EVENT_WRITE_RING_BUFFER;
			msg.xWriteRingBufferParam = xTileWave.sliceTileBuffer(frame[0].ctrl_u8); // 切片瓦片缓冲区暂存到多缓冲区
			// 向消息队列写消息
			osStatus = osMessageQueuePut(
					xTileWave.xMsgQueue,
					&msg, 	// 指向消息的指针，会使用 memcpy 拷贝消息地址上的数据，不是直接传递地址
					0U, 	// 消息优先级 0
					0U);	// 写阻塞时间
			ulQueueCount = osMessageQueueGetCount(xTileWave.xMsgQueue);
			if(ulQueueCountHistoryMax < ulQueueCount) {
				ulQueueCountHistoryMax = ulQueueCount;
			}
			ulMemoryMin = DRAM_SRAM1.getMemFree();
			if(ulHistoryMemoryMin > ulMemoryMin) {
				ulHistoryMemoryMin = ulMemoryMin;
			}
			printf("|  frameTask  | osStatus = %1d | queue count = %1ld | queue count hisrotry max = %1ld | history free min = %6ld | \r\n" ,
					osStatus, ulQueueCount, ulQueueCountHistoryMax, ulHistoryMemoryMin);

			ulPeriodCount++;
		}

		xTaskPeriod = 1000 / frame_freq;	/* 调度周期，单位ms */

		if(xTileWave.ulSliceButNotWrite == 0) // 若切片但不写入是假，那么就需要放写入消息
//		osMessagePut(queue_id, info, millisec);

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

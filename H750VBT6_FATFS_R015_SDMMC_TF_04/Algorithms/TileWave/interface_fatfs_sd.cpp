/**
  ******************************************************************************
  * @file        interface_fatfs_sd.cpp
  * @author      OldGerman
  * @created on  Mar 26, 2023
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
#include "interface_fatfs_sd.h"
#include "common_inc.h"
#include "interface_usb.hpp"
#include "tile_wave.h"
#include "demo_sd_fatfs.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
const uint32_t fatfsSDTaskStackSize = 512 * 4;

/* Private constants ---------------------------------------------------------*/
const osThreadAttr_t fatfsSDTask_attributes = {
		.name = "fatfsSDTask",
		.stack_size = fatfsSDTaskStackSize,
		.priority = (osPriority_t) osPriorityNormal,
};
/* Exported variables --------------------------------------------------------*/
osThreadId_t fatfsSDTaskHandle;
extern TileWave xTileWave;
//__attribute__((section(".RAM_D1_Array"))) ALIGN_32BYTES(uint8_t TxBuffer [64 *1024]);
bool fatfsSD_printUnitData = 1;	//是否打印单元数据
uint32_t ulOffset_DispBeginToReadBufferBegin = 0;
uint32_t ulDispWidthPx = 400; // 显示区宽度 400像素
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Function implementations --------------------------------------------------*/
/**
 * @brief  FATFS SD 卡任务函数
 * @param  argument Pointer to a void
 * @retval None
 */
static void fatfsSDTask(void* argument)
{
	osStatus_t osStatus;
	TileWave::Event_t msg;

	/* 用于计算本函数被调用的实时频率的单次和平均值 */
	double fRealWrittenFreq = 0;
	uint32_t ulTickCountOld =  xTaskGetTickCount();
	uint32_t ulTickCount;
	double	fRealWrittenFreqSum = 0; 		// 写频率的总和
	double 	fRealWrittenFreqAvg = 0;		// 写频率的平均值
	double 	fRealWrittenFreqNum = 0;		// 写频率的个数

	for(;;)
	{
		osStatus = osMessageQueueGet(xTileWave.xMsgQueue, &msg, 0U, osWaitForever);   // wait for message
		if (osStatus == osOK) {
			// process data
			uint32_t ret = 0; // FATFS 函数的返回状态

			if(msg.type == TileWave::EVENT_WRITE_LAYER_BUFFER ||
					msg.type == TileWave::EVENT_LAST_WRITE_LAYER_BUFFER) {
				/* 计算实时频率的单次和平均值 */
				ulTickCount = xTaskGetTickCount();	/* 获取当前的系统时间 */
				uint32_t ulTickOffest = ulTickCount - ulTickCountOld;
				ulTickCountOld = ulTickCount;
				fRealWrittenFreq = (double)1000 / ((double)ulTickOffest);

				// 第一次 fRealWrittenFreq 肯定是 inf，需要舍弃
				// fRealWrittenFreq 是一个正常的浮点数才会计算
				if(isnormal(fRealWrittenFreq))	{
					++fRealWrittenFreqNum;
					fRealWrittenFreqSum += fRealWrittenFreq;
					fRealWrittenFreqAvg = fRealWrittenFreqSum / fRealWrittenFreqNum;
				}
				printf("fatfsSDTaskFreq: %3.3f, %3.3f\r\n", fRealWrittenFreq, fRealWrittenFreqAvg);

				/* 将缓冲区的数据写入SD卡 */
				if(msg.xWriteLayerBufferParam.pucData != NULL) {

					/* STM32H750 的 SDMMC1 仅支持 RAM_D1，而 SDMMC2 支持 RAM_D1 和 RAM_D2
					 * 本工程使用 SDMMC1，以下写操作访问动态内存中的缓冲区请务必在 RAM_D1 */
					ret = xTileWave.write(
							msg.xWriteLayerBufferParam.ulAddr,
							msg.xWriteLayerBufferParam.ulSize,
							msg.xWriteLayerBufferParam.pucData);

					/* 不论 FATFS 是否写入成功，都要释放缓冲区的内存 */
					xTileWave.aligned_free(msg.xWriteLayerBufferParam.pucData);
				}
				/* 打印本次详情 */
				printf("| fatfsSDTask | osStatus = %d | ulPeriod = %4ld | ret = %2ld | addr = %10ld | size = %6ld | mark = %2ld | \r\n",
						osStatus,
						msg.xWriteLayerBufferParam.ulPeriod,
						ret,
						msg.xWriteLayerBufferParam.ulAddr,
						msg.xWriteLayerBufferParam.ulSize,
						msg.xWriteLayerBufferParam.ulMark);

				if (msg.type == TileWave::EVENT_LAST_WRITE_LAYER_BUFFER) {
								fRealWrittenFreqSum = 0;
								fRealWrittenFreqAvg = 0;
								fRealWrittenFreqNum = 0;
				}
			} else if (msg.type == TileWave::EVENT_READ_LAYER_BUFFER ) {
				ret = xTileWave.read(
						msg.xReadLayerBufferParam.ulAddr,
						msg.xReadLayerBufferParam.ulSize,
						msg.xReadLayerBufferParam.pucData);
				if(fatfsSD_printUnitData) {
					float *pfVal = (float*)(msg.xReadLayerBufferParam.pucData);
					for(uint32_t i = 0; i < msg.xReadLayerBufferParam.ulSize / 4 / 8; i++)
					{
						printf("%.2f\n%.2f\n%.2f\n%.2f\n%.2f\n%.2f\n%.2f\n%.2f\n",
									*(pfVal + i * 8 + 0),
									*(pfVal + i * 8 + 1),
									*(pfVal + i * 8 + 2),
									*(pfVal + i * 8 + 3),
									*(pfVal + i * 8 + 4),
									*(pfVal + i * 8 + 5),
									*(pfVal + i * 8 + 6),
									*(pfVal + i * 8 + 7));
					}
					printf("\r\n[read param] ulAddr = %10ld, ulSize = %6ld, ulUnitOffsetFile = %6ld\r\n",
							msg.xReadLayerBufferParam.ulAddr,
							msg.xReadLayerBufferParam.ulSize,
							msg.xReadLayerBufferParam.ulUnitOffsetFile);
				}
			}  else if (msg.type == TileWave::EVENT_READ_LAYER_BUFFER_LIST ) {
				TileWave::ReadLayerBufferParam_t xParam;
				for(uint32_t i = 0; i < msg.xReadLayerBufferParamList.size; i++) {
					xParam = *(msg.xReadLayerBufferParamList.px + i);

					ret = xTileWave.read(
							xParam.ulAddr,
							xParam.ulSize,
							xParam.pucData);
					if(fatfsSD_printUnitData) {
						float *pfVal = (float*)(xParam.pucData);
						for(uint32_t i = 0; i < xParam.ulSize / 4 / 8; i++)
						{
							printf("%.2f\n%.2f\n%.2f\n%.2f\n%.2f\n%.2f\n%.2f\n%.2f\n",
										*(pfVal + i * 8 + 0),
										*(pfVal + i * 8 + 1),
										*(pfVal + i * 8 + 2),
										*(pfVal + i * 8 + 3),
										*(pfVal + i * 8 + 4),
										*(pfVal + i * 8 + 5),
										*(pfVal + i * 8 + 6),
										*(pfVal + i * 8 + 7));
						}
						printf("\r\n[read param] ulAddr = %10ld, ulSize = %6ld, ulUnitOffsetFile = %6ld\r\n",
								xParam.ulAddr,
								xParam.ulSize,
								xParam.ulUnitOffsetFile);
					}
				}
			} else if (msg.type == TileWave::EVENT_ZOOM_LAYER_BUFFER_LIST ) {
				TileWave::ReadLayerBufferParam_t xParam;
				bool firstIn = 1;
				uint32_t count_printed = 0;
				uint32_t count_willPrint = ulDispWidthPx;
				for(uint32_t i = 0; i < msg.xReadLayerBufferParamList.size; i++) {
					xParam = *(msg.xReadLayerBufferParamList.px + i);

					ret = xTileWave.read(
							xParam.ulAddr,
							xParam.ulSize,
							xParam.pucData);
					if(fatfsSD_printUnitData) {
						float *pfVal = (float*)(xParam.pucData);
						uint32_t imax =  xParam.ulSize / 4 ;
						if(firstIn) {
							firstIn = 0;
							pfVal += (ulOffset_DispBeginToReadBufferBegin / sizeof(float));
							imax -= ulOffset_DispBeginToReadBufferBegin / sizeof(float);
						}
						if(count_willPrint < imax) {
							imax = count_willPrint;
						}
						for(uint32_t i = 0; i < imax; i++)
						{
							printf("%.2f\n",
										*(pfVal + i));
							++count_printed;
						}
						count_willPrint = ulDispWidthPx - count_printed;
					}
				}
				printf("\r\n[read param] ulAddr = %10ld, ulSize = %6ld, ulUnitOffsetFile = %6ld\r\n",
						xParam.ulAddr,
						xParam.ulSize,
						xParam.ulUnitOffsetFile);
			}
		}
	}
}

/**
 * @brief  初始化 FATFS SD 卡任务函数
 * @param  None
 * @retval None
 */
void fatfsSDInit()
{
	fatfsSDTaskHandle = osThreadNew(fatfsSDTask, NULL, &fatfsSDTask_attributes);
}

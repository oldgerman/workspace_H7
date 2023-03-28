/**
  ******************************************************************************
  * @file        tile_wave.cpp
  * @author      OldGerman
  * @created on  Mar 20, 2023
  * @brief       See tile_wave.h for details
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
#include "tile_wave.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Function implementations --------------------------------------------------*/
/**
  * @brief	创建瓦片缓冲区链表
  * @param	None
  * @retval	0 - success, 1 - failure
  */
uint32_t TileWave::createTileBufferList()
{
	ulWaveDispDataSize = ulWaveDispWidth * ulWaveFrameSize;
	/* 确定波形显示区的瓦片缓冲区大小有2种情况
	 * 存储所有波形的介质IOsize在 ulWaveDispTileBufferSizeMin ~ ulIOSizeMax 范围
	 * 1. RAM：IOsize越大，访问速度差不多
	 * 2. ROM：IOsize越大，访问速度越快
	 */
	/* 大于波形显示区数据大小的最小2的幂 */
	ulWaveDispTileBufferSizeMin = ulCalculateSmallestPowerOf2GreaterThan(ulWaveDispDataSize);

	/* ulWaveDispTileBufferSize ≥ ulWaveDispTileBufferSizeMin 且
	 * ulWaveDispTileBufferSize ≥ ulIOSizeMin
	 */
	if(ulWaveDispTileBufferSize < ulWaveDispTileBufferSizeMin) {
		ulWaveDispTileBufferSize = ulWaveDispTileBufferSizeMin;
	}
	if(ulWaveDispBufferSize < ulIOSizeMin) {
		ulWaveDispBufferSize = ulIOSizeMin;
	}

	/** 申请字符串缓冲区的内存，非频繁操作的缓冲区，8 字节对齐即可
	  * reference: blog.csdn.net/fengxinlinux/article/details/51541003
	  */
	ppucStrBuffer_ = (char**)aligned_malloc(sizeof(char**) * ulStrBufferRowCount, 8);
	for(uint32_t i = 0; i < ulLayerNumMax; i++) {
		ppucStrBuffer_[i] = (char*)aligned_malloc(sizeof(char*) * ulLayerNumMax, 8);
	}

	/* 创建层并申请每层的动态内存 */
	uint32_t ulLayerTileBufferSize = ulWaveDispBufferSize;	//4KB
	uint32_t ulLayerTileSize = 1;	// 1B
	uint8_t *pucLayerTileBuffer;
	for(uint32_t i = 0; i < ulLayerNumMax; i++)
	{
		if(ulLayerTileSize < ulIOSizeMin)
			ulLayerTileBufferSize = ulIOSizeMin;
		else
			ulLayerTileBufferSize = ulLayerTileSize;

		/* 申请层瓦片缓冲区的动态内存 */
		pucLayerTileBuffer = (uint8_t*)aligned_malloc(ulLayerTileBufferSize, alignment_);

		sprintf(&ppucStrBuffer_[i][0], "| %15ld | %13ld | %17ld |\r\n",
				DRAM_SRAM1.getMemUsed(), DRAM_SRAM1.getMemFree(), DRAM_SRAM1.getMemFreeMin());

		Layer_t xLayer = {
				.ulLayerNum = i,
				.ulTileSize = ulLayerTileSize,
				.ulTileBufferSize = ulLayerTileBufferSize,
				.ulTileBufferOffset = 0,
				.pucTileBuffer = pucLayerTileBuffer,
				.ulBufferSize = ulLayerTileSize * ulLayerTilesNumMax,
				.ulTileBufferWritePeriod = ulLayerTileBufferSize / ulLayerTileSize
		};
		/* 尾插，链表正向遍历越往后层编号越大 */
		xLayersList.push_back(xLayer);

		/* 更新所有层的瓦片缓冲区的总大小 */
		ulLayersTileBufferSize += ulLayerTileBufferSize;

		/* 倍增一些瓦片参数大小 */
		ulLayerTileBufferSize = ulLayerTileBufferSize << 1;
		ulLayerTileSize = ulLayerTileSize << 1;
	}
	/* 设置周期计数器最大值 */
	ulPeriodMax = (*xLayersList.begin()).ulTileBufferWritePeriod;

	/* 申请读写缓冲区的动态内存 */
	// 实时切片时分配

	// 读缓冲区暂时分 5 个 ulIOSizeMin
//	pucReadBuffer = (uint8_t*)aligned_malloc(5 * ulIOSizeMin, alignment_);

	return 0U;
}

/**
  * @brief	切片前重置一些变量
  * @param	None
  * @retval	Nnoe
  */
void TileWave::resetVariablesBeforeSlice()
{
	/* 每次重新开始切片后需要重置 */
	ulPeriod = 0;
	ulWriteBufferOffsetOld = 0;
	fRealWrittenFreqSum = 0;
	fRealWrittenFreqAvg = 0;
	fRealWrittenFreqNum = 0;

	static uint32_t ulEventNumOld = 0;
	if(ulEventNum != ulEventNumOld) {
		ulEventNumOld = ulEventNum;
		/* 当消息队列的剩余消息数是 0，才可删除并创建新的消息队列 */
		if(osMessageQueueGetCount(xMsgQueue) == 0) {
			osMessageQueueDelete(xMsgQueue);	// 第一次执行时会返回 osErrorParameter
			xMsgQueue = osMessageQueueNew(ulEventNum, sizeof(Event_t), NULL);
		}
	}
}

/**
  * @brief	切片瓦片缓冲区
  * @param	pulData 	Pointer to data buffer
  * @retval	0 - success, 1 - failure
  */
TileWave::WriteRingBufferParam_t TileWave::sliceTileBuffer(uint8_t* pulData)
{
	/* 重置向瓦片缓冲区写地址的偏移 */
	xRit = xLayersList.rbegin();
	for(uint8_t i = 0; i < ulLayerNumMax; i++) {
		(*xRit).ulTileBufferOffset = 0;
		++xRit;
	}

	/* 用于计算本函数被调用的实时频率的单次和平均值 */
	static double fRealWrittenFreq = 0;
	static uint32_t ulTickCountOld =  xTaskGetTickCount();
	uint32_t ulTickCount;

	/* 用于瓦片切片 */
	uint32_t ulWriteBufferOffset = 0;
	uint32_t ulWriteMark = 0;

	/* 瓦片切片 */
	xRit = xLayersList.rbegin();

	/** 从帧缓冲区中复制瓦片大小的数据到瓦片缓冲区
	  * TODO: 从帧缓冲区计算2幂缩放倍率的瓦片大小数据
	  */
	for(uint8_t i = 0; i < ulLayerNumMax; i++) {
		memcpy((*xRit).pucTileBuffer + (*xRit).ulTileBufferOffset, 	// 瓦片缓冲区的地址注意加上地址的偏移
				pulData, (*xRit).ulTileSize); 						// 瓦片大小
		(*xRit).ulTileBufferOffset += (*xRit).ulTileSize; 			// 更新向瓦片缓冲区写地址的偏移

		/** 若 计数器周期 整除 层瓦片缓冲区周期
		  * 说明该层需要向缓冲区发送瓦片缓冲区的所有数据
		  */
		if(ulPeriod % (*xRit).ulTileBufferWritePeriod == 0 )		// 从缓冲区最大的层迭代到最小的
		{
			(*xRit).ulTileBufferOffset = 0;							// 归零瓦片缓冲区的偏移地址
			ulWriteBufferOffset += (*xRit).ulTileBufferSize;		// 更新向缓冲区写地址的偏移
			++ulWriteMark;											// 更新标记记
		}
		++xRit;														// 从最大的层迭代到最小的
	}

    /* 👆 先计算出本周期发送的瓦片缓冲区总大小 ulWriteBufferOffset
     * 然后根据这个总大小才能申请本周期的环形缓冲区的内存 */

	/* 释放上个周期的环形缓冲区的动态内存在 fatfsSDtask 写完成后 ret 返回 0 时释放 */
	// aligned_free

	/* pucWriteRingBuffer 每次的地址会不一样，由 aligned_malloc 从找到的 hole 分配的地址决定
	 * 这些地址不会记录在对象成员中，而是记录在外部消息队列 */
	if(ulSliceButNotWrite == 0) {
		pucWriteRingBuffer = (uint8_t*)aligned_malloc(ulWriteBufferOffset, alignment_);
	} else {
		pucWriteRingBuffer = NULL;
	}
	ulWriteBufferOffset = 0;
	xRit = xLayersList.rbegin();

	for(uint8_t i = 0; i < ulLayerNumMax; i++) {
		if(ulPeriod % (*xRit).ulTileBufferWritePeriod == 0 )
		{
			// 将DRAM中的 非连续储存 的 瓦片缓冲区数据 复制 到 写缓冲区 以变为连续储存的
			if(pucWriteRingBuffer != NULL) {
				memcpy(pucWriteRingBuffer + ulWriteBufferOffset,
						(*xRit).pucTileBuffer, (*xRit).ulTileBufferSize);
			}
			// 更新向缓冲区写地址的偏移
			ulWriteBufferOffset += (*xRit).ulTileBufferSize;
		}
		++xRit; // 从最大的层迭代到最小的
	}
	/* 保存需要写缓冲区时的参数配置 */
	WriteRingBufferParam_t WriteRingBufferParam = {
			.ulAddr   = ulWriteBufferOffsetOld,
			.ulSize   = ulWriteBufferOffset,
			.pucData  = pucWriteRingBuffer,
			.ulPeriod = ulPeriod,
			.ulMark   = ulWriteMark
	};

	/* 打印本次切片详情 */
	if(ulPrintSliceDetail) {
		printf("| sliceTileBuffer | ulPeriod = %4ld | addr = %9ld | size = %5ld | mark = %2ld | \r\n",
			ulPeriod, ulWriteBufferOffsetOld, ulWriteBufferOffset, ulWriteMark);
	}

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
//	printf("sliceFreq: %3.3f, %3.3f\r\n", fRealWrittenFreq, fRealWrittenFreqAvg);

	/* 下次瓦片切片前需要处理的变量 */
	++ulPeriod;	// = 1、2、3...2048;
	ulPeriod %= ulPeriodMax;
	ulWriteBufferOffsetOld += ulWriteBufferOffset;

	return WriteRingBufferParam;
}

/**
  * @brief  The constructor of the TileWave object
  * @param  xConfig	reference to the Config_t
  * @retval N/A
  */
TileWave::TileWave(Config_t &xConfig)
{
	/* IO Size */
	ulIOSize = xConfig.ulIOSize;
	ulIOSizeMin = xConfig.ulIOSizeMin;
	ulIOSizeMax = xConfig.ulIOSizeMax;
    /* Layer */
	ulLayerNum = xConfig.ulLayerNum;
	ulLayerNumMax = xConfig.ulLayerNumMax;
	ulLayerTilesNumMax = xConfig.ulLayerTilesNumMax;
	/* WaveForm */
    ulWaveFrameSize = xConfig.ulWaveFrameSize;
    ulWaveDispWidth = xConfig.ulWaveDispWidth;
	ulWaveDispTileBufferSize = xConfig.ulWaveDispTileBufferSize;
	/* Event */
	ulEventNum = xConfig.ulEventNum;

	ulLayersTileBufferSize = 0;

	ulPeriod = 0;
	ulPeriodMax = 0;
	ulWriteBufferOffsetOld = 0;
	fRealWrittenFreqSum = 0;
	fRealWrittenFreqAvg = 0;
	fRealWrittenFreqNum = 0;

	ulPrintSliceDetail = 0;		// 默认不打印切片的详情信息
	ulSliceButNotWrite = 0;		// 默认切片时写文件
}

/**
  * @brief  初始化动态内存API
  * @param  Malloc	function wrapper
  * @free  	Free	function wrapper
  * @retval None
  */
void TileWave::initMemoryHeapAPI(
		std::function<void* (size_t size, size_t alignment)>	Aligned_malloc,
		std::function<void  (void* ptr_aligned)>				Aligend_free,
		std::function<void  (void* ptr, size_t alignment)> 		Aligned_detect)
{
	aligned_malloc = Aligned_malloc;
	aligend_free = Aligend_free;
	aligned_detect = Aligned_detect;
}

/**
  * @brief  初始化读写API
  * @param  Write	function wrapper
  * @free  	Read	function wrapper
  * @retval None
  */
void TileWave::initReadWriteAPI(
	    std::function<uint32_t (uint32_t addr, uint32_t size, uint8_t* pData)> 	Write,
	    std::function<uint32_t (uint32_t addr, uint32_t size, uint8_t* pData)>	Read)
{
	write = Write;
	read = Read;;
}

/**
  * @brief  打执行 createTileBufferList() 时记录的层信息
  * @param  None
  * @retval None
  */
void TileWave::vPrintLayerInfo()
{
	printf("| 层编号 | 瓦片大小 | 瓦片缓冲区大小 | 瓦片缓冲区地址 | 缓冲区大小 | 缓冲区发送周期 | DRAM 当前共使用 | DRAM 当前剩余 | DRAM 历史最少可用 |\r\n");
	printf("| ------ | -------- | -------------- | -------------- | ---------- | -------------- | --------------- | ------------- | ----------------- |\r\n");
	xIt = xLayersList.begin();
	for(uint8_t i = 0; i < ulLayerNumMax; i++) {
		printf("| %6ld | %8ld | %14ld | %14p | %10ld | %14ld %s",
				(*xIt).ulLayerNum,
				(*xIt).ulTileSize,
				(*xIt).ulTileBufferSize,
				(*xIt).pucTileBuffer,
				(*xIt).ulBufferSize,
				(*xIt).ulTileBufferWritePeriod,
				&ppucStrBuffer_[i][0]);
		++xIt;
	}
}

/**
  * @brief  计算大于某数的最小2的幂
  * @param  ulValue	calculated reference value
  * @retval calculated value
  */
uint32_t TileWave::ulCalculateSmallestPowerOf2GreaterThan(uint32_t ulValue)
{
	uint32_t ulNth = 1;
	for(uint8_t i = 0; i < 32; i++)
	{
		ulNth = ulNth << i;
		if(ulNth > ulValue)
			break;
	}
	return ulNth;
}

/**
  * @brief  计算写环形缓冲区的最大大小
  * @param  ulValue	calculated reference value
  * @retval calculated value
  */
//void TileWave::vCalculateWriteRingBufferSizeMax()
// 改用 aligned_malloc 实时分配内存

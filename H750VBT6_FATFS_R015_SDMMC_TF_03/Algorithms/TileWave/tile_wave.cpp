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
	uint32_t ulLayerTileSize = 1;	//1B
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
		/* 尾插入，链表正向遍历越往后层编号越大 */
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
	// 写缓冲区等于瓦片缓冲区大小的总和，暂不考虑.dsppk 文件格式开头自定义的文件信息预留大小
//	pucWriteBuffer = (uint8_t*)aligned_malloc(ulLayersTileBufferSize, alignment_);
	pucWriteBuffer = (uint8_t*)aligned_malloc(ulLayersTileBufferSize, alignment_);

	// 读缓冲区暂时分 5 个 ulIOSizeMin
	pucReadBuffer = (uint8_t*)aligned_malloc(5 * ulIOSizeMin, alignment_);

	return 0U;
}

/**
  * @brief	切片前重置一些变量
  * @param	None
  * @retval	Nnoe
  */
void TileWave::resetVariablesBeforeSlice()
{
	// 每次重新开始切片后需要重置
	ulPeriod = 0;
	ulWriteBufferOffsetOld = 0;
	fRealWrittenFreqSum = 0;
	fRealWrittenFreqAvg = 0;
	fRealWrittenFreqNum = 0;
}

/**
  * @brief	切片瓦片缓冲区
  * @param	pulData 	Pointer to data buffer
  * @retval	0 - success, 1 - failure
  */
TileWave::WriteBufferParam_t TileWave::sliceTileBuffer(uint8_t* pulData)
{
	/* 重置向瓦片缓冲区写入地址的偏移 */
	xRit = xLayersList.rbegin();
	for(uint8_t i = 0; i < ulLayerNumMax; i++) {
		(*xRit).ulTileBufferOffset = 0;
		++xRit;
	}

	/* 用于计算本函数被调用的实时频率的单次和平均值 */
	static double fRealWrittenFreq = 0;
	static uint32_t ulWrittenCount = 0;
	static uint32_t ulTickCountOld =  xTaskGetTickCount();
	uint32_t ulTickCount;

	/* 用于瓦片切片 */
	uint32_t ulWriteBufferOffset = 0;
	uint32_t ulWriteMark = 0;

	/* 瓦片切片 */
	++ulPeriod;	// = 1、2、3...2048;
	xRit = xLayersList.rbegin();
	for(uint8_t i = 0; i < ulLayerNumMax; i++) {
		/** 从帧缓冲区中复制瓦片大小的数据到瓦片缓冲区
		  * TODO: 从帧缓冲区计算2幂缩放倍率的瓦片大小数据
		  */
		memcpy((*xRit).pucTileBuffer + (*xRit).ulTileBufferOffset, 	// 注意加上偏移地址
				pulData, (*xRit).ulTileSize);

		/* 更新写入瓦片缓冲区的偏移地址 */
		(*xRit).ulTileBufferOffset += (*xRit).ulTileSize;

		/** 若 计数器周期 整除 层瓦片缓冲区周期
		  * 说明该层需要向缓冲区发送瓦片缓冲区的所有数据
		  */
		if(ulPeriod % (*xRit).ulTileBufferWritePeriod == 0 ) // 从缓冲区最大的层迭代到最小的
		{
			/* 归零瓦片缓冲区的偏移地址 */
			(*xRit).ulTileBufferOffset = 0;

			// 将DRAM中的 非连续储存 的 瓦片缓冲区数据 复制 到 发送缓冲区 以变为连续储存的
			memcpy(pucWriteBuffer + ulWriteBufferOffset, (*xRit).pucTileBuffer, (*xRit).ulTileBufferSize);
			// 更新向缓冲区写入的起始地址偏移
			ulWriteBufferOffset += (*xRit).ulTileBufferSize;

			++ulWriteMark;
		}
		++xRit;
	}

	/* 向层缓冲区写入数据 */
//	if(ulSliceButNotWrite == 0) {
//		ret = write(ulWriteBufferOffsetOld, ulWriteBufferOffset, pucWriteBuffer);
//	}
	/* 保存需要写入缓冲区的信息 */
	WriteBufferParam_t writeBufferParam = {
			.ulAddr   = ulWriteBufferOffsetOld,
			.ulSize   = ulWriteBufferOffset,
			.pucData  = pucWriteBuffer,
			.ulPeriod = ulPeriod,
			.ulMark   = ulWriteMark
	};

	/* 打印本次切片详情 */
	if(ulPrintSliceDetail) {
		printf("| sliceTileBuffer | ulPeriod = %4ld | addr = %10ld | size = %9ld | mark = %2ld | \r\n",
			ulPeriod, ulWriteBufferOffsetOld, ulWriteBufferOffset, ulWriteMark);
	}

	/* 计算实时频率的单次和平均值 */
	++ulWrittenCount;
	ulTickCount = xTaskGetTickCount();	/* 获取当前的系统时间 */
	uint32_t ulTickOffest = ulTickCount - ulTickCountOld;
	ulTickCountOld = ulTickCount;
	fRealWrittenFreq = (double)1000 / ((double)ulTickOffest / ulWrittenCount);

	// 第一次 fRealWrittenFreq 肯定是 inf，需要舍弃
	// fRealWrittenFreq 是一个正常的浮点数才会计算
	if(isnormal(fRealWrittenFreq))	{
		++fRealWrittenFreqNum;
		fRealWrittenFreqSum += fRealWrittenFreq;
		fRealWrittenFreqAvg = fRealWrittenFreqSum / fRealWrittenFreqNum;
	}
	ulWrittenCount = 0;
	printf("freq: %3.3f, %3.3f\r\n", fRealWrittenFreq, fRealWrittenFreqAvg);

	/* 下次瓦片切片前需要处理的变量 */
	ulPeriod %= ulPeriodMax;
	ulWriteBufferOffsetOld += ulWriteBufferOffset;

	return writeBufferParam;
}

/**
  * @brief  The constructor of the TileWave object
  * @param  xConfig	reference to the Config_t
  * @retval N/A
  */
TileWave::TileWave(Config_t &xConfig)
{
	ulIOSize = xConfig.ulIOSize;
	ulIOSizeMin = xConfig.ulIOSizeMin;
	ulIOSizeMax = xConfig.ulIOSizeMax;
	ulLayerNum = xConfig.ulLayerNum;
	ulLayerNumMax = xConfig.ulLayerNumMax;
	ulLayerTilesNumMax = xConfig.ulLayerTilesNumMax;
    ulWaveFrameSize = xConfig.ulWaveFrameSize;
    ulWaveDispWidth = xConfig.ulWaveDispWidth;
	ulWaveDispTileBufferSize = xConfig.ulWaveDispTileBufferSize;

	ulLayersTileBufferSize = 0;

	ulPeriod = 0;
	ulPeriodMax = 0;
	ulWriteBufferOffsetOld = 0;
	fRealWrittenFreqSum = 0;
	fRealWrittenFreqAvg = 0;
	fRealWrittenFreqNum = 0;

	ulPrintSliceDetail = 0;		//默认打印切片的详情信息
//	ulSliceButNotWrite = 0;		//默认切片时写入文件
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
  * @brief  计算写入循环缓冲区的最大大小
  * @param  ulValue	calculated reference value
  * @retval calculated value
  */
//void TileWave::vCalculateWriteRingBufferSizeMax()
// 改用 aligned_malloc 实时分配内存

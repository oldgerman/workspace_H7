/**
  ******************************************************************************
  * @file        tile_wave.cpp
  * @author      OldGerman
  * @created on  Mar 20, 2023
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
#include "tile_wave.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
std::function<void* (size_t size)> 		TileWave::malloc;
std::function<void  (void* ptr)>		TileWave::free;
std::function<void* (size_t size, size_t alignment)>	TileWave::aligned_malloc;
std::function<void  (void* ptr_aligned)>				TileWave::aligend_free;
std::function<void  (void* ptr, size_t alignment)> 		TileWave::aligned_detect;

std::function<uint32_t (uint32_t addr, uint32_t size, uint8_t* pData)> 		TileWave::write;
std::function<uint32_t (uint32_t addr, uint32_t size, uint8_t* pData)>		TileWave::read;

__attribute__((section(".RAM_D1_Array"))) ALIGN_32BYTES(uint8_t TxBuffer [64 *1024]);
__attribute__((section(".RAM_D1_Array"))) ALIGN_32BYTES(uint8_t RxBuffer [10 *1024]);

/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Function implementations --------------------------------------------------*/
/**
  * @brief	初始化瓦片缓冲区链表
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

	/* 创建层并申请每层的动态内存 */
	uint32_t ulLayerNum;
	uint32_t ulLayerTileBufferSize = ulWaveDispBufferSize;	//4KB
	uint32_t ulLayerTileSize = ulLayerTileBufferSize / ulLayerTilesNumMax; // 2B = 4K / 2048
	void *pucLayerTileBuffer;
	for(ulLayerNum = 0; ulLayerNum < ulLayerNumMax; ulLayerNum++)
	{
		/* 申请层瓦片缓冲区的动态内存 */
		if(ulLayerTileSize < ulIOSizeMin)
			ulLayerTileBufferSize = ulIOSizeMin;
		else
			ulLayerTileBufferSize = ulLayerTileSize;

		pucLayerTileBuffer = malloc(ulLayerTileBufferSize);

		sprintf(&(ucStrBuffer[ulLayerNum][0]), "| %15ld | %13ld | %17ld |\r\n",
				DRAM_SRAM1.getMemUsed(), DRAM_SRAM1.getMemFree(), DRAM_SRAM1.getMemFreeMin());

		Layer_t xLayer = {
				.ulLayerNum = ulLayerNum,
				.ulTileSize = ulLayerTileSize,
				.ulTileBufferSize = ulLayerTileBufferSize,
				.ulTileBufferOffset = 0,
				.pucTileBuffer = pucLayerTileBuffer,
				.ulBufferSize = ulLayerTileSize * ulLayerTilesNumMax,
				.ulTileBufferTxPeriod = ulLayerTileBufferSize / ulLayerTileSize
		};
		/* 尾插入，链表正向遍历越往后层编号越大 */
		xLayersList.push_back(xLayer);

		ulLayersTileBufferSize += ulLayerTileBufferSize;

		/* 倍增大小 */
		ulLayerTileBufferSize = ulLayerTileBufferSize << 1;
		ulLayerTileSize = ulLayerTileSize << 1;
	}

	// malloc未实现申请32字节对齐的内存
	ucpTxBuffer = TxBuffer;
	ucpRxBuffer = RxBuffer;

	return 0U;
}

/**
  * @brief	重置向瓦片缓冲区写入地址的偏移
  * @param	None
  * @retval	None
  */
void TileWave::resetTileBufferOffset() {
	xRit = xLayersList.rbegin();
	for(uint8_t i = 0; i < ulLayerNumMax; i++) {
		(*xRit).ulTileBufferOffset = 0;
		++xRit;
	}
}

/**
  * @brief	切片瓦片缓冲区并写入
  * @param	pulData 	Pointer to data buffer
  * @retval	0 - success, 1 - failure
  */
uint32_t TileWave::writeTileBuffer(uint8_t* pulData) {
	/* FATFS 返回值 */
	uint32_t ret = 0;

	/* 用于计算本函数被调用的实时频率的单次和平均值 */
	static double fRealWrittenFreq = 0;
	static uint32_t ulWrittenCount = 0;
	static uint32_t ulTickCountOld =  xTaskGetTickCount();
	uint32_t ulTickCount;

	/* 用于瓦片切片 */
	uint32_t ulPeriodMax = (*xLayersList.begin()).ulTileBufferTxPeriod;
	uint32_t ulTxBufferOffset = 0;
	char cWrittenMark = 'a';

	/* 瓦片切片 */
	++ulPeriod;	// = 1、2、3...2048;
	xRit = xLayersList.rbegin();
	for(uint8_t i = 0; i < ulLayerNumMax; i++) {
		/** 从帧缓冲区中复制瓦片大小的数据到瓦片缓冲区
		  * TODO: 从帧缓冲区计算2幂缩放倍率的瓦片大小数据
		  */
		memcpy((uint8_t*)((*xRit).pucTileBuffer) + (*xRit).ulTileBufferOffset, 	// 注意加上偏移地址
				pulData, (*xRit).ulTileSize);

		/* 更新写入瓦片缓冲区的偏移地址 */
		(*xRit).ulTileBufferOffset += (*xRit).ulTileSize;

		/** 若 计数器周期 整除 层瓦片缓冲区周期
		  * 说明该层需要向缓冲区发送瓦片缓冲区的所有数据
		  */
		if(ulPeriod % (*xRit).ulTileBufferTxPeriod == 0 ) // 从缓冲区最大的层迭代到最小的
		{
			/* 归零瓦片缓冲区的偏移地址 */
			(*xRit).ulTileBufferOffset = 0;

			// 将DRAM中的 非连续储存 的 瓦片缓冲区数据 复制 到 发送缓冲区 以变为连续储存的
			memcpy((uint8_t*)ucpTxBuffer + ulTxBufferOffset, (*xRit).pucTileBuffer, (*xRit).ulTileBufferSize);
			// 更新向缓冲区写入的起始地址偏移
			ulTxBufferOffset += (*xRit).ulTileBufferSize;

			++cWrittenMark;
		}
		++xRit;
	}

	/* 向层缓冲区写入数据 */
	if(ulSliceButNotWrite == 0) {
		ret = write(ulTxBufferOffsetOld, ulTxBufferOffset, (uint8_t *)ucpTxBuffer);
	}

	/* 打印本次切片详情 */
	if(ulPrintSliceDetail) {
		printf("| writeTileBuffer | ulPeriod = %4ld | ret = %2ld | addr = %10ld | size = %9ld | mark = %c | \r\n",
			ulPeriod, ret, ulTxBufferOffsetOld, ulTxBufferOffset, cWrittenMark);
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
	ulTxBufferOffsetOld += ulTxBufferOffset;

	return ret;
}

/**
  * @brief  The constructor of the TileWave object
  * @param  xConfig	reference to the Config_t
  * @retval N/A
  */
TileWave::TileWave(Config_t &xConfig) {
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

	memset(ucStrBuffer, 0, sizeof(ucStrBuffer));
	ulPeriod = 0;
	ulTxBufferOffsetOld = 0;
	fRealWrittenFreqSum = 0;
	fRealWrittenFreqAvg = 0;
	fRealWrittenFreqNum = 0;
	ulPrintSliceDetail = 0;		//默认打印切片的详情信息
	ulSliceButNotWrite = 0;		//默认切片时写入文件
}

/**
  * @brief  初始化动态内存API
  * @param  Malloc	function object
  * @free  	Free	function object
  * @retval None
  */
void TileWave::initMemoryHeapAPI(
		std::function<void* (size_t)> 	Malloc,
		std::function<void  (void*)>	Free,
		std::function<void* (size_t size, size_t alignment)>	Aligned_malloc,
		std::function<void  (void* ptr_aligned)>				Aligend_free,
		std::function<void  (void* ptr, size_t alignment)> 		Aligned_detect)
{
	malloc = Malloc;
	free = Free;
	aligned_malloc = Aligned_malloc;
	aligend_free = Aligend_free;
	aligned_detect = Aligned_detect;
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
				(*xIt).ulTileBufferTxPeriod,
				&(ucStrBuffer[i][0]));
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
  * @brief  测试动态内存API
  * @param  ulTimes		申请的次数
  * @param  ulStartSize	起始申请大小，单位B
  * @retval None
  */
void TileWave::vTestMallocFree(uint32_t ulTimes, uint32_t ulStartSize)
{
	uint32_t ulDynamicSize;
	void  *SRAM1_Addr[ulTimes];

	printf("\r\n【测试动态内存分配 %ld 次，起始大小 %ld ，每次大小翻倍，单位 byte 】\r\n",
			ulTimes, ulStartSize);
	printf("【从DRAM申请空间】\r\n");
	printf("| 已申请次数 | 内存池大小 |  申请大小  |  申请地址  |   共使用   |    剩余    | 历史最少可用 |\r\n");
	printf("| ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ------------ |\r\n");
	for(uint32_t i = 0; i < ulTimes; i++) {
		ulDynamicSize = ulStartSize << i;
		SRAM1_Addr[i] = DRAM_SRAM1.malloc(ulDynamicSize);
		printf("| %10ld | %10ld | %10ld | %10p | %10ld | %10ld | %12ld |\r\n",
				i + 1,
				DRAM_SRAM1.getMemSize(),
				ulDynamicSize, SRAM1_Addr[i],
				DRAM_SRAM1.getMemUsed(),
				DRAM_SRAM1.getMemFree(),
				DRAM_SRAM1.getMemFreeMin());
	}

	printf("\r\n【释放从DRAM申请的空间】\r\n");
	printf("|  释放次数  | 内存池大小 |  释放大小  |  释放地址  |   共使用   |    剩余    | 历史最少可用 |\r\n");
	printf("| ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ------------ |\r\n");
	for(uint32_t i = 0; i < ulTimes; i++) {
		ulDynamicSize = ulStartSize << i;
		DRAM_SRAM1.free(SRAM1_Addr[i]);
		printf("| %10ld | %10ld | %10ld | %10p | %10ld | %10ld | %12ld |\r\n",
				i + 1,
				DRAM_SRAM1.getMemSize(),
				ulDynamicSize, SRAM1_Addr[i],
				DRAM_SRAM1.getMemUsed(),
				DRAM_SRAM1.getMemFree(),
				DRAM_SRAM1.getMemFreeMin());
	}
}

/**
  * @brief  测试字节对齐动态内存API
  * @param  ulTimes		申请的次数
  * @param  ulStartSize	起始申请大小，单位B
  * @param	ulAlignment	地址对齐的大小
  * @retval None
  */
void TileWave::vTestAlignedMallocFree(uint32_t ulTimes, uint32_t ulStartSize, uint32_t ulAlignment)
{
	uint32_t ulDynamicSize;
	void  *SRAM1_Addr[ulTimes];

	printf("\r\n【测试 %ld 字节对齐的动态内存分配 %ld 次，起始大小 %ld ，每次大小翻倍，单位 byte 】\r\n",
			ulAlignment, ulTimes, ulStartSize);
	printf("【从DRAM申请空间】\r\n");
	printf("| 已申请次数 | 内存池大小 |  申请大小  |  申请地址  |  字节对齐  |   共使用   |    剩余    | 历史最少可用 |\r\n");
	printf("| ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- |------------ |\r\n");
	for(uint32_t i = 0; i < ulTimes; i++) {
		ulDynamicSize = ulStartSize << i;
		SRAM1_Addr[i] = DRAM_SRAM1.aligned_malloc(ulDynamicSize, ulAlignment);
		printf("| %10ld | %10ld | %10ld | %10p | %10s | %10ld | %10ld | %12ld |\r\n",
				i + 1,
				DRAM_SRAM1.getMemSize(),
				ulDynamicSize, SRAM1_Addr[i],
				(DRAM_SRAM1.aligned_detect(SRAM1_Addr[i], ulAlignment) == 0)?("Yes"):("No"),
				DRAM_SRAM1.getMemUsed(),
				DRAM_SRAM1.getMemFree(),
				DRAM_SRAM1.getMemFreeMin());
	}

	printf("\r\n【释放从DRAM申请的空间】\r\n");
	printf("|  释放次数  | 内存池大小 |  释放大小  |  释放地址  |  字节对齐  |   共使用   |    剩余    | 历史最少可用 |\r\n");
	printf("| ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ------------ |\r\n");
	for(uint32_t i = 0; i < ulTimes; i++) {
		ulDynamicSize = ulStartSize << i;
		DRAM_SRAM1.aligned_free(SRAM1_Addr[i]);
		printf("| %10ld | %10ld | %10ld | %10p | %10s | %10ld | %10ld | %12ld |\r\n",
				i + 1,
				DRAM_SRAM1.getMemSize(),
				ulDynamicSize, SRAM1_Addr[i],
				(DRAM_SRAM1.aligned_detect(SRAM1_Addr[i], ulAlignment) == 0)?("Yes"):("No"),
				DRAM_SRAM1.getMemUsed(),
				DRAM_SRAM1.getMemFree(),
				DRAM_SRAM1.getMemFreeMin());
	}
}

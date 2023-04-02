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
#define constrain(amt, low, high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

/* Exported constants --------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Function implementations --------------------------------------------------*/

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
	ulLayerTileNumMax = xConfig.ulLayerTileNumMax;
	/* WaveForm */
    ulWaveFrameSize = xConfig.ulWaveFrameSize;
    ulWaveDispWidth = xConfig.ulWaveDispWidth;
	ulWaveDispTileBufferSize = xConfig.ulWaveDispTileBufferSize;
	/* Event */
	ulEventNum = xConfig.ulEventNum;

	ulLayerTileBufferSizeAll = 0;

	ulPeriod = 1;
	ulPeriodMax = 0;
	ulWriteBufferOffsetOld = 0;
	fRealWrittenFreqSum = 0;
	fRealWrittenFreqAvg = 0;
	fRealWrittenFreqNum = 0;

	ulPrintSliceDetail = 0;		// 默认不打印切片的详情信息
	ulSliceButNotWrite = 0;		// 默认切片时写文件
}

/**
  * @brief  The destructor of the TileWave object
  * @param  None
  * @retval N/A
  */
TileWave::~TileWave()
{
//	aligned_free(ppucStrBuffer_, 8);
//	ppucStrBuffer_ = (char**)aligned_malloc(sizeof(char**) * ulStrBufferRowCount_, 8);
//	for(uint32_t i = 0; i < ulLayerNumMax; i++) {
//		ppucStrBuffer_[i] = (char*)aligned_malloc(sizeof(char*) * ulLayerNumMax, 8);
//	}
}


/**
  * @brief	创建层向量
  * @param	None
  * @retval	0 - success, 1 - failure
  */
uint32_t TileWave::createLayerTable()
{
	ulWaveDispDataSize = ulWaveDispWidth * ulWaveFrameSize;
	/* 确定波形显示区的瓦片缓冲区大小有2种情况
	 * 存储所有波形的介质IOsize在 ulWaveDispTileBufferSizeMin ~ ulIOSizeMax 范围
	 * 1. RAM：IOsize越大，访问速度差不多
	 * 2. ROM：IOsize越大，访问速度越快
	 */
	/* 大于波形显示区数据大小的最小2的幂 */
	ulWaveDispTileBufferSizeMin = ulCalculateMinPowerOf2GreaterThan(ulWaveDispDataSize);

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
	ppucStrBuffer_ = (char**)aligned_malloc(sizeof(char**) * ulStrBufferRowCount_, 8);
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
				.ulLayerBufferSize = ulLayerTileSize * ulLayerTileNumMax,
				.ulTileBufferPeriod = ulLayerTileBufferSize / ulLayerTileSize
		};
		/* 计算单元个数 */
		xLayer.ulTileBufferUnitNum = xLayer.ulTileBufferSize / ulIOSizeMin;
		xLayer.ulLayerBufferUnitNum = xLayer.ulLayerBufferSize / ulIOSizeMin;

		/* 尾插，表格正向遍历越往后层编号越大 */
		xLayerTable.push_back(xLayer);

		/* 更新所有层的瓦片缓冲区的总大小 */
		ulLayerTileBufferSizeAll += ulLayerTileBufferSize;

		/* 倍增一些瓦片参数大小 */
		ulLayerTileBufferSize = ulLayerTileBufferSize << 1;
		ulLayerTileSize = ulLayerTileSize << 1;
	}
	/* 设置周期计数器最大值 */
	ulPeriodMax = (*xLayerTable.begin()).ulTileBufferPeriod;

	/* 申请写缓冲区的动态内存 */
	// 实时切片时分配
	/* 申请读缓冲区的动态内存 */
	// 实时读时分配
	pucReadLayerBuffer = (uint8_t*)aligned_malloc(128*1024, alignment_);

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
	ulPeriod = 1;
	ulWriteBufferOffsetOld = 0;
	fRealWrittenFreqSum = 0;
	fRealWrittenFreqAvg = 0;
	fRealWrittenFreqNum = 0;

	/* 重置向瓦片缓冲区写地址的偏移 */
	for(uint8_t i = 0; i < ulLayerNumMax; i++) {
		xLayerTable[i].ulTileBufferOffset = 0;
	}

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
  * @param xEventType	must be EVENT_WRITE_Layer_BUFFER or EVENT_LAST_WRITE_Layer_BUFFER
  * @retval	0 - success, 1 - failure
  */
TileWave::WriteLayerBufferParam_t TileWave::sliceTileBuffer(uint8_t* pulData, EventType_t xEventType)
{
	std::vector<Layer_t>::reverse_iterator xRit; 	// 层表格的反向迭代器

	/* 用于计算本函数被调用的实时频率的单次和平均值 */
	static double fRealWrittenFreq = 0;
	static uint32_t ulTickCountOld =  xTaskGetTickCount();
	uint32_t ulTickCount;

	/* 用于瓦片切片 */
	uint32_t ulWriteBufferOffset = 0;
	uint32_t ulWriteMark = 0;

	/* 瓦片切片 */
	xRit = xLayerTable.rbegin();
	/* 从帧缓冲区中复制瓦片大小的数据到瓦片缓冲区 */
	for(uint8_t i = 0; i < ulLayerNumMax; i++) {
		memcpy((*xRit).pucTileBuffer + (*xRit).ulTileBufferOffset, 	// 瓦片缓冲区的地址注意加上地址的偏移
				pulData, (*xRit).ulTileSize); 						// 瓦片大小
		(*xRit).ulTileBufferOffset += (*xRit).ulTileSize; 			// 更新向瓦片缓冲区写地址的偏移

		 /* 从帧缓冲区计算2倍缩放的瓦片大小数据 */
		float* pfPtr = (float*)pulData;
		float fVal;
		for(uint32_t j = 0; j < (*xRit).ulTileSize / 4; j++) {
			fVal = ( *(pfPtr + j * 2) + *(pfPtr + j * 2 + 1) ) / 2; // 暂时用2点中值
			if(isnormal(fVal)) {	// 可能到 inf
				*(pfPtr + j) = fVal;
			} else {
				for(uint32_t i = j; i < (*xRit).ulTileSize / 4; i++) {
					*(pfPtr + i) = 0.f;
				}
				break;
			}
		}

		/** 若 计数器周期 整除 层瓦片缓冲区周期
		  * 说明该层需要向缓冲区发送瓦片缓冲区的所有数据
		  */
		if((xEventType == EVENT_WRITE_LAYER_BUFFER &&
				ulPeriod % (*xRit).ulTileBufferPeriod == 0 )|| // 从缓冲区最大的层迭代到最小的
				xEventType == EVENT_LAST_WRITE_LAYER_BUFFER) {
			(*xRit).ulTileBufferOffset = 0;							// 归零瓦片缓冲区的偏移地址
			ulWriteBufferOffset += (*xRit).ulTileBufferSize;		// 更新向缓冲区写地址的偏移
			++ulWriteMark;											// 更新标记
		}
		++xRit;														// 从最大的层迭代到最小的
	}

    /* 👆 先计算出本周期发送的瓦片缓冲区总大小 ulWriteBufferOffset
     * 然后根据这个总大小才能申请本周期的层缓冲区的内存 */

	/* 释放上个周期的层缓冲区的动态内存在 fatfsSDtask 写完成后释放 */
	// aligned_free

	/* pucWriteLayerBuffer 每次的地址会不一样，由 aligned_malloc 从找到的 hole 分配的地址决定
	 * 这些地址不会记录在对象成员中，而是记录在外部消息队列 */
	if(ulSliceButNotWrite == 0) {
		pucWriteLayerBuffer = (uint8_t*)aligned_malloc(ulWriteBufferOffset, alignment_);
	} else {
		pucWriteLayerBuffer = NULL;
	}
	ulWriteBufferOffset = 0;
	xRit = xLayerTable.rbegin();

	for(uint8_t i = 0; i < ulLayerNumMax; i++) {
		if((xEventType == EVENT_WRITE_LAYER_BUFFER &&
				ulPeriod % (*xRit).ulTileBufferPeriod == 0 )|| // 从缓冲区最大的层迭代到最小的
				xEventType == EVENT_LAST_WRITE_LAYER_BUFFER) {
			// 将DRAM中的 非连续储存 的 瓦片缓冲区数据 复制 到 写缓冲区 以变为连续储存的
			if(pucWriteLayerBuffer != NULL) {
				memcpy(pucWriteLayerBuffer + ulWriteBufferOffset,
						(*xRit).pucTileBuffer, (*xRit).ulTileBufferSize);
			}
			// 更新向缓冲区写地址的偏移
			ulWriteBufferOffset += (*xRit).ulTileBufferSize;
		}
		++xRit; // 从最大的层迭代到最小的
	}

	/* 保存需要写缓冲区时的参数配置 */
	WriteLayerBufferParam_t xWriteLayerBufferParam = {
			.ulAddr   = ulWriteBufferOffsetOld,
			.ulSize   = ulWriteBufferOffset,
			.pucData  = pucWriteLayerBuffer,
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
	ulPeriod %= ulPeriodMax;
	++ulPeriod;	// = 1、2、3...2048;
	ulWriteBufferOffsetOld += ulWriteBufferOffset;

	return xWriteLayerBufferParam;
}

/**
  * @brief  初始化动态内存API
  * @param  Malloc	function wrapper
  * @free  	Free	function wrapper
  * @retval None
  */
void TileWave::initMemoryHeapAPI(
		std::function<void* (size_t size, size_t alignment)>	Aligned_malloc,
		std::function<void  (void* ptr_aligned)>				Aligned_free,
		std::function<void  (void* ptr, size_t alignment)> 		Aligned_detect)
{
	aligned_malloc = Aligned_malloc;
	aligned_free = Aligned_free;
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
  * @brief  打执行 createTileBufferTable() 时记录的层信息
  * @param  None
  * @retval None
  */
void TileWave::vPrintLayerInfo()
{
	printf("| 层编号 | 瓦片大小 | 瓦片缓冲区大小 | 瓦片缓冲区地址 | 缓冲区大小 | 缓冲区发送周期 | DRAM 当前共使用 | DRAM 当前剩余 | DRAM 历史最少可用 |\r\n");
	printf("| ------ | -------- | -------------- | -------------- | ---------- | -------------- | --------------- | ------------- | ----------------- |\r\n");

	std::vector<Layer_t>::iterator xIt;			// 层表格的正向迭代器
	xIt = xLayerTable.begin();
	for(uint8_t i = 0; i < ulLayerNumMax; i++) {
		printf("| %6ld | %8ld | %14ld | %14p | %10ld | %14ld %s",
				(*xIt).ulLayerNum,
				(*xIt).ulTileSize,
				(*xIt).ulTileBufferSize,
				(*xIt).pucTileBuffer,
				(*xIt).ulLayerBufferSize,
				(*xIt).ulTileBufferPeriod,
				&ppucStrBuffer_[i][0]);
		++xIt;
	}
}

/**
  * @brief  计算大于某数的最小2的幂
  * @param  ulValue	calculated reference value
  * @retval calculated value
  */
uint32_t TileWave::ulCalculateMinPowerOf2GreaterThan(uint32_t ulValue)
{
	uint32_t ulNth;
	for(uint8_t i = 0; i < 32; i++) {
		ulNth = 1 << i;
		if(ulNth > ulValue) {
			break;
		}
	}
	return ulNth;
}

/**
  * @brief  计算小于等于某数的最大2的幂
  * @param  ulValue	calculated reference value
  * @retval calculated value
  */
uint32_t TileWave::ulCalculateMaxPowerOf2LessThan(uint32_t ulValue)
{
	uint32_t ulNth;
	for(uint8_t i = 0; i < 32; i++) {
		ulNth = 1 << i;
		if(ulNth > ulValue) {
			ulNth = ulNth >> 1;
			break;
		}
	}
	return ulNth;
}

/**
  * @brief  计算2的N次幂的指数
  * @param  ulValue 被计算数
  * @retval 计算的指数
  */
uint32_t TileWave::ulCalculateExponentPowerOf2(uint32_t ulValue)
{
	uint8_t ulExponent = 0;
	for(; ulExponent < 32; ulExponent++){
		if((ulValue >> (ulExponent + 1)) == 0){
			break;
		}
	}
	return ulExponent;
}

/**
  * @brief  计算文件写满的大小
  * @param  None
  * @retval calculated value
  */
uint32_t TileWave::ulCalculateFileSizeFull()
{
	std::vector<Layer_t>::reverse_iterator xRit; 	// 层表格的反向迭代器
	xRit = xLayerTable.rbegin();
	return 2 * (*xRit).ulLayerBufferSize;
}

/**
  * @brief  计算文件任意周期的大小
  * @param  None
  * @retval calculated value
  */
uint32_t TileWave::ulCalculateFileSizeForAnyPeriod(uint32_t ulPeriod)
{
	// TODO
	return 0U;
}

/**
  * @brief  根据缩放倍率和进度计算读缓冲区时的参数配置
  */
// TODO

/**
  * @brief  根据层号、单元偏移、单元个数计算读缓冲区时的参数配置，
  *         多个单元可能是非连续的储存，因此返回多个
  * @param ulLayerNum    层编号 0-14
  * @param ulUnitOffset  单元偏移 >=0
  * @param ulUnitNum     单元个数 >=1，不保证相对于层的单元连续性
  */
TileWave::ReadLayerBufferParamList_t TileWave::xFindUnitList(
		uint32_t ulLayerNum, uint32_t ulUnitOffset, uint32_t ulUnitNum)
{
	ReadLayerBufferParamList_t xRead;

	/* 瓦片缓冲区大小等于单元大小的层的多个单元的连续读取，读取参数组的个数一定等于被读单元数 */
	ReadLayerBufferParam_t xParam = xFindUnit(ulLayerNum, ulUnitOffset, 1);
	xRead.xParamList.push_front(xParam);
	return  xRead;
}

/**
  * @brief  根据层号、单元偏移、单元个数计算读缓冲区时的参数配置，
  *         多个单元可能是非连续的储存，因此返回多个
  * @param ulLayerNum    层编号 0-14
  * @param ulUnitOffset  单元偏移 >=0
  * @param ulUnitNum     单元个数 >=1，不保证相对于层的单元连续性
  */
TileWave::ReadLayerBufferParamList_t TileWave::xFindUnitList(
		uint32_t ulLayerNum, uint32_t ulUnitOffset, uint32_t ulUnitNum)
{
	ReadLayerBufferParamList_t xRead;

	/* 瓦片缓冲区大小等于单元大小的层的多个单元的连续读取，读取参数组的个数一定等于被读单元数 */
	ReadLayerBufferParam_t xParam = xFindUnit(ulLayerNum, ulUnitOffset, 1);
	xRead.xParamList.push_front(xParam);
	return  xRead;
}

/**
  * @brief  根据层号、单元偏移、单元个数计算读缓冲区时的参数配置
  *         1个单元等于最小 IO SIZE
  *         例如 64MB 的层 有 32768 个 2KB 单元
  * @param ulLayerNum    层编号 0-14
  * @param ulUnitOffset  单元偏移 [0, 32767]
  * @param ulUnitNum     单元个数 >=1，不保证相对于层的单元连续性
  */
TileWave::ReadLayerBufferParam_t TileWave::xFindUnit(
		uint32_t ulLayerNum, uint32_t ulUnitOffset, uint32_t ulUnitNum)
{
	/* 约束参数到有效范围 */
	ulLayerNum = constrain(ulLayerNum, 0, xLayerTable.back().ulLayerNum);  // 0 ~ 14
	ulUnitOffset = constrain(ulUnitOffset, 0, xLayerTable[ulLayerNum].ulLayerBufferUnitNum - 1);     // 0 ~ 4095
	ulUnitOffset += 1;
//	ulUnitNum = constrain(ulUnitNum, 1, xLayerTable[ulLayerNum].ulLayerBufferUnitNum - ulUnitOffset);// 1 ~ (4096 - Offset)

	uint32_t ulOffsetUnit = 0;	// 单元偏移，单位：单元大小
	uint32_t ulPeriodQuotient;	// 周期商
	uint32_t ulPeriodRemainder;	// 周期余数，等于目标单元在在单周期层缓冲区的偏移单元个数
	uint32_t ulPeriodPowerOf2;	// 不大于周期商的最大2幂

	ulPeriodQuotient  = xLayerTable[ulLayerNum].ulTileBufferPeriod *
			ulUnitOffset / xLayerTable[ulLayerNum].ulTileBufferUnitNum;
	ulPeriodRemainder = xLayerTable[ulLayerNum].ulTileBufferPeriod *
			ulUnitOffset % xLayerTable[ulLayerNum].ulTileBufferUnitNum;
	ulPeriodPowerOf2 = ulCalculateMaxPowerOf2LessThan(ulPeriodQuotient);

	/* 计算2的幂周期单元偏移 */
	for(uint8_t i = 0; i < ulLayerNumMax; i++) {
		if(xLayerTable[i].ulTileBufferPeriod <= ulPeriodPowerOf2) {
			ulOffsetUnit += ulPeriodQuotient / xLayerTable[i].ulTileBufferPeriod * xLayerTable[i].ulTileBufferSize;
		}
	}
	/* 周期商刚好等于2幂周期时，单元偏移需要减去小于目标层号多加进来的其他层的单元大小 */
	if((ulPeriodQuotient != 0) && ((ulPeriodQuotient + ulPeriodRemainder) == ulPeriodPowerOf2)) {
		for(uint8_t i = 0; i < ulLayerNum; i++) {
			if(ulPeriodQuotient % xLayerTable[i].ulTileBufferPeriod == 0 ) {
				ulOffsetUnit -= xLayerTable[i].ulTileBufferSize;
			}
		}
	}
	/* 由周期余数计算最后一个周期的层缓冲区内的单元偏移 */
	if(ulPeriodRemainder != 0) {
		for(uint8_t i = ulLayerNumMax - 1; i >= 0; i--) { // 从缓冲区最大的层迭代到最小的
			if(i <= ulLayerNum) {
				break;
			}
			if((ulPeriodQuotient + ulPeriodRemainder) % xLayerTable[i].ulTileBufferPeriod == 0 ) {
				ulOffsetUnit += xLayerTable[i].ulTileBufferSize;
			}
		}
	}
	ulOffsetUnit += ulPeriodRemainder * ulIOSizeMin;
	ulOffsetUnit /= ulIOSizeMin;
	ulOffsetUnit -= 1;

	ReadLayerBufferParam_t xParam =
	{
			.ulAddr = ulOffsetUnit * ulIOSizeMin,
			.ulSize = ulUnitNum * ulIOSizeMin,	// TODO: 单元不连续的多次读处理
			.pucData = pucReadLayerBuffer,
			.ulOffsetUnit = ulOffsetUnit
	};
//	pucReadLayerBuffer = (uint8_t*)aligned_malloc(xReadLayerBufferParam.ulSize, alignment_);
//	printf("ulOffsetUnit = %ld\r\n", ulOffsetUnit);
//	printf("[read param] ulAddr = %10ld, ulSize = %6ld, ulOffsetUnit = %6ld\r\n",
//			xParam.ulAddr,
//			xParam.ulSize,
//			xParam.ulOffsetUnit);
	return  xParam;
}

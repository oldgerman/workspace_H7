/**
  ******************************************************************************
  * @file        tile_wave.h
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
  * This program is distributed in the hope that it will beuleful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see https://www.gnu.org/licenses/.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef TILE_WAVE_H_
#define TILE_WAVE_H_

#include "common_inc.h"
#include <list>
#include <algorithm>
#include <functional>
#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

#ifdef __cplusplus

/* 瓦片波形类*/
class TileWave
{
public:
	/* 配置结构体 */
	typedef struct {
		/* IO Size */
		uint32_t ulIOSize;
		uint32_t ulIOSizeMin;	// 2KB
	    uint32_t ulIOSizeMax;	// 64KB
	    /* Layer */
	    uint32_t ulLayerNum;
	    uint32_t ulLayerNumMax;
	    uint32_t ulLayerTilesNumMax;
	    /* WaveForm */
	    uint32_t ulWaveFrameSize;
	    uint32_t ulWaveDispWidth;
	    uint32_t ulWaveDispTileBufferSize;
	}Config_t;

	/* 层结构体 */
	typedef struct {
		uint32_t ulLayerNum;			// 层编号
		uint32_t ulTileSize;			// 瓦片大小，单位B
		uint32_t ulTileBufferSize;		// RAM：瓦片缓冲区大小，单位B
		uint32_t ulTileBufferOffset;	// 向瓦片缓冲区写入地址的偏移，每次写入瓦片数据，都向后偏移一个瓦片大小
		void *pucTileBuffer;			// 瓦片缓冲区地址
		uint32_t ulBufferSize;			// ROM：缓冲区大小，单位B
		uint32_t ulTileBufferTxPeriod;	// 缓冲区发送周期，单位，调度周期的倍数
	}Layer_t;

	TileWave(Config_t &xConfig);

	uint32_t 	createTileBufferList();
	uint32_t 	sliceTileBuffer(uint8_t* pulData);
	void 		vPrintLayerInfo();

	void initReadWriteAPI(
		    std::function<uint32_t (uint32_t addr, uint32_t size, uint8_t* pData)> 	Write,
		    std::function<uint32_t (uint32_t addr, uint32_t size, uint8_t* pData)>	Read);

	void initMemoryHeapAPI(
			std::function<void* (size_t size, size_t alignment)>	Aligned_malloc,
			std::function<void  (void* ptr_aligned)>				Aligend_free,
			std::function<void  (void* ptr, size_t alignment)> 		Aligned_detect);

	/** @brief  一次可以读取或写入的最小数据块，单位B
	  * @notice 不要与储存介质的最小 IO/SIZE 混淆
	  *         此参数应根据诸多参数综合设置
	  *         比如：
				| 波形帧大小 | 波形区水平像素数 | 波形区覆盖的数据大小 | 存储器     | 存储器最小 IO/SIZE | 实际操作储存器的 IO/SZIE                            |ulMinIOSize  |
				| ---------- | ---------------- | -------------------- | ---------- | ------------------ | --------------------------------------------------- | ------------ |
				| 32bit      | 400              | 1600B                | NAND FLASH | 512B               | 4KB（2K > 1600B 就够，但4K比较快，且能减少读写次数）| 4KB          |
				| 16bit      | 400              | 800B                 | SDRAM      | 1B                 | 1KB（1K > 800B 就够）                               | 1KB          |
				| 8bit       | 400              | 400B                 | PSRAM      | 1B                 | 512B（512B > 400B 就够）                            | 512B         |
	  */
	/* IO SZIE， 必须为 2 的幂 */
	uint32_t ulIOSize;							// 当前读写的IO SZIE
	uint32_t ulIOSizeMin;						// = 2048;  /* 单位 B */
	uint32_t ulIOSizeMax;						// = 16384; /* 单位 B */

	/* 层参数 */
    uint32_t ulLayerNum;						// 当前层编号
	uint32_t ulLayerNumMax;						// 最大层数(总层数)

	/* 层缓冲区在FLASH */
    uint32_t ulLayerBufferSizeMax;				// 层缓冲区大小的最大值，单位B
    uint32_t ulLayersBufferSize;    			// 层缓冲区的总大小

    /* 瓦片缓冲区在RAM */
    uint32_t ulLayerTileBufferSizeMax;			// 层瓦片缓冲区大小的最大值，单位B
    uint32_t ulLayersTileBufferSize;    		// 层瓦片缓冲区的总大小
    uint32_t ulLayerTilesNumMax;				// 每层瓦片的最大个数，该值决定能记录多少时间，必须为 2的幂，本工程为 64MB/32K=2048

    /* 波形参数 */
	uint32_t ulWaveDispBufferSize; 				// 缓冲区大小：波形显示
    uint32_t ulWaveFrameSize;  					// 波形帧大小，单位B
    uint32_t ulWaveDispWidth;					// 波形显示宽度，单位Pixel
    uint32_t ulWaveDispDataSize; 				// 波形显示区数据的大小，单位B
    uint32_t ulWaveDispTileBufferSize;  		// 波形显示区的瓦片缓冲区大小，单位B
    uint32_t ulWaveDispTileBufferSizeMin;		// 波形显示区的瓦片缓冲区最小大小，单位B

    /* 层链表及其迭代器 */
	std::list<Layer_t> xLayersList; 			// 层链表(双向)
	std::list<Layer_t>::iterator xIt;			// 层链表的正向迭代器
	std::list<Layer_t>::reverse_iterator xRit; 	// 层链表的反向迭代器

	/* 层缓冲区的读写缓冲区 */
	void *ucpTxBuffer;					// 给64KB
	void *ucpRxBuffer; 					// 暂时随便给5个2KB

	/* 层缓冲区读写API */
    std::function<uint32_t (uint32_t addr, uint32_t size, uint8_t* pData)> 	write;
    std::function<uint32_t (uint32_t addr, uint32_t size, uint8_t* pData)>	read;

    /* 以下变量在停止切片后在下次切片前需要归 0 */
	uint32_t ulPeriod;					// 周期计数器
	uint32_t ulTxBufferOffsetOld;		// 前一次写入层缓存的偏移地址
	double	fRealWrittenFreqSum; 		// 写入频率的总和
	double 	fRealWrittenFreqAvg;		// 写入频率的平均值
	double 	fRealWrittenFreqNum;		// 写入频率的个数

	/* 以下是在协议解析程序中可更改的标志 */
	uint32_t ulPrintSliceDetail;			// 打印实时切片信息
	uint32_t ulSliceButNotWrite;			// 实时切片时不写入数据

private:
	/**
	  * 字节对齐的动态内存 API
	  * 由于实时采样数据数据需要频繁以2次幂进行缩小等计算，M7 内核的 Cahce 可以缓存
	  * 一部分正在计算的数据，访问粒度是 4 字节，那么使用32字节对齐的动态内存能显著减少访问次数
	  */
    std::function<void* (size_t size, size_t alignment)>	aligned_malloc;
    std::function<void  (void* ptr_aligned)>				aligend_free;
    std::function<void  (void* ptr, size_t alignment)> 		aligned_detect;

	/* 暂存初始化层链表时输出的信息 */
	char ucStrBuffer[20][64];

    static uint32_t ulCalculateSmallestPowerOf2GreaterThan(uint32_t ulValue);
};

}
#endif

#endif /* TILE_WAVE_H_ */

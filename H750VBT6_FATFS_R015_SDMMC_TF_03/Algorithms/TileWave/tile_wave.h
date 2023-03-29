/**
  ******************************************************************************
  * @file        tile_wave.h
  * @author      OldGerman
  * @created on  Mar 20, 2023
  * @brief       
  *    2023-03-23  - 实现从动态内存创建层链表
  *                - 实现瓦片波形切片算法并写存储器
  *    2023-03-26  - 将 .h 中的函数定义整理到 .cpp，添加详细注释
  *                - 修复 writeTileBuffer() 中计算平均频率的 BUG
  *                  将 writeTileBuffer() 重命名为 sliceTileBuffer()
  *                - 移除几乎所有的静态成员，使每个 TileWave 对象的资源完全独立
  *                - 添加函数包装器 aligned_malloc、aligned_free、aligned_detect
  *                  此后使用的所有动态内存都是 32 字节对齐的，方便 Cache 操作
  *                - 实现从动态内存申请字符串缓冲区（使用二级指针申请二维数组）
  *    2023-03-28  - 新增消息队列，基于CMSIS-RTOS2，存放事件消息结构体 Event_t。
  *                  通过消息队列，将实时瓦片切片和读写瓦片数据分离为两个任务，
  *                  前者优先级较高&存消息，后者优先级较低&取消息。
  *                  增加消息队列深度可有效解决读写瓦片任务周期的不确定性，
  *                  例如 FATFS + SD 卡。
  *                  示例工程和测试：https://github.com/oldgerman/workspace_H7/
  *                  tree/master/H750VBT6_FATFS_R015_SDMMC_TF_03
  *                - 写层缓冲区是从内存池申请释放的多个缓冲区，实时瓦片切片任务
  *                  申请该缓冲区， 读写瓦片数据任务释放该缓冲区。
  *
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
		uint32_t ulIOSizeMin;
		uint32_t ulIOSizeMax;
	    /* Layer */
		uint32_t ulLayerNum;
		uint32_t ulLayerNumMax;
		uint32_t ulLayerTilesNumMax;
		/* WaveForm */
		uint32_t ulWaveFrameSize;
		uint32_t ulWaveDispWidth;
		uint32_t ulWaveDispTileBufferSize;
		/* Event */
		uint32_t ulEventNum;
	} Config_t;

	/* 层结构体 */
	typedef struct {
		uint32_t ulLayerNum;				// 层编号
		uint32_t ulTileSize;				// 瓦片大小，单位B
		uint32_t ulTileBufferSize;			// RAM：瓦片缓冲区大小，单位B
		uint32_t ulTileBufferOffset;		// 向瓦片缓冲区写地址的偏移，每次写瓦片数据，都向后偏移一个瓦片大小
		uint8_t* pucTileBuffer;				// 瓦片缓冲区地址
		uint32_t ulBufferSize;				// ROM：缓冲区大小，单位B
		uint32_t ulTileBufferWritePeriod;	// 缓冲区发送周期，单位，调度周期的倍数
	} Layer_t;

	/* 写缓冲区时的参数配置 */
	typedef struct {
		uint32_t ulAddr;
		uint32_t ulSize;
		uint8_t* pucData;
		uint32_t ulPeriod;
		uint32_t ulMark;
	} WriteLayerBufferParam_t;

	/* 读缓冲区时的参数配置 */
	typedef struct {
		uint32_t ulAddr;
		uint32_t ulSize;
		uint8_t* pucData;
	} ReadBufferParam_t;

	/* @brief event types used in the ring buffer */
	typedef enum {
		EVENT_WRITE_LAYER_BUFFER = 0,
		EVENT_READ_BUFFER,
		EVENT_STOP
	} EventType_t;

	/* @brief Event structure used in event queue */
	typedef struct {
		EventType_t type; /* event type */
	    union {
	    	WriteLayerBufferParam_t xWriteLayerBufferParam;
	    	ReadBufferParam_t 	   xReadBufferParam;
	    };
	} Event_t;

	/* Constructor */
	TileWave(Config_t &xConfig);

	uint32_t 			createTileBufferList();
	void 				resetVariablesBeforeSlice();
	WriteLayerBufferParam_t 	sliceTileBuffer(uint8_t* pulData);
	void 				vPrintLayerInfo();

	void initReadWriteAPI(
		    std::function<uint32_t (uint32_t addr, uint32_t size, uint8_t* pData)> 	Write,
		    std::function<uint32_t (uint32_t addr, uint32_t size, uint8_t* pData)>	Read);

	void initMemoryHeapAPI(
			std::function<void* (size_t size, size_t alignment)>	Aligned_malloc,
			std::function<void  (void* ptr_aligned)>				Aligned_free,
			std::function<void  (void* ptr, size_t alignment)> 		Aligned_detect);

	/** @brief  一次可以读取或写的最小数据块，单位B
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

	/* 读写缓冲区 */
	uint32_t ulWriteLayerBufferNum;				// 写层缓冲区的个数
	uint8_t* pucWriteLayerBuffer;				// 写层缓冲区的指针
	uint8_t* pucReadLayerBuffer; 				// 读写缓冲区的指针

	/* 读写API */
	std::function<uint32_t (uint32_t addr, uint32_t size, uint8_t* pData)> 	write;
	std::function<uint32_t (uint32_t addr, uint32_t size, uint8_t* pData)>	read;

	/* 以下变量在停止切片后在下次切片前需要归 0 */
	uint32_t ulPeriod;					// 周期计数器
	uint32_t ulPeriodMax;				// 周期计数器的最大值，到此值后从 0 重新开始计数
	uint32_t ulWriteBufferOffsetOld;	// 前一次写层缓存的偏移地址
	double	fRealWrittenFreqSum; 		// 写频率的总和
	double 	fRealWrittenFreqAvg;		// 写频率的平均值
	double 	fRealWrittenFreqNum;		// 写频率的个数

	/* 以下是在协议解析程序中可更改的标志 */
	uint32_t ulPrintSliceDetail;		// 打印实时切片信息
	uint32_t ulSliceButNotWrite;		// 实时切片时不写数据，若为真，那么就不会在切片时申请层缓冲区

	uint32_t ulEventNum;				// 事件个数，决定消息队列深度
	osMessageQueueId_t xMsgQueue;		// 消息队列

	/**
	  * 字节对齐的动态内存 API
	  * 由于实时采样数据数据需要频繁以2次幂进行缩小等计算，M7 内核的 Cahce 可以缓存
	  * 一部分正在计算的数据，访问粒度是 4 字节，那么使用32字节对齐的动态内存能显著减少访问次数
	  */
	std::function<void* (size_t size, size_t alignment)>	aligned_malloc;
	std::function<void  (void* ptr_aligned)>				aligned_free;
	std::function<void  (void* ptr, size_t alignment)> 		aligned_detect;

private:
	static uint32_t ulCalculateSmallestPowerOf2GreaterThan(uint32_t ulValue);

	static const size_t alignment_ = 32;				// 动态内存 32 字节对齐

	char** ppucStrBuffer_;								// 字符串缓冲区暂存创建层链表时输出的信息
	static const uint32_t ulStrBufferRowCount = 64;		// 字符串缓冲区每行 64 个 char 字符
};

}
#endif

#endif /* TILE_WAVE_H_ */

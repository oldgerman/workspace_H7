/**
  ******************************************************************************
  * @file        tile_slicer.h
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef TILE_SLICER_H_
#define TILE_SLICER_H_

#include "common_inc.h"

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

/**
 * 将 IDMA 需要访问的 4K FIFO 作为每个类的实例成员添加RAM_D1修饰，这样声明该对象就不会编译到 DTCM
 * 缓冲区不使用动态内存，缓冲区的最大大小在编译时已写死，运行时只可通过变量限制最大可用的缓冲区，但缓冲区的总大小是不变的
 * 如果使用动态内存，RTOS下就涉及到malloc的线程安全问题，得参照 heap_useNewlib_ST.c 实现 malloc 锁，比较麻烦
 * 如果FIFO中接收/发送的数据量超过/低于这个阈值时，FIFO就会触发相应的接收/发送
 * total buf[........................................................................................]
 * 			 ^		  ^        ^
 *      list.FIFO[0]  FIFO[1]  FIFO[2]  ...
 * FIFO 阈值是 4K，但FIFO大小不能刚好是4K，需要留8K，开双缓冲区
 *
 * PyrLayer 只关心每次被调用时处理数据的变化量，不关心带有时间单位的变量，比如采样率。
 *          数据的变化量受采样率变化影响， "率" 是数据变化量与时间变化量的比值，所以
 *          采样率计算结果带有时间单位的变量
 *
 * 4K FIFO也是可变的
 * NAND Flash  擦除以block为最小操作单位，读/写以page为最小操作单位。出厂默认浮栅不带电荷，为1状态。擦除就是全写1
 */
/* pyramid layer
 * 表示金字塔所有层的类*/
class PyrLayers
{
public:
	PyrLayers();
	/** @brief  一次可以读取或写入的最小数据块，单位B
	  * @notice 不要与储存介质的最小 IO/SIZE 混淆
	  *         此参数应根据诸多参数综合设置
	  *         比如：
				| 波形帧大小 | 波形区水平像素数 | 波形区覆盖的数据大小 | 存储器     | 存储器最小 IO/SIZE | 实际操作储存器的 IO/SZIE                            | usMinIOSize  |
				| ---------- | ---------------- | -------------------- | ---------- | ------------------ | --------------------------------------------------- | ------------ |
				| 32bit      | 400              | 1600B                | NAND FLASH | 512B               | 4KB（2K > 1600B 就够，但4K比较快，且能减少读写次数）| 4KB          |
				| 16bit      | 400              | 800B                 | SDRAM      | 1B                 | 1KB（1K > 800B 就够）                               | 1KB          |
				| 8bit       | 400              | 400B                 | PSRAM      | 1B                 | 512B（512B > 400B 就够）                            | 512B         |
	  */
	uint16_t usIOSize;
	static uint16_t usIOSize_Min; // = 4096;  /* 单位 B */
    static uint32_t ulIOSize_Max; // = 32768; /* 单位 B */

    uint16_t usLayer;             // 层编号
    static uint8_t  ucLayers_Max; // 最大层数
    static uint8_t  ucLayers_All; // 总层数

	uint8_t ucTxBuf[];
	uint8_t ucRxBuf[];

};

}
#endif

#endif /* TILE_SLICER_H_ */

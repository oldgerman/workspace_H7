/**
  ******************************************************************************
  * @file        wave_chart.h
  * @author      OldGerman
  * @created on  Apr 5, 2023
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
#ifndef WAVE_CHART_H_
#define WAVE_CHART_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
/* Exported types ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

#ifdef __cplusplus
}

class WaveChart {
public:
	/* 配置结构体 */
	typedef struct {
		/* WaveForm */
		uint32_t ulWaveSamplePointSize;
		uint32_t ulWaveDispWidth;
		uint32_t ulWaveDispTileBufferSize;
		/* Event */
		uint32_t ulEventNum;
	} Config_t;

	/* 波形参数 */
	uint32_t ulWaveSamplePointSize; 			// 采样点大小，单位 B，必须为 2的幂，例如 8B、16B、32B、64B
	uint32_t ulWaveDispWidth;					// 波形区宽度，单位Px，每像素 1个采样点

	uint32_t ulWaveDispBufferSize; 				// 缓冲区大小：波形显示
	uint32_t ulWaveDispDataSize; 				// 波形显示区数据的大小，单位B
	uint32_t ulWaveDispTileBufferSize;  		// 波形显示区的瓦片缓冲区大小，单位B
	uint32_t ulWaveDispTileBufferSizeMin;		// 波形显示区的瓦片缓冲区最小大小，单位B

	WaveChart(Config_t xConfig) {
		/* WaveForm */
	    ulWaveSamplePointSize = xConfig.ulWaveSamplePointSize;
	    ulWaveDispWidth = xConfig.ulWaveDispWidth;
		ulWaveDispTileBufferSize = xConfig.ulWaveDispTileBufferSize;

		ulWaveDispDataSize = ulWaveDispWidth * ulWaveSamplePointSize;
		/* 确定波形显示区的瓦片缓冲区大小有2种情况
		 * 存储所有波形的介质IOsize在 ulWaveDispTileBufferSizeMin ~ ulIOSizeMax 范围
		 * 1. RAM：IOsize越大，访问速度差不多
		 * 2. ROM：IOsize越大，访问速度越快
		 */
		/* 大于波形显示区数据大小的最小2的幂 */
//		ulWaveDispTileBufferSizeMin = ulCalculateMinPowerOf2GreaterThan(ulWaveDispDataSize);

		/* ulWaveDispTileBufferSize ≥ ulWaveDispTileBufferSizeMin 且
		 * ulWaveDispTileBufferSize ≥ ulIOSizeMin
		 */
		if(ulWaveDispTileBufferSize < ulWaveDispTileBufferSizeMin) {
			ulWaveDispTileBufferSize = ulWaveDispTileBufferSizeMin;
		}
	}

	/**
	  *@brief  快速缩放，仅支持2幂缩放因子
	  *@attention 直接取切片数据显示
	  */
	void xZoomFast() {

	}

	/**
	  *@brief  平滑缩放，支持小数缩放因子
	  *@attention 取切片数据后需要进行运算
	  */
	void xZoomSmooth() {

	}

private:
};
#endif

#endif /* WAVE_CHART_H_ */

/**
  ******************************************************************************
  * @file        lmsfilter.h
  * @author      OldGerman
  * @created on  2023年12月22日
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
#ifndef FILTER_LMSFILTER_H_
#define FILTER_LMSFILTER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "arm_math.h"
#include "arm_const_structs.h"

/* Exported types ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#if 0
class LMSFilter{
public:
    typedef struct {
        uint32_t TEST_LENGTH_SAMPLES;        // 采样点数
        uint32_t NUM_TAPS;                   // 滤波器系数个数

        uint32_t blockSize;                  // 调用一次arm_lms_norm_f32处理的采样点个数
        uint32_t numBlocks;                  // 需要调用arm_lms_norm_f32的次数

        float32_t *testInput_f32_50Hz_200Hz; // 源波形
        float32_t *testInput_f32_REF;        // 参考波形
        float32_t *test_f32_ERR;             // 误差数据
        float32_t *testOutput;               // 滤波后的输出
        float32_t *lmsStateF32;              // 状态缓存，大小numTaps + blockSize - 1
        float32_t *lmsCoeffs32;              // 滤波器系数
    }attr_t;

    attr_t attr;

    LMSFilter(attr_t attr = {
        .TEST_LENGTH_SAMPLES = 2048,
        .blockSize = 2048,
        .NUM_TAPS = 20,

    }){
        this->attr = attr;
    }

    void init();
    void update();
private:

    /**
      * 字节对齐的动态内存 API
      * 实时DSP的数据可以被 M7 的 Cache 缓存
      * 一部分正在计算的数据，访问粒度是 4 字节，那么使用32字节对齐的动态内存能显著减少访问次数
      */
    std::function<void* (size_t size, size_t alignment)>    aligned_malloc;
    std::function<void  (void* ptr_aligned)>                aligned_free;
    std::function<void  (void* ptr, size_t alignment)>      aligned_detect;

    static const size_t alignment_ = 32;                // 动态内存 32 字节对齐
};
#endif

#endif /* FILTER_LMSFILTER_H_ */

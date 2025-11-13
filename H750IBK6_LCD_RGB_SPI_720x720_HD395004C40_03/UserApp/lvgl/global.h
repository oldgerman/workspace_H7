/**
  ******************************************************************************
  * @file        global.h
  * @author      OldGerman
  * @created on  2024年01月04日
  * @brief       
  ******************************************************************************
  * @attention
  *
  * Copyright (C) 2024 OldGerman.
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
#ifndef __GLOBAL_H
#define __GLOBAL_H

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
#endif

/* Includes ------------------------------------------------------------------*/
#include <vector>
#include <string>
/* Exported types ------------------------------------------------------------*/
typedef std::vector<std::string>    String;
typedef std::vector<int8_t>         Int8Array;
typedef std::vector<uint8_t>        Uint8Array;
typedef std::vector<int16_t>        Int16Array;
typedef std::vector<uint16_t>       Uint16Array;
typedef std::vector<int32_t>        Int32Array;
typedef std::vector<uint32_t>       Uint32Array;
typedef std::vector<float>          Float32Array;
typedef std::vector<double>         Float64Array;
typedef std::vector<int64_t>        Int64Array;
typedef std::vector<uint64_t>       Uint64Array;

typedef uint32_t rate;
typedef uint32_t microseconds;
typedef Float32Array sampleArray;
typedef uint32_t sampleIndex;
typedef uint32_t sampleTimestamp;

/* Exported define -----------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
const uint32_t bufferLengthInSeconds = 60 * 5;
const uint32_t numberOfDigitalChannels = 8;

const uint32_t initialSamplingTime = 10;                             // 初始采样时间10s
//const uint32_t initialSamplesPerSecond = 1e6 / initialSamplingTime;  // 初始采样率 是 1000000 / 10s = 100K
const uint32_t initialSamplesPerSecond = 1e1 / initialSamplingTime;  // 初始采样率 是 1000000 / 10s = 100K

/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/**
 * @brief 公共选项 GlobalOptions
 */
typedef struct
{
// public:
    /** Time between each sample denoted in microseconds, which is equal to 1e-6 seconds \
     *  e.g. if samplesPerSecond is 100_000, then a sample is taken every 10th microsecond
     * 每个样本之间的时间以微秒表示，等于 1e-6 秒 \
     * 例如如果 samplesPerSecond 为 100_000，则每10微秒采样一次
     */
    microseconds samplingTime;
    /** 每秒样本数    The number of samples per second */
    rate samplesPerSecond;
    /** @var data: 包含以 uA（微安）表示的所有电流样本   contains all samples of current denoted in uA (microAmpere). */
    sampleArray *data;
    /** @var [bits]: 包含每个样本的位状态，变量可能为空    contains the bit state for each sample, variable may be null */
    Uint16Array *bits;
    /** @var index: 指向数据数组中最后一个样本的索引的指针     pointer to the index of the last sample in data array */
    sampleIndex index;
    /** 最新采集样本的时间戳，每个样本按 {samplingTime} 递增   Timestamp for the latest sample taken, incremented by {samplingTime} for each sample */
    sampleTimestamp timestamp;
}GlobalOptions;

//const GlobalOptions options = {
//    .samplingTime = initialSamplingTime,
//    .samplesPerSecond =  initialSamplesPerSecond,  // 每秒采样率
//    .data = new Float32Array(initialSamplesPerSecond * bufferLengthInSeconds, 0), // stm32禁止对全局变量用new！ // 默认100K采集10秒，初始化一个float32数组，元素个数为 100000 * 10 = 1Mpt
//    .bits =  nullptr,
//    .index =  0,
//    .timestamp = 0,
//};

// 内联函数可以在头文件实现
inline void test_new_delete()
{
    // 局部 options 的 vector 使用 new 测试
    const GlobalOptions options = {
        .samplingTime = initialSamplingTime,
        .samplesPerSecond =  initialSamplesPerSecond,  // 每秒采样率
        .data = new Float32Array(initialSamplesPerSecond * bufferLengthInSeconds, 0), // 默认100K采集10秒，初始化一个float32数组，元素个数为 100000 * 10 = 1Mpt
        .bits =  nullptr,
        .index =  0,
        .timestamp = 0,
    };

    printf("options.data 地址: %10p\r\n", (void*)options.data);
    /** 不释放内存：
        options.data 地址: 0x200054a8
        options.data 地址: 0x20005cd8
        options.data 地址: 0x20005cf0
        options.data 地址: 0x20005d00
        options.data 地址: 0x20005d18
        ...
        options.data 地址: 0x2001efb0
        options.data 地址: 0x2001f478
     *  内存溢出死机了
     */
    delete options.data; // 释放内存
    /** 每次 vector 申请的内存地址都一样
        options.data 地址: 0x200054a8
        options.data 地址: 0x200054a8
        options.data 地址: 0x200054a8
        ...
     */
}

#endif /* __GLOBAL_H */

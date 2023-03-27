/**
  ******************************************************************************
  * @file        rtx_memory.h
  * @modify      OldGerman
  * @created on  Mar 20, 2023
  * @brief
  *    2023-03-21  打包到 osRtxMemory 类
  *    2023-03-22  修复在构造函数中初始化内存池进hardfault的问题
  *    2023-03-26  添加 aligned_malloc、aligned_free、aligned_detect 函数
  *                添加 test_memory、test_aligned_memory 函数，测试结果支持从
  *                构造函数传入的自定义 printf 输出
  *
  ******************************************************************************
  * 模块名称 : 动态内存管理
  * 文件名称 : rtx_memory.h
  * 版    本 : V1.0
  * 说    明 : 将RTX5的动态内存管理整理出来, 可以管理多个内存块
  *
  * 修改记录 :
  * 	版本号   日期         作者        说明
  * 	V1.0    2018-04-09   Eric2013   将RTX5的动态内存管理整理出来
  *
  * Copyright (C), 2018-2030, 安富莱电子 www.armfly.com
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2013-2018 Arm Limited. All rights reserved.
  *
  * SPDX-License-Identifier: Apache-2.0
  *
  * Licensed under the Apache License, Version 2.0 (the License); you may
  * not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  * www.apache.org/licenses/LICENSE-2.0
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an AS IS BASIS, WITHOUT
  * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  * ----------------------------------------------------------------------------
  *
  * Project:     CMSIS-RTOS RTX
  * Title:       RTX Library definitions
  *
  * ----------------------------------------------------------------------------
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef RTX_MEMORY_H_
#define RTX_MEMORY_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdint.h>
#include "stm32h7xx.h"

/* Exported types ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

#ifdef __cplusplus

class osRtxMemory
{
public:
	typedef int (*fprintf_t) (const char * format, ...);

	/* Constructor */
	osRtxMemory(void *Mem, uint32_t SizePool, fprintf_t Printf = nullptr);

	/* Init memory pool */
	uint32_t init();

	/* Memory Heap Library functions */
	void*		malloc(size_t size, uint32_t type = 0);
	uint32_t	free(void *block);
	void*		aligned_malloc(size_t size, size_t alignment);
	uint32_t 	aligned_free(void* ptr_aligned);
	uint32_t 	aligned_detect(void* ptr, size_t alignment);

	/* Memory Heap Library test functions */
	void test_memory(uint32_t times, uint32_t start_size);
	void test_aligned_memory(uint32_t times, uint32_t start_size, uint32_t alignment);

	/* Get memory info */
	uint32_t getMemUsed() {
		memHead = MemHeadPtr(mem);
		return memHead->used;
	}
	uint32_t getMemSize() {
		memHead = MemHeadPtr(mem);
		return memHead->size;
	}
	uint32_t getMemFree() {
		memHead = MemHeadPtr(mem);
		return memHead->size - memHead->used;
	}
	uint32_t getMemFreeMin() {
		if(getMemFree() < sizeFreeMin) {
			sizeFreeMin = getMemFree();
		}
		return sizeFreeMin;
	}

private:
	/* funtion status */
	typedef enum {
		FUN_OK = 0,
		FUN_ERROR,
	}fun_status_t;

	/* Memory Block Info: Length = <31:2>:'00', Type = <1:0> */
	static const uint32_t MB_INFO_LEN_MASK  = 0xFFFFFFFCU;	// Length mask
	static const uint32_t MB_INFO_TYPE_MASK = 0x00000003U;	// Type mask

	/* Memory Pool Header structure */
	typedef struct {
		uint32_t size;				// Memory Pool size
		uint32_t used;				// Used Memory
	} mem_head_t;

	/* Memory Block Header structure */
	typedef struct mem_block_s {
		struct mem_block_s *next;		// Next Memory Block in list
		uint32_t            info;		// Block Info or max used Memory (in last block)
		// 块信息或最大使用内存（在最后一个块中）
	} mem_block_t;

	/* Memory Head Pointer */
	__STATIC_INLINE mem_head_t *MemHeadPtr (void *mem) {
		//lint -e{9079} -e{9087} "conversion from pointer to void to pointer to other type" [MISRA Note 6]
		return ((mem_head_t *)mem);
	}

	/* Memory Block Pointer */
	__STATIC_INLINE mem_block_t *MemBlockPtr (void *mem, uint32_t offset) {
		uint32_t     addr;
		mem_block_t *ptr;

		//lint --e{923} --e{9078} "cast between pointer and unsigned int" [MISRA Note 8]
		addr = (uint32_t)mem + offset;
		ptr  = (mem_block_t *)addr;

		return ptr;
	}

	void       *mem;			// Pointer to memory pool.
	uint32_t    sizePool;		// Size of a memory pool in bytes.
	fprintf_t   printf;			// User-defined printf
	mem_head_t *memHead;		// Memory head structer
	uint32_t    sizeFreeMin;	// Historical minimum free memory size
};

}
#endif

#endif /* RTX_MEMORY_H_ */

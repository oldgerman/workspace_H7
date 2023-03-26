/**
  ******************************************************************************
  * @file        rtx_memory.cpp
  * @modify      OldGerman
  * @created on  Mar 20, 2023
  * @brief       See rtx_memory.h for details
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

/* Includes ------------------------------------------------------------------*/
#include "rtx_memory.h"

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
 * @brief  The constructor of the memory pool object
 * @param  Mem		pointer to memory pool.
 * @param  SizePool	size of a memory pool in bytes.
 * @param  Printf	function pointer
 * @retval N/A
 */
osRtxMemory::osRtxMemory(void *Mem, uint32_t SizePool, fprintf_t Printf)
	:mem(Mem), sizePool(SizePool), printf(Printf), sizeFreeMin(sizePool)
{}

/**
 * @brief  Initialize Memory Pool with variable block size.
 * @param  None
 * @retval 0 - success, 1 - failure.
 */
uint32_t osRtxMemory::init()
{
	uint32_t size = sizePool;
	mem_head_t  *head;
	mem_block_t *ptr;

	// Check parameters
	//lint -e{923} "cast from pointer to unsigned int" [MISRA Note 7]
	if ((mem == NULL) || (((uint32_t)mem & 7U) != 0U) || ((size & 7U) != 0U) ||
			(size < (sizeof(mem_head_t) + (2U*sizeof(mem_block_t))))) {
		//EvrRtxMemoryInit(mem, size, 0U);
		//lint -e{904} "Return statement before end of function" [MISRA Note 1]
		return FUN_ERROR;
	}

	// Initialize memory pool header
	head = MemHeadPtr(mem);
	head->size = size;

	/**
	  * 第一个控制块是 16byte（2个控制块大小）
	  * 后续的控制块都是 8byte（1个控制块大小)
	  */
	head->used = sizeof(mem_head_t) + sizeof(mem_block_t);

	// Initialize first and last block header
	ptr = MemBlockPtr(mem, sizeof(mem_head_t));
	ptr->next = MemBlockPtr(mem, size - sizeof(mem_block_t));
	ptr->next->next = NULL;
	ptr->next->info = sizeof(mem_head_t) + sizeof(mem_block_t);
	ptr->info = 0U;

	//EvrRtxMemoryInit(mem, size, 1U);

	return FUN_OK;
}

/**
 * @brief  Allocate a memory block from a Memory Pool.
 * @param  size            size of a memory block in bytes.
 * @param  type            memory block type: 0 - generic, 1 - control block
 * @retval allocated memory block or NULL in case of no memory is available.
 */
void* osRtxMemory::malloc(size_t size, uint32_t type)
{
	mem_block_t *ptr;
	mem_block_t *p, *p_new;
	uint32_t     block_size;
	uint32_t     hole_size;

	// Check parameters
	if ((mem == NULL) || (size == 0U) || ((type & ~MB_INFO_TYPE_MASK) != 0U)) {
		//EvrRtxMemoryAlloc(mem, size, type, NULL);
		//lint -e{904} "Return statement before end of function" [MISRA Note 1]
		return NULL;
	}

	// Add block header to size
	block_size = size + sizeof(mem_block_t);			// 块大小 = 用户申请的大小 + 内存控制块大小
	// Make sure that block is 8-byte aligned
	block_size = (block_size + 7U) & ~((uint32_t)7U); 	// 强制块大小 8 字节对齐
	// 第一个控制块的指针
	p = MemBlockPtr(mem, sizeof(mem_head_t));
	// Search for hole big enough
	for (;;) {
		//lint -e{923} -e{9078} "cast from pointer to unsigned int"
		hole_size  = (uint32_t)p->next - (uint32_t)p;
		hole_size -= p->info & MB_INFO_LEN_MASK;

		/* 找到比 block_size 大的内存片段就退出循环 */
		if (hole_size >= block_size) {
			// Hole found
			break;
		}

		/* 没有找到，继续找下一个 */
		p = p->next;
		if (p->next == NULL) {
			// Failed (end of list)
			//EvrRtxMemoryAlloc(mem, size, type, NULL);
			//lint -e{904} "Return statement before end of function" [MISRA Note 1]
			return NULL;
		}
	}

	// 更新本控制块已使用的内存大小 Update used memory
	(MemHeadPtr(mem))->used += block_size;

	// 更新内存池总共使用的大小 Update max used memory
	p_new = MemBlockPtr(mem, (MemHeadPtr(mem))->size - sizeof(mem_block_t));
	if (p_new->info < (MemHeadPtr(mem))->used) {
		p_new->info = (MemHeadPtr(mem))->used;
	}

	// 分配内存 Allocate block
	if (p->info == 0U) {
		// No block allocated, set info of first element
		// 还没有块被分配，设置 info 为第一个元素
		p->info = block_size | type;
		ptr = MemBlockPtr(p, sizeof(mem_block_t));
	} else {
		// Insert new element into the list
		// 在链表中插入新元素
		p_new = MemBlockPtr(p, p->info & MB_INFO_LEN_MASK);
		p_new->next = p->next;
		p_new->info = block_size | type;
		p->next = p_new;
		ptr = MemBlockPtr(p_new, sizeof(mem_block_t));
	}

	//EvrRtxMemoryAlloc(mem, size, type, ptr);

	return ptr;
}

/**
 * @brief  Allocates a byte-aligned memory from a memory pool.
 * @param  size            size of a memory block in bytes.
 * @param  alignment       byte-aligned size
 * @retval allocated memory block or NULL in case of no memory is available.
 * @reference blog.csdn.net/jin739738709/article/details/122992753
 */
void* osRtxMemory::aligned_malloc(size_t size, size_t alignment)
{
	/* 分配足够的内存, 这里的算法很经典, 早期的STL中使用的就是这个算法 */

	/* 首先维护FreeBlock指针占用的内存大小 */
	const int pointerSize = sizeof(void*);

	/** alignment - 1 + pointerSize这个是FreeBlock内存对齐需要的内存大小
	  * 前面的例子sizeof(T) = 20, __alignof(T) = 16,
	  * g_MaxNumberOfObjectsInPool = 1000
	  * 那么调用本函数就是alignedMalloc(1000 * 20, 16)
	  * 那么alignment - 1 + pointSize = 19
	  */
	const int requestedSize = size + alignment - 1 + pointerSize;

	/* 分配的实际大小就是20000 + 19 = 20019 */
	void* raw = malloc(requestedSize);

	/* 这里实Pool真正为对象实例分配的内存地址 */
	uintptr_t start = (uintptr_t)raw + pointerSize;

	/** 向上舍入操作
	  * 解释一下, __ALIGN - 1指明的是实际内存对齐的粒度
	  * 例如__ALIGN = 8时, 我们只需要7就可以实际表示8个数(0~7)
	  * 那么~(__ALIGN - 1)就是进行舍入的粒度
	  * 我们将(bytes) + __ALIGN-1)就是先进行进位, 然后截断
	  * 这就保证了我是向上舍入的
	  * 例如byte = 100, __ALIGN = 8的情况
	  * ~(__ALIGN - 1) = (1 000)B
	  * ((bytes) + __ALIGN-1) = (1 101 011)B
	  * (((bytes) + __ALIGN-1) & ~(__ALIGN - 1)) = (1 101 000 )B = (104)D
	  * 104 / 8 = 13, 这就实现了向上舍入
	  * 对于byte刚好满足内存对齐的情况下, 结果保持byte大小不变
	  * 记得《Hacker's Delight》上面有相关的计算
	  * 这个表达式与下面给出的等价
	  * ((((bytes) + _ALIGN - 1) * _ALIGN) / _ALIGN)
	  * 但是SGI STL使用的方法效率非常高
	  */
	void* ptr_aligned = (void*)((start + alignment - 1) & ~(alignment - 1));

	/* 这里维护一个指向malloc()真正分配的内存 */
	*(void**)((uintptr_t)ptr_aligned - pointerSize) = raw;

	/* 返回实例对象真正的地址 */
	return ptr_aligned;
}

/**
 * @brief  Return an allocated a byte-aligned memory block back to a Memory Pool.
 * @param  size            size of a memory block in bytes.
 * @param  alignment       byte-aligned size, must be at least 8 bytes
 * @retval allocated memory block or NULL in case of no memory is available.
 * @reference blog.csdn.net/jin739738709/article/details/122992753
 * @attention
 * 这里是内部维护的内存情况
 *                   这里满足内存对齐要求
 *                             |
 * ----------------------------------------------------------------------
 * | 内存对齐填充 | 维护的指针 | 对象1 | 对象2 | 对象3 | ...... | 对象n |
 * ----------------------------------------------------------------------
 * ^                     | 指向malloc()分配的地址起点
 * |                     |
 * -----------------------
 */
uint32_t osRtxMemory::aligned_free(void* ptr_aligned)
{
	return free(((void**)ptr_aligned)[-1]);
}

/**
 * @brief  Detect address alignment
 * @param  ptr            Point to the address that needs to be checked
 * @param  alignment      byte-aligned size
 * @retval 0 - is aligned, 1 - not aligned.
 * @reference blog.csdn.net/jin739738709/article/details/122992753
 */
uint32_t osRtxMemory::aligned_detect(void* ptr, size_t alignment)
{
	/*《Hacker's Delight》中的经典算法 */
	return !(((uintptr_t)ptr & (alignment - 1)) == 0);
}

/**
 * @brief  Return an allocated memory block back to a Memory Pool.
 * @param  block           memory block to be returned to the memory pool.
 * @retval 0 - success, 1 - failure.
 */
uint32_t osRtxMemory::free(void *block)
{
	const mem_block_t *ptr;
	mem_block_t *p, *p_prev;

	// Check parameters
	if ((mem == NULL) || (block == NULL)) {
		//EvrRtxMemoryFree(mem, block, 0U);
		//lint -e{904} "Return statement before end of function" [MISRA Note 1]
		return FUN_ERROR;
	}

	// Memory block header
	ptr = MemBlockPtr(block, 0U);
	ptr--;

	// Search for block header
	p_prev = NULL;
	p = MemBlockPtr(mem, sizeof(mem_head_t));
	while (p != ptr) {
		p_prev = p;
		p = p->next;
		if (p == NULL) {
			// Not found
			//EvrRtxMemoryFree(mem, block, 0U);
			//lint -e{904} "Return statement before end of function" [MISRA Note 1]
			return FUN_ERROR;
		}
	}

	// Update used memory
	(MemHeadPtr(mem))->used -= p->info & MB_INFO_LEN_MASK;

	// Free block
	if (p_prev == NULL) {
		// Release first block, only set info to 0
		p->info = 0U;
	} else {
		// Discard block from chained list
		p_prev->next = p->next;
	}

	//EvrRtxMemoryFree(mem, block, 1U);

	return FUN_OK;
}

/**
  * @brief  测试动态内存API
  * @param  times		申请的次数
  * @param  start_size	起始申请大小，单位B
  * @retval None
  */
void osRtxMemory::test_memory(uint32_t times, uint32_t start_size)
{
	if(printf == nullptr)
		return;

	uint32_t mem_size;
	void*   SRAM1_Addr[times];

	printf("\r\n【测试动态内存分配 %ld 次，起始大小 %ld ，每次大小翻倍，单位 byte 】\r\n",
			times, start_size);
	printf("【从DRAM申请空间】\r\n");
	printf("| 已申请次数 | 内存池大小 |  申请大小  |  申请地址  |   共使用   |    剩余    | 历史最少可用 |\r\n");
	printf("| ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ------------ |\r\n");
	for(uint32_t i = 0; i < times; i++) {
		mem_size = start_size << i;
		SRAM1_Addr[i] = malloc(mem_size);
		printf("| %10ld | %10ld | %10ld | %10p | %10ld | %10ld | %12ld |\r\n",
				i + 1,
				getMemSize(),
				mem_size, SRAM1_Addr[i],
				getMemUsed(),
				getMemFree(),
				getMemFreeMin());
	}

	printf("\r\n【释放从DRAM申请的空间】\r\n");
	printf("|  释放次数  | 内存池大小 |  释放大小  |  释放地址  |   共使用   |    剩余    | 历史最少可用 |\r\n");
	printf("| ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ------------ |\r\n");
	for(uint32_t i = 0; i < times; i++) {
		mem_size = start_size << i;
		free(SRAM1_Addr[i]);
		printf("| %10ld | %10ld | %10ld | %10p | %10ld | %10ld | %12ld |\r\n",
				i + 1,
				getMemSize(),
				mem_size, SRAM1_Addr[i],
				getMemUsed(),
				getMemFree(),
				getMemFreeMin());
	}
}

/**
  * @brief  测试字节对齐动态内存API
  * @param  times		申请的次数
  * @param  start_size	起始申请大小，单位B
  * @param	alignment	地址对齐的大小
  * @retval None
  */
void osRtxMemory::test_aligned_memory(uint32_t times, uint32_t start_size, uint32_t alignment)
{
	if(printf == nullptr)
		return;

	uint32_t mem_size;
	void*   SRAM1_Addr[times];

	printf("\r\n【测试 %ld 字节对齐的动态内存分配 %ld 次，起始大小 %ld ，每次大小翻倍，单位 byte 】\r\n",
			alignment, times, start_size);
	printf("【从DRAM申请空间】\r\n");
	printf("| 已申请次数 | 内存池大小 |  申请大小  |  申请地址  |  字节对齐  |   共使用   |    剩余    | 历史最少可用 |\r\n");
	printf("| ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- |------------ |\r\n");
	for(uint32_t i = 0; i < times; i++) {
		mem_size = start_size << i;
		SRAM1_Addr[i] = aligned_malloc(mem_size, alignment);
		printf("| %10ld | %10ld | %10ld | %10p | %10s | %10ld | %10ld | %12ld |\r\n",
				i + 1,
				getMemSize(),
				mem_size, SRAM1_Addr[i],
				(aligned_detect(SRAM1_Addr[i], alignment) == 0)?("Yes"):("No"),
				getMemUsed(),
				getMemFree(),
				getMemFreeMin());
	}

	printf("\r\n【释放从DRAM申请的空间】\r\n");
	printf("|  释放次数  | 内存池大小 |  释放大小  |  释放地址  |  字节对齐  |   共使用   |    剩余    | 历史最少可用 |\r\n");
	printf("| ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ------------ |\r\n");
	for(uint32_t i = 0; i < times; i++) {
		mem_size = start_size << i;
		aligned_free(SRAM1_Addr[i]);
		printf("| %10ld | %10ld | %10ld | %10p | %10s | %10ld | %10ld | %12ld |\r\n",
				i + 1,
				getMemSize(),
				mem_size, SRAM1_Addr[i],
				(aligned_detect(SRAM1_Addr[i], alignment) == 0)?("Yes"):("No"),
				getMemUsed(),
				getMemFree(),
				getMemFreeMin());
	}
}

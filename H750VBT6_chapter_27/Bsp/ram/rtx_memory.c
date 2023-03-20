/*
*********************************************************************************************************
*
*	模块名称 : 动态内存管理
*	文件名称 : rtx_memory.c
*	版    本 : V1.0
*	说    明 : 将RTX5的动态内存管理整理出来, 可以管理多个内存块
*
*	修改记录 :
*		版本号   日期         作者        说明
*		V1.0    2018-04-09   Eric2013   将RTX5的动态内存管理整理出来
*		        2023-03-20   OldGerman  将return 1为成功0为失败对调，方便将返回值累加为0判断成功
*
*	Copyright (C), 2018-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/	
/*
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
 * -----------------------------------------------------------------------------
 *
 * Project:     CMSIS-RTOS RTX
 * Title:       Memory functions
 *
 * -----------------------------------------------------------------------------
 */
 
#include "rtx_memory.h"

//  ==== Library functions ====

/**
  * @brief  Initialize Memory Pool with variable block size.
  * @param  mem             pointer to memory pool.
  * @param  size            size of a memory pool in bytes.
  * @retval 0 - success, 1 - failure.
  */
__WEAK uint32_t osRtxMemoryInit (void *mem, uint32_t size) {
  mem_head_t  *head;
  mem_block_t *ptr;

  // Check parameters
  //lint -e{923} "cast from pointer to unsigned int" [MISRA Note 7]
  if ((mem == NULL) || (((uint32_t)mem & 7U) != 0U) || ((size & 7U) != 0U) ||
      (size < (sizeof(mem_head_t) + (2U*sizeof(mem_block_t))))) {
    //EvrRtxMemoryInit(mem, size, 0U);
    //lint -e{904} "Return statement before end of function" [MISRA Note 1]
    return 1U;
  }

  // Initialize memory pool header
  head = MemHeadPtr(mem);
  head->size = size;
  head->used = sizeof(mem_head_t) + sizeof(mem_block_t);

  // Initialize first and last block header
  ptr = MemBlockPtr(mem, sizeof(mem_head_t));
  ptr->next = MemBlockPtr(mem, size - sizeof(mem_block_t));
  ptr->next->next = NULL;
  ptr->next->info = sizeof(mem_head_t) + sizeof(mem_block_t);
  ptr->info = 0U;

  //EvrRtxMemoryInit(mem, size, 1U);

  return 0U;
}

/**
  * @brief  Allocate a memory block from a Memory Pool.
  * @param  mem             pointer to memory pool.
  * @param  size            size of a memory block in bytes.
  * @param  type            memory block type: 0 - generic, 1 - control block
  * @retval allocated memory block or NULL in case of no memory is available.
  */
__WEAK void *osRtxMemoryAlloc (void *mem, uint32_t size, uint32_t type) {
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
  block_size = size + sizeof(mem_block_t);
  // Make sure that block is 8-byte aligned
  block_size = (block_size + 7U) & ~((uint32_t)7U);

  // Search for hole big enough
  p = MemBlockPtr(mem, sizeof(mem_head_t));
  for (;;) {
    //lint -e{923} -e{9078} "cast from pointer to unsigned int"
    hole_size  = (uint32_t)p->next - (uint32_t)p;
    hole_size -= p->info & MB_INFO_LEN_MASK;
    if (hole_size >= block_size) {
      // Hole found
      break;
    }
    p = p->next;
    if (p->next == NULL) {
      // Failed (end of list)
      //EvrRtxMemoryAlloc(mem, size, type, NULL);
      //lint -e{904} "Return statement before end of function" [MISRA Note 1]
      return NULL;
    }
  }

  // Update used memory
  (MemHeadPtr(mem))->used += block_size;

  // Update max used memory
  p_new = MemBlockPtr(mem, (MemHeadPtr(mem))->size - sizeof(mem_block_t));
  if (p_new->info < (MemHeadPtr(mem))->used) {
    p_new->info = (MemHeadPtr(mem))->used;
  }

  // Allocate block
  if (p->info == 0U) {
    // No block allocated, set info of first element
    p->info = block_size | type;
    ptr = MemBlockPtr(p, sizeof(mem_block_t));
  } else {
    // Insert new element into the list
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
  * @brief  Return an allocated memory block back to a Memory Pool.
  * @param  mem             pointer to memory pool.
  * @param  block           memory block to be returned to the memory pool.
  * @retval 0 - success, 1 - failure.
  */
__WEAK uint32_t osRtxMemoryFree (void *mem, void *block) {
  const mem_block_t *ptr;
        mem_block_t *p, *p_prev;

  // Check parameters
  if ((mem == NULL) || (block == NULL)) {
    //EvrRtxMemoryFree(mem, block, 0U);
    //lint -e{904} "Return statement before end of function" [MISRA Note 1]
    return 1U;
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
      return 1U;
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

  return 0U;
}

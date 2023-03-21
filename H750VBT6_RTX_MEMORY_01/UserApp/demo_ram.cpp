/**
  ******************************************************************************
  * @file        demo_ram.cpp
  * @modify      OldGerman
  * @created on  Mar 20, 2023
  * @brief       
  ******************************************************************************
  * @attention
  *	    说    明 : TCM，SRAM等五块内存的动态内存分配实现。
  *	              实验目的：
  *	                1. 学习TCM，SRAM等五块内存的动态内存分配实现。
  *
  *    修改记录 :
  *			版本号   日期         作者        说明
  *			V1.0    2018-12-12   Eric2013     1. CMSIS软包版本 V5.4.0
  *	                                          2. HAL库版本 V1.3.0
  *
  *		Copyright (C), 2018-2030, 安富莱电子 www.armfly.com
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "demo_ram.h"
#include "common_inc.h" //提供 printf

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void PrintfHelp(void)
{
	printf("操作提示:\r\n");

	printf("1. 从DTCM依次申请280字节，64字节和6111字节\r\n");
	printf("2. 释放从DTCM申请的空间\r\n");

	printf("3. 从AXI SRAM依次申请160字节，32字节和2333字节\r\n");
	printf("4. 释放从AXI SRAM申请的空间\r\n");

	printf("5. 从D2域SRAM依次申请200字节，96字节和4111字节\r\n");
	printf("6. 释放从D2域SRAM申请的空间\r\n");

	printf("7. 从D3域SRAM依次申请300字节，128字节和5111字节\r\n");
	printf("8. 释放从D3域SRAM申请的空间\r\n");
}

/* Function implementations --------------------------------------------------*/
void DemoRAM(uint8_t cmd)
{
	void *DTCM_Addr0, *AXISRAM_Addr0, *SRAM1_Addr0, *SRAM4_Addr0;
	void *DTCM_Addr1, *AXISRAM_Addr1, *SRAM1_Addr1, *SRAM4_Addr1;
	void *DTCM_Addr2, *AXISRAM_Addr2, *SRAM1_Addr2, *SRAM4_Addr2;

	switch (cmd)
	{
	    case '0':
	    	PrintfHelp();
	    	break;

	    /* 从DTCM依次申请280字节，64字节和6111字节 */
		case '1':
            /* 从DTCM申请280字节空间，使用指针变量DTCM_Addr0操作这些空间时不要超过280字节大小 */
			printf("=========================================================\r\n");
			DTCM_Addr0 = DRAM_DTCM.malloc(280, 0);
			printf("DTCM总大小 = %d字节，申请大小 = 0280字节，当前共使用大小 = %d字节，当前剩余大小 = %d字节，历史最少可用大小 = %d字节\r\n",
					DRAM_DTCM.getMemSize(), DRAM_DTCM.getMemUsed(), DRAM_DTCM.getMemFree(), DRAM_DTCM.getMemFreeMin());

			/* 从DTCM申请64字节空间，使用指针变量DTCM_Addr1操作这些空间时不要超过64字节大小 */
			DTCM_Addr1 = DRAM_DTCM.malloc(64, 0);
			printf("DTCM总大小 = %d字节，申请大小 = 0064字节，当前共使用大小 = %d字节，当前剩余大小 = %d字节，历史最少可用大小 = %d字节\r\n",
					DRAM_DTCM.getMemSize(), DRAM_DTCM.getMemUsed(), DRAM_DTCM.getMemFree(), DRAM_DTCM.getMemFreeMin());

			/* 从DTCM申请6111字节空间，使用指针变量DTCM_Addr2操作这些空间时不要超过6111字节大小 */
			DTCM_Addr2 = DRAM_DTCM.malloc(6111, 0);
			printf("DTCM总大小 = %d字节，申请大小 = 6111字节，当前共使用大小 = %d字节，当前剩余大小 = %d字节，历史最少可用大小 = %d字节\r\n",
					DRAM_DTCM.getMemSize(), DRAM_DTCM.getMemUsed(), DRAM_DTCM.getMemFree(), DRAM_DTCM.getMemFreeMin());
			break;

		/* 释放从DTCM申请的空间 */
		case '2':
			/* 释放从DTCM申请的280字节空间 */
			DRAM_DTCM.free(DTCM_Addr0);
			printf("释放DTCM动态内存区申请的0280字节，当前共使用大小 = %d字节，当前剩余大小 = %d字节，历史最少可用大小 = %d字节\r\n",
					DRAM_DTCM.getMemUsed(), DRAM_DTCM.getMemFree(), DRAM_DTCM.getMemFreeMin());

			/* 释放从DTCM申请的64字节空间 */
			DRAM_DTCM.free(DTCM_Addr1);
			printf("释放DTCM动态内存区申请的0064字节，当前共使用大小 = %d字节，当前剩余大小 = %d字节，历史最少可用大小 = %d字节\r\n",
					DRAM_DTCM.getMemUsed(), DRAM_DTCM.getMemFree(), DRAM_DTCM.getMemFreeMin());

			/* 释放从DTCM申请的6111字节空间 */
			DRAM_DTCM.free(DTCM_Addr2);
			printf("释放DTCM动态内存区申请的6111字节，当前共使用大小 = %d字节，当前剩余大小 = %d字节，历史最少可用大小 = %d字节\r\n",
					DRAM_DTCM.getMemUsed(), DRAM_DTCM.getMemFree(), DRAM_DTCM.getMemFreeMin());
			break;

		/* 从AXI SRAM依次申请160字节，32字节和2333字节 */
		case '3':
            /* 从AXI SRAM 申请160字节空间，使用指针变量AXISRAM_Addr0操作这些空间时不要超过160字节大小 */
			printf("=========================================================\r\n");
			AXISRAM_Addr0 = DRAM_AXISRAM.malloc(160, 0);
			printf("AXI SRAM总大小 = %d字节，申请大小 = 0162字节，当前共使用大小 = %d字节，当前剩余大小 = %d字节，历史最少可用大小 = %d字节\r\n",
					DRAM_AXISRAM.getMemSize(), DRAM_AXISRAM.getMemUsed(), DRAM_AXISRAM.getMemFree(), DRAM_AXISRAM.getMemFreeMin());

			/* 从AXI SRAM 申请32字节空间，使用指针变量AXISRAM_Addr1操作这些空间时不要超过32字节大小 */
			AXISRAM_Addr1 = DRAM_AXISRAM.malloc(32, 0);
			printf("AXI SRAM总大小 = %d字节，申请大小 = 0032字节，当前共使用大小 = %d字节，当前剩余大小 = %d字节，历史最少可用大小 = %d字节\r\n",
					DRAM_AXISRAM.getMemSize(), DRAM_AXISRAM.getMemUsed(), DRAM_AXISRAM.getMemFree(), DRAM_AXISRAM.getMemFreeMin());

			/* 从AXI SRAM 申请2333字节空间，使用指针变量AXISRAM_Addr2操作这些空间时不要超过2333字节大小 */
			AXISRAM_Addr2 = DRAM_AXISRAM.malloc(2333, 0);
			printf("AXI SRAM总大小 = %d字节，申请大小 = 2333字节，当前共使用大小 = %d字节，当前剩余大小 = %d字节，历史最少可用大小 = %d字节\r\n",
					DRAM_AXISRAM.getMemSize(), DRAM_AXISRAM.getMemUsed(), DRAM_AXISRAM.getMemFree(), DRAM_AXISRAM.getMemFreeMin());
			break;

		/* 释放从AXI SRAM申请的空间 */
		case '4':
			/* 释放从AXI SRAM申请的160字节空间 */
			DRAM_AXISRAM.free(AXISRAM_Addr0);
			printf("释放AXI SRAM动态内存区申请的0160字节，当前共使用大小 = %d字节，当前剩余大小 = %d字节，历史最少可用大小 = %d字节\r\n",
					DRAM_AXISRAM.getMemUsed(), DRAM_AXISRAM.getMemFree(), DRAM_AXISRAM.getMemFreeMin());

			/* 释放从AXI SRAM申请的32字节空间 */
			DRAM_AXISRAM.free(AXISRAM_Addr1);
			printf("释放AXI SRAM动态内存区申请的0032字节，当前共使用大小 = %d字节，当前剩余大小 = %d字节，历史最少可用大小 = %d字节\r\n",
					DRAM_AXISRAM.getMemUsed(), DRAM_AXISRAM.getMemFree(), DRAM_AXISRAM.getMemFreeMin());

			/* 释放从AXI SRAM申请的2333字节空间 */
			DRAM_AXISRAM.free(AXISRAM_Addr2);
			printf("释放AXI SRAM动态内存区申请的2333字节，当前共使用大小 = %d字节，当前剩余大小 = %d字节，历史最少可用大小 = %d字节\r\n",
					DRAM_AXISRAM.getMemUsed(), DRAM_AXISRAM.getMemFree(), DRAM_AXISRAM.getMemFreeMin());
			break;

		/* 从D2域SRAM依次申请200字节，96字节和4111字节 */
		case '5':
            /* 从D2域的SRAM申请200字节空间，使用指针变量SRAM1_Addr0操作这些空间时不要超过200字节大小 */
			printf("=========================================================\r\n");
			SRAM1_Addr0 = DRAM_SRAM1.malloc(200, 0);
			printf("D2域SRAM总大小 = %d字节，申请大小 = 0200字节，当前共使用大小 = %d字节，当前剩余大小 = %d字节，历史最少可用大小 = %d字节\r\n",
					DRAM_SRAM1.getMemSize(), DRAM_SRAM1.getMemUsed(), DRAM_SRAM1.getMemFree(), DRAM_SRAM1.getMemFreeMin());

			/* 从D2域的SRAM申请96字节空间，使用指针变量SRAM1_Addr1操作这些空间时不要超过96字节大小 */
			SRAM1_Addr1 = DRAM_SRAM1.malloc(96, 0);
			printf("D2域SRAM总大小 = %d字节，申请大小 = 0096字节，当前共使用大小 = %d字节，当前剩余大小 = %d字节，历史最少可用大小 = %d字节\r\n",
					DRAM_SRAM1.getMemSize(), DRAM_SRAM1.getMemUsed(), DRAM_SRAM1.getMemFree(), DRAM_SRAM1.getMemFreeMin());

			/* 从D2域的SRAM申请4111字节空间，使用指针变量SRAM1_Addr2操作这些空间时不要超过4111字节大小 */
			SRAM1_Addr2 = DRAM_SRAM1.malloc(4111, 0);
			printf("D2域SRAM总大小 = %d字节，申请大小 = 4111字节，当前共使用大小 = %d字节，当前剩余大小 = %d字节，历史最少可用大小 = %d字节\r\n",
					DRAM_SRAM1.getMemSize(), DRAM_SRAM1.getMemUsed(), DRAM_SRAM1.getMemFree(), DRAM_SRAM1.getMemFreeMin());
			break;

		/* 释放从D2域SRAM申请的空间 */
		case '6':
			/* 释放从D2域的SRAM申请的200字节空间 */
			DRAM_SRAM1.free(SRAM1_Addr0);
			printf("释放D2域SRAM动态内存区申请的0200字节，当前共使用大小 = %d字节，当前剩余大小 = %d字节，历史最少可用大小 = %d字节\r\n",
					DRAM_SRAM1.getMemUsed(), DRAM_SRAM1.getMemFree(), DRAM_SRAM1.getMemFreeMin());

			/* 释放从D2域的SRAM申请的96字节空间 */
			DRAM_SRAM1.free(SRAM1_Addr1);
			printf("释放D2域SRAM动态内存区申请的0096字节，当前共使用大小 = %d字节，当前剩余大小 = %d字节，历史最少可用大小 = %d字节\r\n",
					DRAM_SRAM1.getMemUsed(), DRAM_SRAM1.getMemFree(), DRAM_SRAM1.getMemFreeMin());

			/* 释放从D2域的SRAM申请的4111字节空间 */
			DRAM_SRAM1.free(SRAM1_Addr2);
			printf("释放D2域SRAM动态内存区申请的4111字节，当前共使用大小 = %d字节，当前剩余大小 = %d字节，历史最少可用大小 = %d字节\r\n",
					DRAM_SRAM1.getMemUsed(), DRAM_SRAM1.getMemFree(), DRAM_SRAM1.getMemFreeMin());
			break;

		/* 从D3域SRAM依次申请300字节，128字节和5111字节 */
		case '7':
            /* 从D3域的SRAM申请300字节空间，使用指针变量SRAM4_Addr0操作这些空间时不要超过300字节大小 */
			printf("=========================================================\r\n");
			SRAM4_Addr0 = DRAM_SRAM4.malloc(300, 0);
			printf("D3域SRAM总大小 = %d字节，申请大小 = 0300字节，当前共使用大小 = %d字节，当前剩余大小 = %d字节，历史最少可用大小 = %d字节\r\n",
					DRAM_SRAM4.getMemSize(), DRAM_SRAM4.getMemUsed(), DRAM_SRAM4.getMemFree(), DRAM_SRAM4.getMemFreeMin());

			/* 从D3域的SRAM申请96字节空间，使用指针变量SRAM4_Addr1操作这些空间时不要超过96字节大小 */
			SRAM4_Addr1 = DRAM_SRAM4.malloc(128, 0);
			printf("D3域SRAM总大小 = %d字节，申请大小 = 0128字节，当前共使用大小 = %d字节，当前剩余大小 = %d字节，历史最少可用大小 = %d字节\r\n",
					DRAM_SRAM4.getMemSize(), DRAM_SRAM4.getMemUsed(), DRAM_SRAM4.getMemFree(), DRAM_SRAM4.getMemFreeMin());

			/* 从D3域的SRAM申请5111字节空间，使用指针变量SRAM4_Addr2操作这些空间时不要超过5111字节大小 */
			SRAM4_Addr2 = DRAM_SRAM4.malloc(5111, 0);
			printf("D3域SRAM总大小 = %d字节，申请大小 = 5111字节，当前共使用大小 = %d字节，当前剩余大小 = %d字节，历史最少可用大小 = %d字节\r\n",
					DRAM_SRAM4.getMemSize(), DRAM_SRAM4.getMemUsed(), DRAM_SRAM4.getMemFree(), DRAM_SRAM4.getMemFreeMin());
			break;

		/* 释放从D3域SRAM申请的空间 */
		case '8':
			/* 释放从D3域的SRAM申请的300字节空间 */
			DRAM_SRAM4.free(SRAM4_Addr0);
			printf("释放D3域SRAM动态内存区申请的0300字节，当前共使用大小 = %d字节，当前剩余大小 = %d字节，历史最少可用大小 = %d字节\r\n",
					DRAM_SRAM4.getMemUsed(), DRAM_SRAM4.getMemFree(), DRAM_SRAM4.getMemFreeMin());

			/* 释放从D3域的SRAM申请的128字节空间 */
			DRAM_SRAM4.free(SRAM4_Addr1);
			printf("释放D3域SRAM动态内存区申请的0128字节，当前共使用大小 = %d字节，当前剩余大小 = %d字节，历史最少可用大小 = %d字节\r\n",
					DRAM_SRAM4.getMemUsed(), DRAM_SRAM4.getMemFree(), DRAM_SRAM4.getMemFreeMin());

			/* 释放从D3域的SRAM申请的5111字节空间 */
			DRAM_SRAM4.free(SRAM4_Addr2);
			printf("释放D3域SRAM动态内存区申请的5111字节，当前共使用大小 = %d字节，当前剩余大小 = %d字节，历史最少可用大小 = %d字节\r\n",
					DRAM_SRAM4.getMemUsed(), DRAM_SRAM4.getMemFree(), DRAM_SRAM4.getMemFreeMin());
			break;

		default:
		  /* 其它的键值不处理 */
		  break;
	}
}

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
	void *DTCM_Addres0, *AXISRAM_Addres0, *SRAM1_Addres0, *SRAM4_Addres0;
	void *DTCM_Addres1, *AXISRAM_Addres1, *SRAM1_Addres1, *SRAM4_Addres1;
	void  *DTCM_Addres2, *AXISRAM_Addres2, *SRAM1_Addres2, *SRAM4_Addres2;

	switch (cmd)
	{
	    case '0':
	    	PrintfHelp();
	    	break;

	    /* 从DTCM依次申请280字节，64字节和6111字节 */
		case '1':
            /* 从DTCM申请280字节空间，使用指针变量DTCM_Addres0操作这些空间时不要超过280字节大小 */
			printf("=========================================================\r\n");
			DTCM_Addres0 = osRtxMemoryAlloc(AppMallocDTCM, 280, 0);
			DTCMUsed = MemHeadPtr(AppMallocDTCM);
			printf("DTCM总大小 = %d字节，申请大小 = 0280字节，当前共使用大小 = %d字节\r\n",
		                                                DTCMUsed->size, DTCMUsed->used);

			/* 从DTCM申请64字节空间，使用指针变量DTCM_Addres1操作这些空间时不要超过64字节大小 */
			DTCM_Addres1 = osRtxMemoryAlloc(AppMallocDTCM, 64, 0);
			DTCMUsed = MemHeadPtr(AppMallocDTCM);
			printf("DTCM总大小 = %d字节，申请大小 = 0064字节，当前共使用大小 = %d字节\r\n",
									                   DTCMUsed->size, DTCMUsed->used);

			/* 从DTCM申请6111字节空间，使用指针变量DTCM_Addres2操作这些空间时不要超过6111字节大小 */
			DTCM_Addres2 = osRtxMemoryAlloc(AppMallocDTCM, 6111, 0);
			DTCMUsed = MemHeadPtr(AppMallocDTCM);
			printf("DTCM总大小 = %d字节，申请大小 = 6111字节，当前共使用大小 = %d字节\r\n",
		                                                DTCMUsed->size, DTCMUsed->used);
			break;

		/* 释放从DTCM申请的空间 */
		case '2':
			/* 释放从DTCM申请的280字节空间 */
			osRtxMemoryFree(AppMallocDTCM, DTCM_Addres0);
			DTCMUsed = MemHeadPtr(AppMallocDTCM);
			printf("释放DTCM动态内存区申请的0280字节，当前共使用大小 = %d字节\r\n", DTCMUsed->used);

			/* 释放从DTCM申请的64字节空间 */
			osRtxMemoryFree(AppMallocDTCM, DTCM_Addres1);
			DTCMUsed = MemHeadPtr(AppMallocDTCM);
			printf("释放DTCM动态内存区申请的0064字节，当前共使用大小 = %d字节\r\n", DTCMUsed->used);

			/* 释放从DTCM申请的6111字节空间 */
			osRtxMemoryFree(AppMallocDTCM, DTCM_Addres2);
			DTCMUsed = MemHeadPtr(AppMallocDTCM);
			printf("释放DTCM动态内存区申请的6111字节，当前共使用大小 = %d字节\r\n", DTCMUsed->used);
			break;

		/* 从AXI SRAM依次申请160字节，32字节和2333字节 */
		case '3':
            /* 从AXI SRAM 申请160字节空间，使用指针变量AXISRAM_Addres0操作这些空间时不要超过160字节大小 */
			printf("=========================================================\r\n");
			AXISRAM_Addres0 = osRtxMemoryAlloc(AppMallocAXISRAM, 160, 0);
			AXISRAMUsed = MemHeadPtr(AppMallocAXISRAM);
			printf("AXI SRAM总大小 = %d字节，申请大小 = 0162字节，当前共使用大小 = %d字节\r\n",
		                                                AXISRAMUsed->size, AXISRAMUsed->used);

			/* 从AXI SRAM 申请32字节空间，使用指针变量AXISRAM_Addres1操作这些空间时不要超过32字节大小 */
			AXISRAM_Addres1 = osRtxMemoryAlloc(AppMallocAXISRAM, 32, 0);
			AXISRAMUsed = MemHeadPtr(AppMallocAXISRAM);
			printf("AXI SRAM总大小 = %d字节，申请大小 = 0032字节，当前共使用大小 = %d字节\r\n",
									                   AXISRAMUsed->size, AXISRAMUsed->used);

			/* 从AXI SRAM 申请2333字节空间，使用指针变量AXISRAM_Addres2操作这些空间时不要超过2333字节大小 */
			AXISRAM_Addres2 = osRtxMemoryAlloc(AppMallocAXISRAM, 2333, 0);
			AXISRAMUsed = MemHeadPtr(AppMallocAXISRAM);
			printf("AXI SRAM总大小 = %d字节，申请大小 = 2333字节，当前共使用大小 = %d字节\r\n",
		                                                AXISRAMUsed->size, AXISRAMUsed->used);
			break;

		/* 释放从AXI SRAM申请的空间 */
		case '4':
			/* 释放从AXI SRAM申请的160字节空间 */
			osRtxMemoryFree(AppMallocAXISRAM, AXISRAM_Addres0);
			AXISRAMUsed = MemHeadPtr(AppMallocAXISRAM);
			printf("释放AXI SRAM动态内存区申请的0160字节，当前共使用大小 = %d字节\r\n", AXISRAMUsed->used);

			/* 释放从AXI SRAM申请的32字节空间 */
			osRtxMemoryFree(AppMallocAXISRAM, AXISRAM_Addres1);
			AXISRAMUsed = MemHeadPtr(AppMallocAXISRAM);
			printf("释放AXI SRAM动态内存区申请的0032字节，当前共使用大小 = %d字节\r\n", AXISRAMUsed->used);

			/* 释放从AXI SRAM申请的2333字节空间 */
			osRtxMemoryFree(AppMallocAXISRAM, AXISRAM_Addres2);
			AXISRAMUsed = MemHeadPtr(AppMallocAXISRAM);
			printf("释放AXI SRAM动态内存区申请的2333字节，当前共使用大小 = %d字节\r\n", AXISRAMUsed->used);
			break;

		/* 从D2域SRAM依次申请200字节，96字节和4111字节 */
		case '5':
            /* 从D2域的SRAM申请200字节空间，使用指针变量SRAM1_Addres0操作这些空间时不要超过200字节大小 */
			printf("=========================================================\r\n");
			SRAM1_Addres0 = osRtxMemoryAlloc(AppMallocSRAM1, 200, 0);
			SRAM1Used = MemHeadPtr(AppMallocSRAM1);
			printf("D2域SRAM总大小 = %d字节，申请大小 = 0200字节，当前共使用大小 = %d字节\r\n",
		                                                SRAM1Used->size, SRAM1Used->used);

			/* 从D2域的SRAM申请96字节空间，使用指针变量SRAM1_Addres1操作这些空间时不要超过96字节大小 */
			SRAM1_Addres1 = osRtxMemoryAlloc(AppMallocSRAM1, 96, 0);
			SRAM1Used = MemHeadPtr(AppMallocSRAM1);
			printf("D2域SRAM总大小 = %d字节，申请大小 = 0096字节，当前共使用大小 = %d字节\r\n",
									                   SRAM1Used->size, SRAM1Used->used);

			/* 从D2域的SRAM申请4111字节空间，使用指针变量SRAM1_Addres2操作这些空间时不要超过4111字节大小 */
			SRAM1_Addres2 = osRtxMemoryAlloc(AppMallocSRAM1, 4111, 0);
			SRAM1Used = MemHeadPtr(AppMallocSRAM1);
			printf("D2域SRAM总大小 = %d字节，申请大小 = 4111字节，当前共使用大小 = %d字节\r\n",
		                                                SRAM1Used->size, SRAM1Used->used);
			break;

		/* 释放从D2域SRAM申请的空间 */
		case '6':
			/* 释放从D2域的SRAM申请的200字节空间 */
			osRtxMemoryFree(AppMallocSRAM1, SRAM1_Addres0);
			SRAM1Used = MemHeadPtr(AppMallocSRAM1);
			printf("释放D2域SRAM动态内存区申请的0200字节，当前共使用大小 = %d字节\r\n", SRAM1Used->used);

			/* 释放从D2域的SRAM申请的96字节空间 */
			osRtxMemoryFree(AppMallocSRAM1, SRAM1_Addres1);
			SRAM1Used = MemHeadPtr(AppMallocSRAM1);
			printf("释放D2域SRAM动态内存区申请的0096字节，当前共使用大小 = %d字节\r\n", SRAM1Used->used);

			/* 释放从D2域的SRAM申请的4111字节空间 */
			osRtxMemoryFree(AppMallocSRAM1, SRAM1_Addres2);
			SRAM1Used = MemHeadPtr(AppMallocSRAM1);
			printf("释放D2域SRAM动态内存区申请的4111字节，当前共使用大小 = %d字节\r\n", SRAM1Used->used);
			break;

		/* 从D3域SRAM依次申请300字节，128字节和5111字节 */
		case '7':
            /* 从D3域的SRAM申请300字节空间，使用指针变量SRAM4_Addres0操作这些空间时不要超过300字节大小 */
			printf("=========================================================\r\n");
			SRAM4_Addres0 = osRtxMemoryAlloc(AppMallocSRAM4, 300, 0);
			SRAM4Used = MemHeadPtr(AppMallocSRAM4);
			printf("D3域SRAM总大小 = %d字节，申请大小 = 0300字节，当前共使用大小 = %d字节\r\n",
		                                                SRAM4Used->size, SRAM4Used->used);

			/* 从D3域的SRAM申请96字节空间，使用指针变量SRAM4_Addres1操作这些空间时不要超过96字节大小 */
			SRAM4_Addres1 = osRtxMemoryAlloc(AppMallocSRAM4, 128, 0);
			SRAM4Used = MemHeadPtr(AppMallocSRAM4);
			printf("D3域SRAM总大小 = %d字节，申请大小 = 0128字节，当前共使用大小 = %d字节\r\n",
									                   SRAM4Used->size, SRAM4Used->used);

			/* 从D3域的SRAM申请5111字节空间，使用指针变量SRAM4_Addres2操作这些空间时不要超过5111字节大小 */
			SRAM4_Addres2 = osRtxMemoryAlloc(AppMallocSRAM4, 5111, 0);
			SRAM4Used = MemHeadPtr(AppMallocSRAM4);
			printf("D3域SRAM总大小 = %d字节，申请大小 = 5111字节，当前共使用大小 = %d字节\r\n",
		                                                SRAM4Used->size, SRAM4Used->used);
			break;

		/* 释放从D3域SRAM申请的空间 */
		case '8':
			/* 释放从D3域的SRAM申请的300字节空间 */
			osRtxMemoryFree(AppMallocSRAM4, SRAM4_Addres0);
			SRAM4Used = MemHeadPtr(AppMallocSRAM4);
			printf("释放D3域SRAM动态内存区申请的0300字节，当前共使用大小 = %d字节\r\n", SRAM4Used->used);

			/* 释放从D3域的SRAM申请的128字节空间 */
			osRtxMemoryFree(AppMallocSRAM4, SRAM4_Addres1);
			SRAM4Used = MemHeadPtr(AppMallocSRAM4);
			printf("释放D3域SRAM动态内存区申请的0128字节，当前共使用大小 = %d字节\r\n", SRAM4Used->used);

			/* 释放从D3域的SRAM申请的5111字节空间 */
			osRtxMemoryFree(AppMallocSRAM4, SRAM4_Addres2);
			SRAM4Used = MemHeadPtr(AppMallocSRAM4);
			printf("释放D3域SRAM动态内存区申请的5111字节，当前共使用大小 = %d字节\r\n", SRAM4Used->used);
			break;

		default:
		  /* 其它的键值不处理 */
		  break;
	}
}

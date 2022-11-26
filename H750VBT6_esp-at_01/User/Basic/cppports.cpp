/*
 * cppports.cpp
 *
 *  Created on: May 16, 2022
 *      Author: OldGerman
 */

#include "cppports.h"
#include "bsp.h"
#include "binary.h"

#ifndef DBG_PRINT
#if 0  //< Change 0 to 1 to open debug macro and check program debug information
#define DBG_PRINT usb_printf
#else
	#define DBG_PRINT(...)
	#endif
#endif



static void PrintfInfo(void)
{
	printf("*************************************************************\n\r");

	/* 检测CPU ID */
	{
		uint32_t CPU_Sn0, CPU_Sn1, CPU_Sn2;

		CPU_Sn0 = *(__IO uint32_t*)(0x1FF1E800);
		CPU_Sn1 = *(__IO uint32_t*)(0x1FF1E800 + 4);
		CPU_Sn2 = *(__IO uint32_t*)(0x1FF1E800 + 8);

		printf("CPU : STM32H750VBT6, LQFP100, 主频: %ldMHz\r\n", SystemCoreClock / 1000000);
		printf("UID = %08X %08X %08X\n\r", (unsigned int)CPU_Sn2, (unsigned int)CPU_Sn1, (unsigned int)CPU_Sn0);
	}
}

static void PrintfHelp(void)
{
	printf("*************************************************************\n\r");
	printf("ESP-HOSTED-FG吞吐量测试:\r\n");
	printf("\r\n");
	printf("\r\n");
	printf("\r\n");
	printf("操作提示:\r\n");
	printf("1. \r\n");
	printf("2. r\n");
	printf("3. \r\n");
	printf("4. \r\n");
	printf("\r\n");
}

void btA_CLICKED_func(){

}

void btB_CLICKED_func(){

}

void btA_LONG_PRESSED_func(){

}

void btB_LONG_PRESSED_func(){

}


void setup(){
	bsp_Init();
	PrintfInfo();
	PrintfHelp();

//	bsp_tim6_enable_IT();	//指示系统正在运行

}

// armfly串口FIFO的comGetChar函数正确使用姿势
// https://www.armbbs.cn/forum.php?mod=viewthread&tid=94579&extra=page%3D1

enum ucStatus {
	ucStatus_waitCmd = 0,
	ucStatus_setFirstHalf,
	ucStatus_setSecondHalf,
	ucStatus_readBitsChar
};
void loop(){
    uint8_t read;
    uint8_t ucStatus = ucStatus_waitCmd;  /* 状态机标志 */
//    uint8_t ucCount = 0, i;
//    uint8_t buf[128];
//    bool setFirstHalf = 0;
	while(1) {
//		static uint32_t timeOld = HAL_GetTick();
//		if(waitTime(&timeOld, 10)){
//			bsp_Button_Update();
//			if (comGetChar(COM1, &read))
//			{
//				switch (ucStatus)
//				{
//				/* 状态0保证接收到A或B */
//				case ucStatus_waitCmd:
//					break;
//
//				case ucStatus_setFirstHalf:
//					break;
//
//				case ucStatus_setSecondHalf:
//					break;
//
//				case ucStatus_readBitsChar:
//					break;
//				}
//			}
//		}
	}
}
/* Demo:

*/

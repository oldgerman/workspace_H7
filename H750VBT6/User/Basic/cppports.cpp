/*
 * cppports.cpp
 *
 *  Created on: May 16, 2022
 *      Author: OldGerman
 */

#include "cppports.h"
#include "bsp.h"
#ifndef DBG_PRINT
#if 0  //< Change 0 to 1 to open debug macro and check program debug information
#define DBG_PRINT usb_printf
#else
	#define DBG_PRINT(...)
	#endif
#endif





/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"V7-QSPI Flash的读写例程"
#define EXAMPLE_DATE	"2020-11-01"
#define DEMO_VER		"1.0"

//static void PrintfLogo(void);
//extern void DemoSpiFlash(void);

/*
*********************************************************************************************************
*	函 数 名: PrintfLogo
*	功能说明: 打印例程名称和例程发布日期, 接上串口线后，打开PC机的超级终端软件可以观察结果
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void PrintfInfo(void)
{
	printf("*************************************************************\n\r");

	/* 检测CPU ID */
	{
		uint32_t CPU_Sn0, CPU_Sn1, CPU_Sn2;

		CPU_Sn0 = *(__IO uint32_t*)(0x1FF1E800);
		CPU_Sn1 = *(__IO uint32_t*)(0x1FF1E800 + 4);
		CPU_Sn2 = *(__IO uint32_t*)(0x1FF1E800 + 8);

		printf("\r\nCPU : STM32H750VBT6, LQFP100, 主频: %ldMHz\r\n", SystemCoreClock / 1000000);
		printf("UID = %08X %08X %08X\n\r", (unsigned int)CPU_Sn2, (unsigned int)CPU_Sn1, (unsigned int)CPU_Sn0);
	}
}


void setup(){
	bsp_InitUart();
}

void loop(){
//	uint8_t read;
//	const char buf1[] = "接收到串口命令 1\r\n";
//	const char buf2[] = "接收到串口命令 2\r\n";
//	const char buf3[] = "接收到串口命令 3\r\n";
//	const char buf4[] = "接收到串口命令 4\r\n";
	while(1) {
#if 0
		printf("Hello World!\r\n");
		HAL_Delay(500);
		//		uint8_t rd[10] = {0};
		//		scanf("%s",rd);
		//
		//		printf("hello %s\r\n",rd);

		if (comGetChar(COM1, &read)) {
//			if(scanf("%s", &read)) {
			switch (read) {
			case '1': comSendBuf(COM1, (uint8_t *)buf1, strlen(buf1)); break;
			case '2': comSendBuf(COM1, (uint8_t *)buf2, strlen(buf2)); break;
			case '3': comSendBuf(COM1, (uint8_t *)buf3, strlen(buf3)); break;
			case '4': comSendBuf(COM1, (uint8_t *)buf4, strlen(buf4)); break;
			default: break;
			}
		}
#else
		PrintfInfo();	/* 打印例程名称和版本等信息 */
		DemoSpiFlash();   /* QSPI Flash测试 */
#endif
	}
}

void MIX_Update()
{
	;
}

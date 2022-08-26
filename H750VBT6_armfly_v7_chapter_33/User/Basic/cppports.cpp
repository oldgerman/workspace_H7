/*
 * cppports.cpp
 *
 *  Created on: May 16, 2022
 *      Author: OldGerman
 */

#include "cppports.h"
#include "bsp.h"

//	static void MIX_Update();
//	void MIX_Update()
//	{
//		;
//	}

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

static void PrintfHelp(void)
{
	printf("*************************************************************\n\r");
	printf("定时器周期性中断(驱动支持TIM6), 在TIM6中断回调函数控制PC3 io翻转周期为50ms\r\n");
	printf("操作提示:\r\n");
	printf("1. KEY A按下，开启TIM6的周期性中断\r\n");
	printf("2. KEY B按下，关闭TIM6的周期性中断\r\n");
}


void setup(){
	bsp_Init();
	PrintfInfo();
	PrintfHelp();
}

void loop(){
	while(1) {
		static uint32_t timeOld = HAL_GetTick();
		if(waitTime(&timeOld, 50)){
			if(HAL_GPIO_ReadPin(KEY_A_GPIO_Port, KEY_A_Pin) == GPIO_PIN_SET) {
				bsp_tim6_enable_IT();
				printf("检测到KEY A 按下，打开tim6中断\r\n");
			}
			if(HAL_GPIO_ReadPin(KEY_B_GPIO_Port, KEY_B_Pin) == GPIO_PIN_RESET) {
				bsp_tim6_disable_IT();
				printf("检测到KEY B 按下，关闭tim6中断\r\n");
				HAL_GPIO_WritePin(LRGB_G_GPIO_Port, LRGB_G_Pin, GPIO_PIN_SET);
			}
		}
	}
}

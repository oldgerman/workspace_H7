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
	printf("定时器pwm调节测试:\r\n\r\n");
	printf("定时器通道对应输出引脚 PB1: TIM3 CH4   PB15: TIM12 CH2\r\n");
	printf("操作提示:\r\n");
	printf("1. KEY A短按实现一个 5 微秒的单次延迟，开启后翻转PA1，时间到后翻转PA1\r\n");
	printf("2. KEY B短按实现一个 10 微秒的单次延迟，开启后翻转PA1，时间到后翻转PA1\r\n");
	printf("\r\n");
}

static void timerCallBack(){
	HAL_GPIO_TogglePin(LRGB_R_GPIO_Port, LRGB_R_Pin);
}

void btA_CLICKED_func(){
	printf("检测到 KEY A 短按\r\n");
	HAL_GPIO_TogglePin(LRGB_R_GPIO_Port, LRGB_R_Pin);
	bsp_StartHardTimer(TIM_CHANNEL_1, 5, timerCallBack);
}

void btB_CLICKED_func(){
	printf("检测到 KEY B 短按\r\n");
	HAL_GPIO_TogglePin(LRGB_R_GPIO_Port, LRGB_R_Pin);
	bsp_StartHardTimer(TIM_CHANNEL_2, 10, timerCallBack);
}

void btA_LONG_PRESSED_func(){
;
}

void btB_LONG_PRESSED_func(){
;
}

void setup(){
	bsp_Init();
	PrintfInfo();
	PrintfHelp();
	bsp_tim6_enable_IT();	//全程保持LRGB_B闪烁
}

void loop(){
	while(1) {
		static uint32_t timeOld = HAL_GetTick();
		if(waitTime(&timeOld, 10)){
			bsp_Button_Update();
		}
	}
}

/* Demo:
	*************************************************************
	CPU : STM32H750VBT6, LQFP100, 主频: 400MHz
	UID = 32363235 31305114 001F002C
	*************************************************************
	定时器pwm调节测试:

	定时器通道对应输出引脚 PB1: TIM3 CH4   PB15: TIM12 CH2
	操作提示:
	1. KEY A短按实现一个 5 微秒的单次延迟，开启后翻转PA1，时间到后翻转PA1
	2. KEY B短按实现一个 10 微秒的单次延迟，开启后翻转PA1，时间到后翻转PA1

	检测到 KEY A 短按
	......
	检测到 KEY B 短按
	......
 */

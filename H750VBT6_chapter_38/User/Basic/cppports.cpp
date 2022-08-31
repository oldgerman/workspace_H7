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


uint32_t lptim_timeOut_us = 256;	//256us
//bool en_Stop_Mode = false;			//停机模式使能标志

static void lptim_timeOut_CallBack();

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
	printf("低功耗定时器停机模式唤醒测试:\r\n");
	printf("低功耗定时器时钟源 LSE: 32768Hz\r\n");
	printf("每次在超时中断回调函数中置PA1为低电平\r\n");
	printf("操作提示:\r\n");
	printf("1. KEY A 长按或连续长按，以10倍增量修改超时时间，短按以应用修改，并置PA1为高电平\r\n");
	printf("2. KEY B 短按进入停机模式\r\n");
	printf("\r\n");
}

void btA_CLICKED_func(){
	printf("应用期望超时时间： %ldus\r\n", lptim_timeOut_us);
	bsp_tim6_disable_IT();
	uint32_t timeCal_us = bsp_LPTIMx_TimeOut_Set(&hlptim1, 32768, lptim_timeOut_us, lptim_timeOut_CallBack);
	printf("实际超时时间： %ldus\r\n", timeCal_us);
	HAL_GPIO_WritePin(LRGB_R_GPIO_Port, LRGB_R_Pin, GPIO_PIN_RESET);
	bsp_LPTIMx_TimeOut_En(true);
}

void btB_CLICKED_func(){
	printf("进入停机模式, %ldus后唤醒\r\n", lptim_timeOut_us);
	HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
}

void btA_LONG_PRESSED_func(){
	lptim_timeOut_us *= 10;
	if(lptim_timeOut_us > 256000000){
		lptim_timeOut_us = 256;
	}
	printf("修改期望超时时间： %ldus\r\n", lptim_timeOut_us);
}

void btB_LONG_PRESSED_func(){
	;
}

static void lptim_timeOut_CallBack(){
	SystemClock_Config();   			// 从停机模式退出需要首先执行时钟配置
	HAL_GPIO_WritePin(LRGB_R_GPIO_Port, LRGB_R_Pin, GPIO_PIN_SET);
	bsp_tim6_enable_IT();				//	指示系统正在运行
	printf("执行超时中断回调函数，并退出停机模式\r\n");
}

void setup(){
	bsp_Init();
	PrintfInfo();
	PrintfHelp();
	bsp_tim6_enable_IT();	//指示系统正在运行
	HAL_GPIO_WritePin(LRGB_R_GPIO_Port, LRGB_R_Pin, GPIO_PIN_SET);	//预置PA1为高电平
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

*/

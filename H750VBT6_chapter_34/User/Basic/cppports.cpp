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

//static void PrintfLogo(void);
//extern void DemoSpiFlash(void);

static pwmSet_InfoTypeDef pwmSetInfo_TIM3;
static pwmSet_InfoTypeDef pwmSetInfo_TIM12;

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
	printf("1. KEY A长按或连续长按，以10倍增量修改TIM3  pwm频率，短按以应用修改\r\n");
	printf("2. KEY B长按或连续长按，以10倍增量修改TIM12 pwm频率，短按以应用修改\r\n");
	printf("\r\n");
}

static void Printf_pwmSetInfo_TIMx(pwmSet_InfoTypeDef *pwmSetInfo)
{
	printf("期望pwm占空比： %f%%\r\n", pwmSetInfo->pwm_Dutycycle_Expect);
	printf("期望pwm频率： %ldHz\r\n", pwmSetInfo->pwm_Frequency_Expect);
	printf("实际pwm占空比： %f%%\r\n", pwmSetInfo->pwm_Dutycycle);
	printf("实际pwm占空比步幅： %f%%\r\n", pwmSetInfo->pwmStep_Dutycycle);
	printf("实际pwm频率： %fHz\r\n", pwmSetInfo->pwm_Frequency);
	printf("\r\n");
}


uint32_t tim3_pwm_hz = 7;
uint32_t tim12_pwm_hz = 3;

void btA_CLICKED_func(){
	bsp_tim6_enable_IT();
	printf("TIM3:\r\n");
	pwmSetInfo_TIM3 = bsp_TIMx_PWM_Set(&htim3, TIM_CHANNEL_4, tim3_pwm_hz, 50.0);
	bsp_TIMx_PWM_En(&htim3, TIM_CHANNEL_4, true);
	Printf_pwmSetInfo_TIMx(&pwmSetInfo_TIM3);
}

void btB_CLICKED_func(){
	bsp_tim6_disable_IT();
	printf("TIM12:\r\n");
	HAL_GPIO_WritePin(LRGB_G_GPIO_Port, LRGB_G_Pin, GPIO_PIN_SET);
	pwmSetInfo_TIM12 = bsp_TIMx_PWM_Set(&htim12, TIM_CHANNEL_2, tim12_pwm_hz, 66.6);
	bsp_TIMx_PWM_En(&htim12, TIM_CHANNEL_2, true);
	Printf_pwmSetInfo_TIMx(&pwmSetInfo_TIM12);
}

void btA_LONG_PRESSED_func(){
	printf("TIM3:\r\n");
	static uint8_t cnt = 0;
	++cnt;
	tim3_pwm_hz *= 10;
	if(cnt > 7){
		tim3_pwm_hz /= 100000000;
		cnt = 0;
	}
	printf("修改期望pwm频率： %ldHz\r\n", tim3_pwm_hz);
}

void btB_LONG_PRESSED_func(){
	printf("TIM12:\r\n");
	static uint8_t cnt = 0;
	++cnt;
	tim12_pwm_hz *= 10;
	if(cnt > 7){
		tim12_pwm_hz /= 100000000;
		cnt = 0;
	}
	printf("修改pwm频率： %ldHz\r\n", tim12_pwm_hz);
}

void setup(){
	bsp_Init();
	PrintfInfo();
	PrintfHelp();
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
 *
	*************************************************************
	CPU : STM32H750VBT6, LQFP100, 主频: 400MHz
	UID = 32363235 31305114 001F002C
	*************************************************************
	定时器pwm调节测试:

	定时器通道对应输出引脚 PB1: TIM3 CH4   PB15: TIM12 CH2
	操作提示:
	1. KEY A长按或连续长按，以10倍增量修改TIM3  pwm频率，短按以应用修改
	2. KEY B长按或连续长按，以10倍增量修改TIM12 pwm频率，短按以应用修改

	TIM3:
	修改期望pwm频率： 7Hz
	TIM3:
	修改期望pwm频率： 70Hz
	TIM3:
	修改期望pwm频率： 700Hz
	TIM3:
	修改期望pwm频率： 7000Hz
	TIM3:
	修改期望pwm频率： 70000Hz
	TIM3:
	修改期望pwm频率： 700000Hz
	TIM3:
	修改期望pwm频率： 7000000Hz
	TIM3:
	修改期望pwm频率： 70000000Hz

	TIM12:
	修改pwm频率： 3Hz
	TIM12:
	修改pwm频率： 30Hz
	TIM12:
	修改pwm频率： 300Hz
	TIM12:
	修改pwm频率： 3000Hz
	TIM12:
	修改pwm频率： 30000Hz
	TIM12:
	修改pwm频率： 300000Hz
	TIM12:
	修改pwm频率： 3000000Hz
	TIM12:
	修改pwm频率： 30000000Hz


	TIM12:
	期望pwm占空比： 66.599998%
	期望pwm频率： 3Hz
	实际pwm占空比： 66.599998%
	实际pwm占空比步幅： 0.001527%
	实际pwm频率： 2.999995Hz

	TIM3:
	期望pwm占空比： 50.000000%
	期望pwm频率： 7Hz
	实际pwm占空比： 50.000000%
	实际pwm占空比步幅： 0.001526%
	实际pwm频率： 6.999979Hz

	......

	TIM12:
	期望pwm占空比： 66.599998%
	期望pwm频率： 30000000Hz
	实际pwm占空比： 66.599998%
	实际pwm占空比步幅： 14.285714%
	实际pwm频率： 28571428.000000Hz

	TIM3:
	期望pwm占空比： 50.000000%
	期望pwm频率： 70000000Hz
	实际pwm占空比： 50.000000%
	实际pwm占空比步幅： 33.333332%
	实际pwm频率： 66666668.000000Hz
*/

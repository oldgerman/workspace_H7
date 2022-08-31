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

static pwmSet_InfoTypeDef pwmSetInfo_LPTIM1;
uint32_t lptim1_pwm_hz = 1;
float lptim1_pwm_DutyCycle = 50.0;

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
	printf("低功耗定时器LPTIM1 pwm调节测试:\r\n");
	printf("低功耗定时器时钟源 LSE: 32768Hz\r\n");
	printf("低功耗定时器通道对应输出引脚 PD13（没有引出2.54排针，对应开发板U3的Pin7）\r\n");
	printf("操作提示:\r\n");
	printf("1. KEY A 长按或连续长按，以2倍增量修改LPTIM1 pwm频率\r\n");
	printf("2. KEY B 短按以1%%，长按或连续长按以10%%增量修改LPTIM1 pwm占空比\r\n");
	printf("3. KEY A 短按以应用修改\r\n");
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



void btA_CLICKED_func(){
	pwmSetInfo_LPTIM1 = bsp_LPTIMx_PWM_Set(&hlptim1, 32768, lptim1_pwm_hz, lptim1_pwm_DutyCycle);
	bsp_LPTIMx_PWM_En(&hlptim1, true);
	Printf_pwmSetInfo_TIMx(&pwmSetInfo_LPTIM1);
}



void btB_CLICKED_func(){
	lptim1_pwm_DutyCycle += 1.0;
	if(lptim1_pwm_DutyCycle > 100){
		lptim1_pwm_DutyCycle = 0;
	}
	printf("修改pwm占空比： %f%%\r\n", lptim1_pwm_DutyCycle);
}

void btA_LONG_PRESSED_func(){
	static uint8_t cnt = 0;
	++cnt;
	lptim1_pwm_hz = 1 << cnt;
	if(cnt > 14){
		lptim1_pwm_hz = 1;
		cnt = 0;
	}
	printf("修改期望pwm频率： %ldHz\r\n", lptim1_pwm_hz);
}

void btB_LONG_PRESSED_func(){
	lptim1_pwm_DutyCycle += 10.0;
	if(lptim1_pwm_DutyCycle > 100){
		lptim1_pwm_DutyCycle = 0;
	}
	printf("修改pwm占空比： %f%%\r\n", lptim1_pwm_DutyCycle);
}

void setup(){
	bsp_Init();
	PrintfInfo();
	PrintfHelp();
	bsp_tim6_enable_IT();	//指示系统正在运行
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
低功耗定时器LPTIM1 pwm调节测试:

低功耗定时器时钟源 LSE: 32768Hz
低功耗定时器通道对应输出引脚 PD13（没有引出2.54排针，对应开发板U3的Pin7）
操作提示:
1. KEY A 长按或连续长按，以2倍增量修改LPTIM1 pwm频率
2. KEY B 短按以1%，长按或连续长按以10%增量修改LPTIM1 pwm占空比
3. KEY A 短按以应用修改

修改期望pwm频率： 2Hz
修改期望pwm频率： 4Hz
修改期望pwm频率： 8Hz
......
修改期望pwm频率： 16384Hz


修改pwm占空比： 51.000000%
修改pwm占空比： 52.000000%
修改pwm占空比： 53.000000%
......
期望pwm占空比： 75.000000%

......
期望pwm占空比： 50.000000%
期望pwm频率： 512Hz
实际pwm占空比： 50.000000%
实际pwm占空比步幅： 1.562500%
实际pwm频率： 512.000000Hz
......
期望pwm频率： 512Hz
实际pwm占空比： 75.000000%
实际pwm占空比步幅： 1.562500%
实际pwm频率： 512.000000Hz
*/

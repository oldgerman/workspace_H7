/*
 * cppports.cpp
 *
 *  Created on: May 16, 2022
 *      Author: OldGerman
 */

#include "cppports.h"
#include "bsp.h"
#include "binary.h"
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

// LPTIM2 PWM频率
static uint32_t lptim2_pwm_hz = 1000;		//	默认1KHz
static float lptim2_pwm_DutyCycle = 50.0;	// 默认50%
static pwmSet_InfoTypeDef pwmSetInfo_LPTIM2;

static void Printf_pwmSetInfo_TIMx(pwmSet_InfoTypeDef *pwmSetInfo)
{
	printf("期望pwm占空比： %f%%\r\n", pwmSetInfo->pwm_Dutycycle_Expect);
	printf("期望pwm频率： %ldHz\r\n", pwmSetInfo->pwm_Frequency_Expect);
	printf("实际pwm占空比： %f%%\r\n", pwmSetInfo->pwm_Dutycycle);
	printf("实际pwm占空比步幅： %f%%\r\n", pwmSetInfo->pwmStep_Dutycycle);
	printf("实际pwm频率： %fHz\r\n", pwmSetInfo->pwm_Frequency);
	printf("\r\n");
}

/* 任意GPIO PWM脉冲数据 */
#define bitLVL_NUM 2
uint8_t bitLVL[bitLVL_NUM] = {
		B10101100,	//以bit指定一串电平
		B11110000
};

/*
	STM32CubeIDE不支持Keil的写法 __attribute__((at()))
	需要改用__attribute__((section()))
*/
//将array_test_SRAM4放到SRAM4中测试编译OK
//注意SECTION名字不要出现bss text等
//uint8_t array_test_SRAM4[1024] __attribute__((section(".RAM_D3_Array")));

uint32_t IO_Toggle[bitLVL_NUM * 8] __attribute__((section(".RAM_D3_Array")));


void bsp_Lptim_DMA_PWM_XferCpltCallback(DMA_HandleTypeDef * hdma){
    /*
     * armfly V7：41.2.4 BDMA中断处理
       1、传输完成开始使用DMA缓冲区的前半部分，此时可以动态修改后半部分数据
          比如缓冲区大小是IO_Toggle[0] 到 IO_Toggle[7]
          那么此时可以修改IO_Toggle[4] 到 IO_Toggle[7]
       2、变量所在的SRAM区已经通过MPU配置为WT模式，更新变量IO_Toggle会立即写入。
       3、不配置MPU的话，也可以通过Cahce的函数SCB_CleanDCache_by_Addr做Clean操作。
    */
	if(hdma == &hdma_bdma_generator0){
		bsp_Lptim_DMA_convertLevelToBSRR(&bitLVL[1], 1, &IO_Toggle[8], LPTIM_DMA_PWM_Pin);
	}
}

void bsp_Lptim_DMA_PWM_XferHalfCpltCallback(DMA_HandleTypeDef * hdma){
    /*
     * armfly V7：41.2.4 BDMA中断处理
       1、半传输完成开始使用DMA缓冲区的后半部分，此时可以动态修改前半部分数据
          比如缓冲区大小是IO_Toggle[0] 到 IO_Toggle[7]
          那么此时可以修改IO_Toggle[0] 到 IO_Toggle[3]
       2、变量所在的SRAM区已经通过MPU配置为WT模式，更新变量IO_Toggle会立即写入。
       3、不配置MPU的话，也可以通过Cahce的函数SCB_CleanDCache_by_Addr做Clean操作。
    */
	if(hdma == &hdma_bdma_generator0){
		bsp_Lptim_DMA_convertLevelToBSRR(&bitLVL[0], 1, &IO_Toggle[0], LPTIM_DMA_PWM_Pin);
	}
}


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
	printf("BDMA应用之控制任意IO做PWM和脉冲数控制:\r\n");
	printf("BDMA请求生成信号 为LPTIM2 ，LPTIM2时钟源频率：100MHz，默认PWM频率1KHz，占空比50%%\r\n");
	printf("BDMA + LPTIM2 控制的IO为PE0，需要使用示波器测量");
	printf("LPTIM本身输出的PWM引脚为PB13（虽然没有必要让LPTIM2 生成的PWM输出到io，但还是启用以方便测试）");
	printf("操作提示:\r\n");
	printf("1. KEY A 长按或连续长按，以2倍增量修改LPTIM2 pwm频率，短按以应用PWM修改，并打开PWM\r\n");
	printf("2. KEY B 长按或连续长按以10%%增量修改LPTIM2 pwm占空比，短按关闭PWM\r\n");
	printf("3. 串口输入A，设置波形数据前8位\r\n");
	printf("4. 串口输入B，设置波形数据后8位\r\n");
	printf("\r\n");
}

void btA_CLICKED_func(){
	printf("KEY A 短按，打开 LPTIM2 PWM\r\n");
	pwmSetInfo_LPTIM2 = bsp_Lptim_DMA_PWM_Set(&hlptim2, 100000000, lptim2_pwm_hz, lptim2_pwm_DutyCycle);
	bsp_Lptim_DMA_PWM_En(&hlptim2, true);
	Printf_pwmSetInfo_TIMx(&pwmSetInfo_LPTIM2);
}

void btB_CLICKED_func(){
	printf("KEY B 短按，关闭 LPTIM2 PWM\r\n");
	bsp_Lptim_DMA_PWM_En(&hlptim2, false);
}

void btA_LONG_PRESSED_func(){
	lptim2_pwm_DutyCycle += 10.0;
	if(lptim2_pwm_DutyCycle > 100){
		lptim2_pwm_DutyCycle = 0;
	}
	printf("KEY A 长按 修改期望pwm占空比： %f%%\r\n", lptim2_pwm_DutyCycle);
}

void btB_LONG_PRESSED_func(){
	static uint8_t cnt = 0;
	++cnt;
	lptim2_pwm_hz = 1 << cnt;
	if(cnt > 24){
		lptim2_pwm_hz = 1;
		cnt = 0;
	}
	printf("KEY A 长按 修改期望pwm频率： %ldHz\r\n", lptim2_pwm_hz);
}


void setup(){
	bsp_Init();
	PrintfInfo();
	PrintfHelp();

	bsp_tim6_enable_IT();	//指示系统正在运行

	bsp_Lptim_DMA_convertLevelToBSRR(bitLVL, bitLVL_NUM, IO_Toggle, LPTIM_DMA_PWM_Pin);
	bsp_Lptim_DMA_DMA_Set(
			&hdma_bdma_generator0, (uint32_t)IO_Toggle, (uint32_t)&LPTIM_DMA_PWM_GPIO_Port->BSRR, sizeof(IO_Toggle) / sizeof(IO_Toggle[0]),
			bsp_Lptim_DMA_PWM_XferCpltCallback,
			bsp_Lptim_DMA_PWM_XferHalfCpltCallback);
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
    uint8_t ucCount = 0, i;
    uint8_t buf[128];
    bool setFirstHalf = 0;
	while(1) {
		static uint32_t timeOld = HAL_GetTick();
		if(waitTime(&timeOld, 10)){
			bsp_Button_Update();
			if (comGetChar(COM1, &read))
			{
				switch (ucStatus)
				{
				/* 状态0保证接收到A或B */
				case ucStatus_waitCmd:
					if(read == 'A')
					{
						printf("设置波形数据前8位，请输入形如 01001110 的数据\r\n");
						ucStatus = ucStatus_setFirstHalf;
					}
					else if(read == 'B')
					{
						printf("设置波形数据后8位，请输入形如 01001110 的数据\r\n");
						ucStatus = ucStatus_setSecondHalf;
					}
					break;

				case ucStatus_setFirstHalf:
					setFirstHalf = true;
					ucStatus = ucStatus_readBitsChar;
					break;

				case ucStatus_setSecondHalf:
					ucStatus = ucStatus_readBitsChar;
					setFirstHalf = false;
					break;

				case ucStatus_readBitsChar:
					buf[ucCount] = read;

					/* 接收够8个数据 */
					bool bitCharError = false;
					if(ucCount == 8)
					{
						/* 打印接收到的数据值 */
						printf("接收到的数据：");
						for(i = 0; i < ucCount; i++)
						{
							*(buf + i) -= '0';	//	字符转整形
							if(buf[i] > 1 || buf[i] < 0)	//检测数据合法性
								bitCharError = true;
							printf("%d ", buf[i]);
						}
						if(bitCharError){
							printf("输入数据无效，请重新输入\r\n");
							ucStatus = ucStatus_waitCmd;
						}
						else{
							uint8_t bits = B00000000;
							for(i = 0; i < 8; i++)
								bits |= buf[7 - i] << i;

							if(setFirstHalf){
								bitLVL[0] = bits;
							}else{
								bitLVL[1] = bits;
							}
							printf("输入数据有效，已更改PWM波形数据\r\n");
							ucStatus = ucStatus_waitCmd;
						}
						ucStatus = 0;
						ucCount = 0;
					}
					else
					{
						ucCount++;
					}
					break;
				}
			}
		}
	}
}
/* Demo:

*/

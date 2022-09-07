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

// tim12 PWM频率
static uint32_t tim12_pwm_hz = 1000;		//	默认1KHz
static float tim12_pwm_DutyCycle = 50.0;	// 默认50%
static pwmSet_InfoTypeDef pwmSetInfo_tim12;

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
// 缓冲区0
uint8_t bitLVL_M0[bitLVL_NUM] = {	//以bit指定一串电平
		B10101100,
		B11110000
};
// 缓冲区1
uint8_t bitLVL_M1[bitLVL_NUM] = {	//以bit指定一串电平
		B11101110,
		B11001100
};

/*
	STM32CubeIDE不支持Keil的写法 __attribute__((at()))
	需要改用__attribute__((section()))
*/
//将array_test_SRAM4放到SRAM4中测试编译OK
//注意SECTION名字不要出现bss text等
//uint8_t array_test_SRAM4[1024] __attribute__((section(".RAM_D3_Array")));

// 缓冲区0
uint32_t IO_Toggle_M0[bitLVL_NUM * 8] __attribute__((section(".RAM_D3_Array")));
// 缓冲区1
uint32_t IO_Toggle_M1[bitLVL_NUM * 8] __attribute__((section(".RAM_D3_Array")));

//回调函数示例模板
void bsp_Tim_DMA_PWM_XferCpltCallback(DMA_HandleTypeDef * hdma){
	/* 当前使用的缓冲 0 */
	/*
	 	armfly V7：43.2.4 DMA中断处理
		1、当前正在使用缓冲 1，此时可以动态修改缓冲 0 的数据。 比如缓冲区 0 是 IO_Toggle，缓冲区 1 是 IO_Toggle1，那么此时就可以修改 IO_Toggle。
		2、变量所在的 SRAM 区已经通过 MPU 配置为 WT 模式，更新变量 IO_Toggle 会立即写入。
		3、不配置 MPU 的话，也可以通过 Cahce 的函数 SCB_CleanDCache_by_Addr 做 Clean 操作。
	*/
	if(hdma == &hdma_dma_generator0){
		bsp_convertLevelToBSRR(bitLVL_M0, bitLVL_NUM, IO_Toggle_M0, TIM_DMA_PWM_Pin);
	}
}

void bsp_Tim_DMA_PWM_XferM1CpltCallback(DMA_HandleTypeDef * hdma){
	/* 当前使用的缓冲 1 */
	/*
	    armfly V7：43.2.4 DMA中断处理
		1、当前正在使用缓冲 0，此时可以动态修改缓冲 1 的数据。 比如缓冲区 0 是 IO_Toggle，缓冲区 1 是 IO_Toggle1，那么此时就可以修改 IO_Toggle1。
		2、变量所在的 SRAM 区已经通过 MPU 配置为 WT 模式，更新变量 IO_Toggle 会立即写入。
		3、不配置 MPU 的话，也可以通过 Cahce 的函数 SCB_CleanDCache_by_Addr 做 Clean 操作。
	*/
	if(hdma == &hdma_dma_generator0){
		bsp_convertLevelToBSRR(bitLVL_M1, bitLVL_NUM, IO_Toggle_M1, TIM_DMA_PWM_Pin);
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
	printf("DMA应用之双缓冲控制任意IO做PWM和脉冲数控制:\r\n");
	printf("DMA1请求生成信号 为tim12 CH1，默认PWM频率1KHz，占空比50%%\r\n");
	printf("DMA + tim12 控制的IO为PE0，需要使用示波器测量\r\n");
	printf("tim12 CH1在CubeMX配置为PWM Generation Output，输出的PWM引脚为PB14（虽然没有必要让tim12 生成的PWM输出到io，但还是启用以方便测试）\r\n");
	printf("操作提示:\r\n");
	printf("1. KEY A 长按或连续长按，以2倍增量修改tim12 pwm频率，短按以应用PWM修改，并打开PWM\r\n");
	printf("2. KEY B 长按或连续长按以10%%增量修改tim12 pwm占空比，短按关闭PWM\r\n");
	printf("3. 串口输入A，设置波形缓冲区0（16位）\r\n");
	printf("4. 串口输入B，设置波形缓冲区1（16位）\r\n");
	printf("\r\n");
}

void btA_CLICKED_func(){
	printf("KEY A 短按，打开 tim12 PWM\r\n");
	pwmSetInfo_tim12 = bsp_Tim_DMA_PWM_Set(&htim12, TIM_CHANNEL_1, tim12_pwm_hz, tim12_pwm_DutyCycle);
	bsp_Tim_DMA_PWM_En(&htim12, TIM_CHANNEL_1, true);
	Printf_pwmSetInfo_TIMx(&pwmSetInfo_tim12);
}

void btB_CLICKED_func(){
	printf("KEY B 短按，关闭 tim12 PWM\r\n");
	bsp_Tim_DMA_PWM_En(&htim12, TIM_CHANNEL_1, false);
}

void btA_LONG_PRESSED_func(){
	tim12_pwm_DutyCycle += 10.0;
	if(tim12_pwm_DutyCycle > 100){
		tim12_pwm_DutyCycle = 0;
	}
	printf("KEY A 长按 修改期望pwm占空比： %f%%\r\n", tim12_pwm_DutyCycle);
}

void btB_LONG_PRESSED_func(){
	static uint8_t cnt = 0;
	++cnt;
	tim12_pwm_hz = 1 << cnt;
	if(cnt > 24){
		tim12_pwm_hz = 1;
		cnt = 0;
	}
	printf("KEY A 长按 修改期望pwm频率： %ldHz\r\n", tim12_pwm_hz);
}


void setup(){
	bsp_Init();
	PrintfInfo();
	PrintfHelp();

	bsp_tim6_enable_IT();	//指示系统正在运行

	bsp_convertLevelToBSRR(bitLVL_M0, bitLVL_NUM, IO_Toggle_M0, TIM_DMA_PWM_Pin);
	bsp_convertLevelToBSRR(bitLVL_M1, bitLVL_NUM, IO_Toggle_M1, TIM_DMA_PWM_Pin);
	bsp_Tim_DMA_DMA_Set(
			&hdma_dma_generator0,
			(uint32_t)IO_Toggle_M0, (uint32_t)&TIM_DMA_PWM_GPIO_Port->BSRR, (uint32_t)IO_Toggle_M1,
			bitLVL_NUM * 8,
			bsp_Tim_DMA_PWM_XferCpltCallback,
			bsp_Tim_DMA_PWM_XferM1CpltCallback);
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
    uint16_t ucCount = 0, i;
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
						printf("设置波形数据前%d位，请输入形如 0100111011110000 的数据\r\n", bitLVL_NUM * 8);
						ucStatus = ucStatus_setFirstHalf;
					}
					else if(read == 'B')
					{
						printf("设置波形数据后%d位，请输入形如 0100111011110000 的数据\r\n", bitLVL_NUM * 8);
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

					/* 接收够16个数据 */
					bool bitCharError = false;
					if(ucCount == bitLVL_NUM * 8)
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
							uint8_t bits[bitLVL_NUM] = {0};
							for(i = 0; i < bitLVL_NUM * 8; i++){
								uint32_t j = i / 8;
								(*(bits + j)) |= buf[bitLVL_NUM * 8 - 1 - i] << (i - 8 * j);
							}
							for(i = 0; i < bitLVL_NUM; i++) {
								if(setFirstHalf){
									bitLVL_M0[i] = bits[bitLVL_NUM - 1 - i];
								}else{
									bitLVL_M1[i] = bits[bitLVL_NUM - 1 - i];
								}
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

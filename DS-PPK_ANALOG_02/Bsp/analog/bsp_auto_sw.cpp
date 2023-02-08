/*
 * bsp_auto_sw[bs].cpp
 *
 *  Created on: Jan 31, 2023
 *      Author: PSA
 */

#include "bsp_analog.h"

static volatile uint32_t bs = 0;
static volatile uint32_t swx_old = 0;
static volatile uint32_t cs = 0;

osThreadId_t autoSwInitTaskHandle;
const uint32_t autoSwInitTaskStackSize = 256 * 4;
const osThreadAttr_t autoSwInitTask_attributes = {
    .name = "autoSwInitTask",
    .stack_size = autoSwInitTaskStackSize,
    .priority = (osPriority_t) osPriorityLow,
};

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if ( GPIO_Pin == SW1_Pin ||  GPIO_Pin == SW2_Pin ||
			GPIO_Pin == SW3_Pin ||  GPIO_Pin == SW4_Pin)
	{
		bs = !(timestamp.bs);			//当前使用的缓冲区
		cs = timestamp.auto_sw[bs].cs;	//当前auto_sw 缓冲区的游标

		// 记录时间戳
		timestamp.auto_sw[bs].time[cs] =  bsp_timestamp_get();

		/* 继承上一次档位的io电平状态
		 * 有两种情况，假设是前半缓冲区
		 *  情况1. 第一次进入 前半缓冲区，上一次档位需用 后半缓冲区 的最后一次数据
		 *  情况2. 第二次进入 前半缓冲区，上一次档位需用 前半缓冲区 的上一次数据
		 */
#if 1
		//等效 #else 的 情况1、情况2
		timestamp.auto_sw[bs].range[cs].swx = swx_old;
#else
		//情况1
		if(cs == 0)
		{
			timestamp.auto_sw[bs].range[cs].swx =
					swx_old;
		}
		//情况2
		else
		{
			timestamp.auto_sw[bs].range[cs].swx =
					timestamp.auto_sw[bs].range[cs - 1].swx;
			//						  ^~~       ^~~~~~
		}
#endif

		// 记录档位IO电平
		if ( GPIO_Pin == SW1_Pin) {
			//check pin state
			uint8_t(HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin)) ?
					(timestamp.auto_sw[bs].range[cs].sw1 = 1) :
					(timestamp.auto_sw[bs].range[cs].sw1 = 0);
		}
		else if ( GPIO_Pin == SW2_Pin) {
			//check pin state
			uint8_t(HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin)) ?
					(timestamp.auto_sw[bs].range[cs].sw2 = 1) :
					(timestamp.auto_sw[bs].range[cs].sw2 = 0);
		}
		else if ( GPIO_Pin == SW3_Pin) {
			//check pin state
			uint8_t(HAL_GPIO_ReadPin(SW3_GPIO_Port, SW3_Pin)) ?
					(timestamp.auto_sw[bs].range[cs].sw3 = 1) :
					(timestamp.auto_sw[bs].range[cs].sw3 = 0);
		}
		else if ( GPIO_Pin == SW4_Pin) {
			//check pin state
			uint8_t(HAL_GPIO_ReadPin(SW4_GPIO_Port, SW4_Pin)) ?
					(timestamp.auto_sw[bs].range[cs].sw4 = 1) :
					(timestamp.auto_sw[bs].range[cs].sw4 = 0);
		}

		if(timestamp.auto_sw[bs].cs < (adc1_adc3_buffer_size / 2))	//有效范围检查，避免缓冲区游标越界访问
		{
			swx_old = timestamp.auto_sw[bs].range[cs].swx; 			//保存当前swx，做继承换挡数据备用
			timestamp.auto_sw[bs].cs++;								//增加游标
		}
	}
}


static void threadAutoSwInit(void* argument)
{
	/*先得关闭SMU*/
	bsp_smu_set_en(0);
	uint8_t ret = 0;
	for(;;)
	{
		ret += (HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin) == GPIO_PIN_SET);
		ret += (HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin) == GPIO_PIN_SET);
		ret += (HAL_GPIO_ReadPin(SW3_GPIO_Port, SW3_Pin) == GPIO_PIN_SET);
		ret += (HAL_GPIO_ReadPin(SW4_GPIO_Port, SW4_Pin) == GPIO_PIN_SET);

		/* 直到所有的SW都是高电平 */
		if(ret == 4) {

			/* 初始化时间戳 */
			bsp_timestamp_init();

			/* 设置前半缓冲区的第一个元素为 0b00001111，即当前 sw1~sw4 引脚电平状态*/
			timestamp.auto_sw[0].range[0].swx = 0x0f;

			/* 清除所有SW1~SW4中断标志 */
			__HAL_GPIO_EXTI_CLEAR_FLAG(SW1_Pin | SW2_Pin | SW3_Pin | SW4_Pin);
			/* 打开SMU，此时立即产生换挡中断 */
			bsp_smu_set_en(1);
			break;
		}else
			osDelay(500);
	}
	printf("[autoSwInitTask]: Task deleted");
	osThreadTerminate(autoSwInitTaskHandle);	//删除 auto sw 初始化任务
}
void bsp_auto_sw_init()
{
	//创建线程
    autoSwInitTaskHandle = osThreadNew(threadAutoSwInit, nullptr, &autoSwInitTask_attributes);

}

/*
 * bsp_auto_sw.cpp
 *
 *  Created on: Jan 31, 2023
 *      Author: PSA
 */

#include "bsp_analog.h"

volatile auto_sw_data_t auto_sw_data;

osThreadId_t autoSwInitTaskHandle;
const uint32_t autoSwInitTaskStackSize = 256 * 4;
const osThreadAttr_t autoSwInitTask_attributes = {
    .name = "autoSwInitTask",
    .stack_size = autoSwInitTaskStackSize,
    .priority = (osPriority_t) osPriorityLow,
};

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if ( GPIO_Pin == SW1_Pin) {
		//check pin state
		uint8_t(HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin)) ? (auto_sw_data.sw1 = 1) : (auto_sw_data.sw1 = 0);
	}
	else if ( GPIO_Pin == SW2_Pin) {
		//check pin state
		uint8_t(HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin)) ? (auto_sw_data.sw2 = 1) : (auto_sw_data.sw2 = 0);
	}
	else if ( GPIO_Pin == SW3_Pin) {
		//check pin state
		uint8_t(HAL_GPIO_ReadPin(SW3_GPIO_Port, SW3_Pin)) ? (auto_sw_data.sw3 = 1) : (auto_sw_data.sw3 = 0);
	}
	else if ( GPIO_Pin == SW4_Pin) {
		//check pin state
		uint8_t(HAL_GPIO_ReadPin(SW4_GPIO_Port, SW4_Pin)) ? (auto_sw_data.sw4 = 1) : (auto_sw_data.sw4 = 0);
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
			/* SWx电平数据置为 1111 */
			auto_sw_data.swx = 0x0f;
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

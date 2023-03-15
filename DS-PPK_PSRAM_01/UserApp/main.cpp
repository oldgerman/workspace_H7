/**
  ******************************************************************************
  * @file        main.cpp
  * @author      OldGerman
  * @created on  Jan 7, 2023
  * @brief
  ******************************************************************************
  * @attention
  *
  * Copyright (C) 2022 OldGerman.
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see https://www.gnu.org/licenses/.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "common_inc.h"
#include "arm_math.h"
#include "tim.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
const uint32_t ledTaskStackSize = 256 * 4;
const osThreadAttr_t ledTask_attributes = {
    .name = "ledTask",
    .stack_size = ledTaskStackSize,
    .priority = (osPriority_t) osPriorityNormal,
};

/* Private constants ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
osThreadId_t ledTaskHandle;

/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Function implementations --------------------------------------------------*/

/* Thread definitions */
void threadLedUpdate(void* argument){
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = 1000;
	/* 获取当前的系统时间 */
	xLastWakeTime = xTaskGetTickCount();

	for(;;){
		/* 翻转开发板引脚 */
//		HAL_GPIO_TogglePin(VOUT_EN_GPIO_Port, VOUT_EN_Pin);

		/* 打印时间节拍 */
//		printf("[led_task] sysTick : %ld ms\r\n", xTaskGetTickCount());

		/* arm math 单精度硬件浮点测试 */
//		float data[3];
//		data[0] = arm_sin_f32(3.1415926/6);	// sin(PI/6)
//		data[1] = arm_sin_f32(3.1415926/1);	// sin(PI/1)
//		data[2] = arm_sin_f32(3.1415926/3);	// sin(PI/3)
//		printf("[sin] 30°= %.6f, 45°= %.6f, 60°= %.6f\r\n", data[0], data[1], data[2]);

		vTaskDelayUntil(&xLastWakeTime, xFrequency);
	}
}

void ledUpdateInit()
{
	ledTaskHandle = osThreadNew(threadLedUpdate, nullptr, &ledTask_attributes);
}

void Main()
{
    /* 初始化一些通信，USB-CDC/VCP/WIFI等 */
    InitCommunication();

    /* 初始化LED时间片任务 */
    ledUpdateInit();
}

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
#include "i2c.h"
#include "bq25601.h"
#include "cw2015_battery.h"
#include "I2C_Wrapper.h"
#include "bsp_analog.h"
#include "dac.h"
#include "arm_math.h"
#include "frame_processor.h"
#include "bsp_logic.h"
#include "bsp_auto_sw.h"
#include "bsp_smu.h"
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

void i2c_scaner(I2C_HandleTypeDef *hi2c, uint8_t i2cBusNum) {
	uint8_t i = 0;
	HAL_StatusTypeDef status;
	printf("MCU: i2c%d scan...\r\n",i2cBusNum);

	for (i = 0; i < 128; i++) {
		status = HAL_I2C_IsDeviceReady(hi2c, (uint16_t)(i<<1), 1, 10);
		if (status == HAL_OK) {
			printf("addr: 0x%02X is ok\r\n",i);
		} else if (status == HAL_TIMEOUT) {
			printf("addr: 0x%02X is timeout\r\n",i);
		} else if (status == HAL_BUSY) {
			printf("addr: 0x%02X is busy\r\n",i);
		}
	}
}

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
//		i2c_scaner(&hi2c1,1);
//		i2c_scaner(&hi2c2,2);

		/* arm math 单精度硬件浮点测试 */
//		float data[3];
//		data[0] = arm_sin_f32(3.1415926/6);	// sin(PI/6)
//		data[1] = arm_sin_f32(3.1415926/1);	// sin(PI/1)
//		data[2] = arm_sin_f32(3.1415926/3);	// sin(PI/3)
//		printf("[sin] 30°= %.6f, 45°= %.6f, 60°= %.6f\r\n", data[0], data[1], data[2]);

		charging_hw_update();

		bsp_adc2GetValues();
//		bsp_adc3GetValues();

		static uint32_t adc_callback_cnt_old = 0;
		printf("[adc_callback_cnt] %.lu times 1s\r\n", adc_callback_cnt - adc_callback_cnt_old);
		adc_callback_cnt_old = adc_callback_cnt;

		vTaskDelayUntil(&xLastWakeTime, xFrequency);
	}
}

void ledUpdateInit()
{
	ledTaskHandle = osThreadNew(threadLedUpdate, nullptr, &ledTask_attributes);
}

void Main()
{
	/* 初始化具有互斥锁的I2C对象 */
	FRToSI2C2.FRToSInit();

    /* 初始化一些通信，USB-CDC/VCP/WIFI等 */
    InitCommunication();

    /* 初始化LED时间片任务 */
    ledUpdateInit();

    /* 初始化电源管理芯片 */
	charging_driver_probe();

	/* 初始化电量计芯片 */
    cw_bat_init();


	/* 初始化协议帧处理器，务必放在ADC之前 */
	frame_processor_init();

    /* 初始化测量电路的外设、外围电路 */
    bsp_smuEnablePowerChips(true);                                /* 打开测量电路的电源 */
	HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 0);   /* 设置仪表放大器的VREF引脚电压为0V */
	HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);
	HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_2, DAC_ALIGN_12B_R, 2007); /* VLDO 输出3.30V */
	HAL_DAC_Start(&hdac1, DAC_CHANNEL_2);
	bsp_adc2Init();
	bsp_adc1Init();
	bsp_adc3Init();
	bsp_auto_sw_init();
	bsp_logicInit();

	/* 打开统计CPU利用率的定时器 */
	HAL_TIM_Base_Start_IT(&htim7);
}

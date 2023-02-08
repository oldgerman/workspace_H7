/**
  ******************************************************************************
  * @file           : main.cpp
  * @brief          :
  ******************************************************************************
  * @Created on		: Jan 7, 2023
  * @Author 		: OldGerman
  * @attention		:
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
/* Private define ------------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Global constant data ------------------------------------------------------*/
const uint32_t ledTaskStackSize = 256 * 4;
/* Global variables ----------------------------------------------------------*/
/* Private constant data -----------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
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
osThreadId_t ledTaskHandle;
void ThreadLedUpdate(void* argument){
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = 1000;
	/* 获取当前的系统时间 */
	xLastWakeTime = xTaskGetTickCount();
	charging_driver_probe();
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
		bsp_adc3GetValues();

		vTaskDelayUntil(&xLastWakeTime, xFrequency);
	}
}

void Main(){
	// 创建并释放FRToSI2C2的信号量
	FRToSI2C2.FRToSInit();

    // Init all communication staff, include USB-CDC/VCP/UART/CAN etc.
    InitCommunication();

    const osThreadAttr_t ledTask_attributes = {
        .name = "ledTask",
        .stack_size = ledTaskStackSize,
        .priority = (osPriority_t) osPriorityNormal,
    };
    ledTaskHandle = osThreadNew(ThreadLedUpdate, nullptr, &ledTask_attributes);

    cw_bat_init();

    /* INA_VREF = 0V */
	HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 0);
	HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);

    /* VLDO 输出5.00V */
	HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_2, DAC_ALIGN_12B_R, 751);
	HAL_DAC_Start(&hdac1, DAC_CHANNEL_2);

	bsp_adc2Init();

	frame_processor_init();
	bsp_auto_sw_init();

	bsp_adc1Init();
	bsp_adc3Init();

}

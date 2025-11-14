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

/* Thread definitions */
osThreadId_t ledTaskHandle;
void ThreadLedUpdate(void* argument){
	TickType_t xLastWakeTime = xTaskGetTickCount();
	const TickType_t xFrequency = 500;

	/* 获取当前的系统时间 */
	xLastWakeTime = xTaskGetTickCount();
	for(;;){
		/* 翻转开发板红色LED */
//		HAL_GPIO_TogglePin(LRGB_R_GPIO_Port, LRGB_R_Pin);

		/* 打印时间节拍 */
		printf("[led_task] sysTick : %ld ms\r\n", xTaskGetTickCount());
		vTaskDelayUntil(&xLastWakeTime, xFrequency);
	}
}

void Main(){
    // Init all communication staff, include USB-CDC/VCP/UART/CAN etc.
    InitCommunication();

    const osThreadAttr_t ledTask_attributes = {
        .name = "ledTask",
        .stack_size = ledTaskStackSize,
        .priority = (osPriority_t) osPriorityNormal,
    };
    ledTaskHandle = osThreadNew(ThreadLedUpdate, nullptr, &ledTask_attributes);
}

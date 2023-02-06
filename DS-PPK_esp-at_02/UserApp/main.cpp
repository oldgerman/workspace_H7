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

using namespace ns_frtos_spi;
using namespace ns_frtos_spi_esp_at;

/* FRTOS_SPIBase类对象：SPI2_Base */
RAM_REGION_NO_CACHE uint8_t SPI2_RxBuf[FRTOS_SPIBase::sizeCmdOnly];
RAM_REGION_NO_CACHE uint8_t SPI2_TxBuf[FRTOS_SPIBase::sizeCmdOnly];

FRTOS_SPIBase SPI2_Base(hspi2, SPI2_TxBuf, SPI2_RxBuf, FRTOS_SPIBase::sizeCmdOnly);

const uint32_t SPI2_Cmd_PSC  = SPI_BAUDRATEPRESCALER_4;		//40MHz
//const uint32_t SPI2_Cmd_PSC  = SPI_BAUDRATEPRESCALER_16;	//10MHz
//const uint32_t SPI2_Cmd_PSC  = SPI_BAUDRATEPRESCALER_64;	//2.5MHz

FRTOS_SPICmd SPI2_Cmd(
		&SPI2_Base,
		USR_SPI_CS_GPIO_Port,
		USR_SPI_CS_Pin,
		SPI2_Cmd_PSC,
		SPI_PHASE_1EDGE,
		SPI_POLARITY_LOW);

void esp_at_init(){
	FRTOS_SPIDev_ESP_AT::init(
			&SPI2_Cmd,
			GPIO_RESET_GPIO_Port,
			GPIO_RESET_Pin,
			osPriorityHigh);
}

/* Thread definitions */
osThreadId_t ledTaskHandle;
void ThreadLedUpdate(void* argument){
	TickType_t xLastWakeTime = xTaskGetTickCount();
	const TickType_t xFrequency = 1000;

	/* 获取当前的系统时间 */
	xLastWakeTime = xTaskGetTickCount();
	for(;;){
		/* 翻转开发板红色LED */
		HAL_GPIO_TogglePin(LRGB_R_GPIO_Port, LRGB_R_Pin);

		/* 打印时间节拍 */
//		printf("[led_task] sysTick : %ld ms\r\n", xTaskGetTickCount());
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

	esp_at_init();

}

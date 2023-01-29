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

typedef enum {
	/* FUN			  		CHx */
	MUX_FUN_CAL_RES_500mR 	= 6,
	MUX_FUN_CAL_RES_50R 	= 1,
	MUX_FUN_CAL_RES_500R 	= 0,
	MUX_FUN_CAL_RES_5KR 	= 3,
	MUX_FUN_CAL_RES_50KR 	= 4,
	MUX_FUN_CAL_RES_500KR 	= 7,
	MUX_FUN_DPDT_7222_S 	= 2,
	MUX_FUN_NC 				= 5,
}mux_fun_t;

static void mux_FunSet(mux_fun_t sLinesCode)
{
	/**
	 * CBA	CHx
	 * 000	0
	 * 001	1
	 * 010	2
	 * 011	3
	 * 100	4
	 * 101	5
	 * 110	6
	 * 111	7
	 */
	if(sLinesCode < 8) {
		HAL_GPIO_WritePin(MUX_C_GPIO_Port, MUX_C_Pin, (GPIO_PinState)((sLinesCode & 4) >> 2));
		HAL_GPIO_WritePin(MUX_B_GPIO_Port, MUX_B_Pin, (GPIO_PinState)((sLinesCode & 2) >> 1));
		HAL_GPIO_WritePin(MUX_A_GPIO_Port, MUX_A_Pin, (GPIO_PinState)((sLinesCode & 1) >> 0));
	}
}

static void mux_FunTest()
{
	static uint8_t cnt_mux = 0;
	static mux_fun_t sLinesCode = MUX_FUN_NC;
	switch (cnt_mux) {
		case 0:
			sLinesCode = MUX_FUN_CAL_RES_500KR;
			break;
		case 1:
			sLinesCode = MUX_FUN_CAL_RES_50KR;
			break;
		case 2:
			sLinesCode = MUX_FUN_CAL_RES_500mR;
			break;
		case 3:
			sLinesCode = MUX_FUN_CAL_RES_5KR;
			break;
		case 4:
			sLinesCode = MUX_FUN_CAL_RES_500R;
			break;
		case 5:
			sLinesCode = MUX_FUN_CAL_RES_50R;
			break;
		case 6:
			sLinesCode = MUX_FUN_DPDT_7222_S;
			break;
		case 7:
			sLinesCode = MUX_FUN_NC;
			break;
		default:
			break;
	}
	mux_FunSet(sLinesCode);
	++cnt_mux;
	cnt_mux = cnt_mux % 8;
}

/* Thread definitions */
osThreadId_t ledTaskHandle;
void ThreadLedUpdate(void* argument){
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = 250;
	/* 获取当前的系统时间 */
	xLastWakeTime = xTaskGetTickCount();
	charging_driver_probe();
	for(;;){
		/* 翻转开发板引脚 */
		HAL_GPIO_TogglePin(VOUT_EN_GPIO_Port, VOUT_EN_Pin);

		/* 打印时间节拍 */
//		printf("[led_task] sysTick : %ld ms\r\n", xTaskGetTickCount());
//		i2c_scaner(&hi2c1,1);
//		i2c_scaner(&hi2c2,2);
		charging_hw_update();
		vTaskDelayUntil(&xLastWakeTime, xFrequency);
//		mux_FunTest();
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
}

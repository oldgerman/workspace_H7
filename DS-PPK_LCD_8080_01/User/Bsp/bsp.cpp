/*
 * BSP.cpp
 *
 *  Created on: May 16, 2022
 *      Author: OldGerman
 */

#include "bsp.h"

void bsp_Init(){
	/* 复位ILI9488、FT6236、ESP32-C3*/
	HAL_GPIO_WritePin(CHIPS_RESET_GPIO_Port, CHIPS_RESET_Pin, GPIO_PIN_RESET);
	HAL_Delay(500);
	HAL_GPIO_WritePin(CHIPS_RESET_GPIO_Port, CHIPS_RESET_Pin, GPIO_PIN_SET);

	/* 打开AW9364背光，最大亮度 */
	HAL_GPIO_WritePin(BLK_GPIO_Port, BLK_Pin, GPIO_PIN_SET);

	LCD_InitHard();
	touch_init();

	bsp_Button_Init();
}

bool firstPwrOffToRUN = true;
/* @brief 在main()执行完MX_XXX_Init()函数们，到执行setup()之前插入的函数
 * @param None
 * @return None
 */
void preSetupInit(void){
	;
}


/* 非阻塞下等待固定的时间
 * @param timeOld 必须传入局部静态变量或全局变量
 * @param 等待时间
 * @return bool
 */
bool waitTime(uint32_t *timeOld, uint32_t wait) {
	uint32_t time = HAL_GetTick();
	if ((time - *timeOld) > wait) {
		*timeOld = time;
		return true;
	}
	return false;
}


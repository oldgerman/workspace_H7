/*
 * bsp_tim6.c
 *
 *  Created on: Aug 24, 2022
 *      Author: PSA
 */
#include "bsp.h"
#include "tim.h"
void bsp_tim6_Init(){
	HAL_TIM_Base_Start(&htim6);
	bsp_tim6_enable_IT();
}

void bsp_tim6_enable_IT(){
	__HAL_TIM_ENABLE_IT(&htim6, TIM_IT_UPDATE);
}

void bsp_tim6_disable_IT(){
	__HAL_TIM_DISABLE_IT(&htim6, TIM_IT_UPDATE);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim == &htim6)
	{
//		TIM6->SR = ~TIM_FLAG_UPDATE; //GPIOB->ODR ^= GPIO_PIN_1;
//		/* 使用通用 GPIO */ HC574_TogglePin(GPIO_PIN_23); /* 使用的 FMC 扩展 IO */
		HAL_GPIO_WritePin(LRGB_G_GPIO_Port, LRGB_G_Pin, !HAL_GPIO_ReadPin(LRGB_G_GPIO_Port, LRGB_G_Pin));
	}
}

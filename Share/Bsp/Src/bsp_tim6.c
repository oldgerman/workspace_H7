/*
 * bsp_tim6.c
 *
 *  Created on: Aug 24, 2022
 *      Author: OldGerman
 */
#include "bsp_config.h"
#ifdef EN_BSP_TIM6
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

/**
 * @brief   需要被重载HAL_TIM_PeriodElapsedCallback()调用
 * @@param  htim TIM handle
 * @retval  None
 */
void bsp_tim6_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim == &htim6)
	{
		HAL_GPIO_TogglePin(LRGB_G_GPIO_Port, LRGB_G_Pin);
//
//		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15);
//		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_8);
	}
}

#endif

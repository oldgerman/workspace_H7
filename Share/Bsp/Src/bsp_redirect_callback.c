/*
 * bsp_redirect_callback.c
 *
 *  Created on: Aug 30, 2022
 *      Author: OldGerman
 */

#include "bsp.h"
/* HAL库weak回调函数重定向
 * 根据CubeMX的配置，形如 HAL_XXX_MODULE_ENABLED 的宏会在stm32h7xx_hal_conf.h被定义
 */

#ifdef HAL_TIM_MODULE_ENABLED
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim){
#ifdef EN_BSP_TIMER
	bsp_Timer_TIM_OC_DelayElapsedCallback(htim);
#endif
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
#ifdef EN_BSP_TIM6
	bsp_tim6_TIM_PeriodElapsedCallback(htim);
#endif
}
#endif
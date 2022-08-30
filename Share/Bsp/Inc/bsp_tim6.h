/*
 * bsp_tim6.h
 *
 *  Created on: Aug 24, 2022
 *      Author: OldGerman
 *  @Notice tim6请在CubeMx中根据自己的喜好初始化中断的频率，并使能全局中断，优先级不必太高
 *  @Notice bsp_tim6_TIM_PeriodElapsedCallback()需要被重载HAL_TIM_PeriodElapsedCallback()调用
 *  		该函数每次执行都翻转PC3，即开发板原理图的LRGB_G，即RGB灯的绿色通道，以表示系统正在运行
 */

#ifndef BSP_INC_BSP_TIM6_H_
#define BSP_INC_BSP_TIM6_H_

void bsp_tim6_Init();
void bsp_tim6_enable_IT();
void bsp_tim6_disable_IT();
void bsp_tim6_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);

#endif /* BSP_INC_BSP_TIM6_H_ */

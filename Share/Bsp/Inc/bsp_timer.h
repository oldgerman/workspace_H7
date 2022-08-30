/*
 * bsp_timer.h
 *
 *  Created on: Aug 29, 2022
 *      Author: OldGerman
 *
 * @Notice 需要事先在CubeMX中选择TIM2~TIM5其中一个，的最多四个通道配置为Output Compare No Output
 * 		   需要使能该定时器的全局中断，优先级越高越好，其他CubeMX需要配置的参数保持默认值就行
 * @Notice 实现1us级精确延迟必须保证TIM2~TIM5的时钟源是200M
 * @Notice 运行时仅支持TIM2~TIM5其中一个，同一个定时器至多四个通道实现四路硬件延迟，可以使用
 * @Notice TIM2、TIM4是16位定时器，TIM2、TIM5是32bit定时器
 * 			对于16位定时器，最大 65.5ms
 * 			对于32位定时器，最大 4294秒
 * @Notice 如果想在回调函数s_TIM_CallBackX()里精确控制GPIO电平变换时间，最好配置Maximum Output Speed为Very High
 * @Notice bsp_Timer_TIM_OC_DelayElapsedCallback()需要被重载HAL_TIM_PWM_PulseFinishedCallback()调用
 */

#ifndef BSP_INC_BSP_TIMER_H_
#define BSP_INC_BSP_TIMER_H_

HAL_StatusTypeDef bsp_InitHardTimer(TIM_HandleTypeDef *htim);
HAL_StatusTypeDef bsp_StartHardTimer(uint32_t Channel, uint32_t uiTimeOut, void (*pCallBack)(void));
void bsp_Timer_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim);

#endif /* BSP_INC_BSP_TIMER_H_ */

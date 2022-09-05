/*
 * bsp_lptim_wkup.h
 *
 *  Created on: Aug 31, 2022
 *      Author: OldGerman
 *
 *  @Notice 需要事先在CubeMX中选择一个LPTIM（仅支持LPTIM1）
 *  		并在CubeMX时钟树中配置该LPTIM的时钟源为LSI或LSE，还要勾选LPTIM的NVIC中断，优先级不必太高，其他参数默认
 *	@notice 只有LPTIM1的中断能将H7从停机模式唤醒（前提要在NVIC中使能，且lptim时钟源是LSE、LSI或外部时钟（H7从停机模式时这些时钟不会关闭））
 *  @notice bsp_lptim_timeOut_LPTIM_CompareMatchCallback()需要被重载HAL_LPTIM_CompareMatchCallback()调用
 */

#ifndef BSP_INC_BSP_LPTIM_TIME_OUT_H_
#define BSP_INC_BSP_LPTIM_TIME_OUT_H_

HAL_StatusTypeDef bsp_LPTIMx_TimeOut_En(bool enable);
uint32_t bsp_LPTIMx_TimeOut_Set(
		LPTIM_HandleTypeDef *hlptim,
		uint32_t LptimClockFreq,
		uint32_t time,
		void (*pCallBack)(void));
void bsp_lptim_timeOut_LPTIM_CompareMatchCallback(LPTIM_HandleTypeDef *hlptim);
#endif /* BSP_INC_BSP_LPTIM_TIME_OUT_H_ */

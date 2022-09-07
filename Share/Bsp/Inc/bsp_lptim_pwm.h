/*
 * bsp_lptim_pwm.h
 *
 *  Created on: Aug 30, 2022
 *      Author: OldGerman
 *
 *  @Notice 需要事先在CubeMX中选择一个LPTIM（支持LPTIM1~5）、勾选Waveform Generation，输出极性为LOW
 *  		并在CubeMX时钟树中配置该LPTIM的时钟源，其他参数默认，无需勾选LPTIM的NVIC中断
 */

#ifndef BSP_INC_BSP_LPTIM_PWM_H_
#define BSP_INC_BSP_LPTIM_PWM_H_

#include "bsp_data structure.h"
#include "bsp_functions.h"

HAL_StatusTypeDef bsp_LPTIMx_PWM_En(LPTIM_HandleTypeDef *hlptim, bool enable);
pwmSet_InfoTypeDef bsp_LPTIMx_PWM_Set(
		LPTIM_HandleTypeDef *hlptim,
		uint32_t LptimClockFreq,
		uint32_t pwmFrequency,
		float pwmDutyCycle);

#endif /* BSP_INC_BSP_LPTIM_PWM_H_ */

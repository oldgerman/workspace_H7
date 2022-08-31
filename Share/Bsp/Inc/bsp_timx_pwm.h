/*
 * bsp_pwm.h
 *
 *  Created on: Aug 25, 2022
 *      Author: OldGerman
 */

#ifndef BSP_INC_BSP_TIMX_PWM_H_
#define BSP_INC_BSP_TIMX_PWM_H_

#include "bsp_data structure.h"

HAL_StatusTypeDef bsp_TIMx_PWM_En(TIM_HandleTypeDef* htim, uint32_t Channel, bool enable);
pwmSet_InfoTypeDef bsp_TIMx_PWM_Set(
		TIM_HandleTypeDef*
		htim, uint32_t Channel,
		uint32_t timBusCLK,
		float pwmDutyCycle);
#endif /* BSP_INC_BSP_TIMX_PWM_H_ */

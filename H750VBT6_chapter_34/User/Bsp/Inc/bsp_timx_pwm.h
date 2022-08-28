/*
 * bsp_pwm.h
 *
 *  Created on: Aug 25, 2022
 *      Author: OldGerman
 */

#ifndef BSP_INC_BSP_TIMX_PWM_H_
#define BSP_INC_BSP_TIMX_PWM_H_

void bsp_SetTIMxPWMCHx_OutPut();

typedef struct pwmSetInfo{
	float pwm_Dutycycle_Expect;		/* 期望的pwm占空比 */
	uint32_t pwm_Frequency_Expect;	/* 期望的pwm频率 */
	float pwm_Dutycycle;			/* 实际pwm占空比 */
	float pwmStep_Dutycycle;		/* 实际pwm占空比步幅 */
	float pwm_Frequency;			/* 实际pwm频率 */
}pwmSet_InfoTypeDef;

HAL_StatusTypeDef bsp_TIMx_PWM_En(TIM_HandleTypeDef* htim, uint32_t Channel, bool enable);
pwmSet_InfoTypeDef bsp_TIMx_PWM_Set(
		TIM_HandleTypeDef*
		htim, uint32_t Channel,
		uint32_t timBusCLK,
		float pwmDutyCycle);
#endif /* BSP_INC_BSP_TIMX_PWM_H_ */

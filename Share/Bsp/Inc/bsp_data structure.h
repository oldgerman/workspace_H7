/*
 * bsp_data structure.h
 *
 *  Created on: Aug 31, 2022
 *      Author: OldGerman
 *
 *  @brief BSP库公用结构体定义式声明
 */

#ifndef BSP_INC_BSP_DATA_STRUCTURE_H_
#define BSP_INC_BSP_DATA_STRUCTURE_H_

typedef struct pwmSetInfo{
//	uint32_t clkSourceFreq;			/* 时钟源频率，单位Hz */
	float pwm_Dutycycle_Expect;		/* 期望的pwm占空比 */
	uint32_t pwm_Frequency_Expect;	/* 期望的pwm频率 */
	float pwm_Dutycycle;			/* 实际pwm占空比 */
	float pwmStep_Dutycycle;		/* 实际pwm占空比步幅 */
	float pwm_Frequency;			/* 实际pwm频率 */
}pwmSet_InfoTypeDef;

#endif /* BSP_INC_BSP_DATA_STRUCTURE_H_ */

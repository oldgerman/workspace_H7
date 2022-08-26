/*
 * bsp_pwm.c
 *
 *  Created on: Aug 25, 2022
 *      Author: PSA
 */

#include "bsp.h"
//#include <math.h>

/**
  * @brief  将一个数字(浮点型)从一个范围重新映射到另一个区域
  * @param  x: 要映射的数字
  * @param  in_min: 值的当前范围的下界
  * @param  in_max: 值的当前范围的上界
  * @param  out_min: 值的目标范围的下界
  * @param  out_max: 值目标范围的上界
  * @retval 映射的值(double)
  */
static double fmap(double x, double in_min, double in_max, double out_min, double out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


static uint32_t binPower(uint8_t power){
	return 1 << power;
}


static void setPSC(uint32_t *ptrPSC, uint32_t *ptrARR, uint32_t pwmFrequency, uint32_t product_PSC_ARR){
	uint32_t ARR = *ptrARR;
	uint32_t PSC = *ptrPSC;

	for(uint16_t i = 1; ; i++){
//		PSC = binPower(i);	// log 2 N 时间复杂度
		PSC = i;			// N 时间复杂度, pwm=1Hz时，N最大1525
		ARR = product_PSC_ARR / PSC / pwmFrequency;		/* 下列计算时暂时不减去1，最后设置时才减去1 */
		if(ARR <= 65536){
			break;
		}
	}

	*ptrARR = ARR;
	*ptrPSC = PSC;
}

HAL_StatusTypeDef bsp_TIMx_PWM_En(TIM_HandleTypeDef* htim, uint32_t Channel, bool enable){
	HAL_TIM_Base_Start(htim);
	if(enable){
		if(HAL_TIM_PWM_Start(htim, Channel) != HAL_OK)
			return HAL_ERROR;
	}
	else{
		if( HAL_TIM_PWM_Stop(htim, Channel) != HAL_OK)
			return HAL_ERROR;
	}
	return HAL_OK;
}

/**
 * @brief  设置tim pwm的频率和占空比，会按照pwm偏好计算
 * @author OldGerman
 * @Date   2022/08/26
 * @param  htim:				htim句柄指针
 * @param  Channel:				tim的通道
 * @param  timBusCLK: 			htim所挂在总线的时钟频率				单位: Hz
 * @param  pwmFrequency: 		pwm频率，  范围 1 ~ timBusCLK	单位: Hz
 * @paeam  pwmDutyCycle: 		pwm占空比，范围 0.0... ~ 100.0...，	单位: %
 * @param  pwmPref: 			pwm偏好，侧重 频率步幅精度 或 占空比步幅精度
 * @retval pwmSet_InfoTypeDef 	经归并计算后的实际情况
 */
/* stm32 float: -2,147,483,648 ~ 2,147,483,647 */
pwmSet_InfoTypeDef bsp_TIMx_PWM_Set(
		TIM_HandleTypeDef* htim,
		uint32_t Channel,
		uint32_t timBusCLK,
		uint32_t pwmFrequency,
		float pwmDutyCycle,
		pwmSet_PrefTypeDef pwmSetPref)
{
	//关闭调节对通道的PWM
	HAL_TIM_PWM_Stop(htim, Channel);

	pwmSet_InfoTypeDef pwmSetInfo = {0};
	pwmSetInfo.pwmSetPref = pwmSetPref;				/* 保存pwm偏好 */

	uint32_t product_PSC_ARR = timBusCLK;			/* PSC和ARR的乘积总是等于TIM所挂在总线的时钟频率 */
	uint32_t ARR = 0;
	uint32_t PSC = 0;
	uint32_t CCR = 0;
	float pwmT_ns = 0;
	float pwmStep_Dutycycle_ns = 0;
	float pwmDutyCycle_ns = 0;

	if(pwmSetPref == pwmSetPref_Dutycycle)			/* 侧重占空比步幅精度 */
	{
		/*
		 * 牺牲频率步幅精度，换取占空比步幅精度
		 * 备注，对于16bit ARR寄存器的TIM，PSC和ARR寄存器都是16bit，对于32bit,,,要再考虑一下
		 */

		/*对于PSC的确定需要递归算法 */
		setPSC(&PSC, &ARR, pwmFrequency, product_PSC_ARR);

		for(uint8_t i = 0; ; i++){
			if(binPower(i) > ARR){
				/* 对ARR做二进制的幂次"四舍五入",ARR取2幂次 */
				if(abs((binPower(i) - ARR)) > abs(binPower(i - 1) - ARR)){
					ARR = binPower(i - 1);
				}else{
					ARR = binPower(i);
				}
				pwmFrequency = (float)product_PSC_ARR / PSC /ARR + 0.5;	// 归并pwmFrequency, 并仅保留整数，小数部分四舍五入到整数
				break;
			}
		}
		pwmT_ns = (float)1000000000 / pwmFrequency;					/* 计算PWM周期，单位ns*/
		pwmStep_Dutycycle_ns = pwmT_ns / ARR;						/* 计算PWM占空比步幅，单位ns*/
		pwmDutyCycle_ns = fmap(pwmDutyCycle, 0, 100, 0, pwmT_ns);	//映射0-100%到0-1000000000ns
		pwmDutyCycle_ns = pwmDutyCycle_ns - fmod(pwmDutyCycle_ns, pwmStep_Dutycycle_ns);	/*对浮点型进行取模运算, 归并pwmDutyCycle */
		CCR = pwmDutyCycle_ns / pwmStep_Dutycycle_ns;
		pwmDutyCycle = fmap(pwmDutyCycle_ns, 0, pwmT_ns, 0, 100);
	}
	else if(pwmSetPref == pwmSetPref_Frequency) /* 侧重频率步幅精度，占空比固定50%*/
	{
		/*
		 * 牺牲占空比步幅精度，换取频率步幅精度
		 */
		;
	}

	/* 更新pwmSetInfo */
	pwmSetInfo.pwm_Dutycycle = pwmDutyCycle;
	pwmSetInfo.pwmStep_Dutycycle = fmap(pwmStep_Dutycycle_ns, 0, pwmT_ns, 0, 100);
	pwmSetInfo.pwm_Frequency = pwmFrequency;
	pwmSetInfo.pwmStep_Frequency = 0;

	/* 更新TIMx要修的的寄存器 */
	htim->Instance->PSC = PSC - 1;
	htim->Instance->ARR = ARR - 1;
	CCR -= 1;
	switch (Channel)
	{
		case TIM_CHANNEL_1:
		{
			htim->Instance->CCR1 = CCR;
			break;
		}

		case TIM_CHANNEL_2:
		{
			htim->Instance->CCR2 = CCR;
			break;
		}

		case TIM_CHANNEL_3:
		{
			htim->Instance->CCR3 = CCR;
			break;
		}

		case TIM_CHANNEL_4:
		{
			htim->Instance->CCR4 = CCR;
			break;
		}

		case TIM_CHANNEL_5:
		{
			htim->Instance->CCR5 = CCR;
			break;
		}

		case TIM_CHANNEL_6:
		{
			htim->Instance->CCR6 = CCR;
			break;
		}

		default:
			break;
	}

	return 	pwmSetInfo;
}



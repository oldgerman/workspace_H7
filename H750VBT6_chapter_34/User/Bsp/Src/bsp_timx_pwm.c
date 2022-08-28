/*
 * bsp_pwm.c
 *
 *  Created on: Aug 25, 2022
 *      Author: OldGerman
 */

#include "bsp.h"

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

/**
 * @brief  开启或关闭tim pwm的通道
 * @param  htim:				htim句柄指针
 * @param  Channel:				tim的通道
 * @param  enable:				true:打开对应通道的pwm; false:关闭对应通道的pwm
 * @retval HAL Status
 */
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
 * @brief  设置tim pwm的频率和占空比
 * @param  htim:				htim句柄指针
 * @param  Channel:				tim的通道
 * @param  pwmFrequency: 		pwm频率，  范围 1 ~ timBusCLK	单位: Hz
 * @param  pwmDutyCycle: 		pwm占空比，范围 0.0... ~ 100.0...，	单位: %
 * @retval pwmSet_InfoTypeDef 	经计算后的实际情况
 */
pwmSet_InfoTypeDef bsp_TIMx_PWM_Set(
		TIM_HandleTypeDef* htim,
		uint32_t Channel,
		uint32_t pwmFrequency,
		float pwmDutyCycle)
{
	/* 关闭调节对通道的PWM */
	HAL_TIM_PWM_Stop(htim, Channel);

	/* 确定定时器的时钟源频率，以及定时器是16bit还是32bit的 */
	uint32_t timBusCLK;						//	htim所挂在总线的时钟频率：单位: Hz
	TIM_TypeDef* TIMx = htim->Instance;
	if ((TIMx == TIM1) || (TIMx == TIM8) || (TIMx == TIM15) || (TIMx == TIM16) || (TIMx == TIM17)) {
		timBusCLK = SystemCoreClock / 2;	// APB2 定时器时钟 = 200M
	}
	else {
		timBusCLK = SystemCoreClock / 2; 	// APB1 定时器 = 200M
	}

	/* 确定定时器位数 */
	uint32_t tim_count_max;
	if((TIMx == TIM2) || (TIMx == TIM5)){	// TIM2 和 TIM5是 32 位定时器，其它都是 16 位定时器
		tim_count_max = 0xffffffff;			// 32bit  0 ~ 4,294,967,295
	}else{
		tim_count_max = 0xffff;				// 16bit  0 ~ 65535
	}

	/* 声明或初始化局部变量 */
	pwmSet_InfoTypeDef pwmSetInfo = {0};
	pwmSetInfo.pwm_Frequency_Expect = pwmFrequency;
	pwmSetInfo.pwm_Dutycycle_Expect = pwmDutyCycle;
	uint32_t product_PSC_ARR = timBusCLK;			// PSC和ARR的乘积总是等于TIM所挂在总线的时钟频率
	uint32_t ARR = 0;
	uint32_t PSC = 0;
	uint32_t CCR = 0;
	float pwmT_ns = 0;								// stm32 float: -2,147,483,648 ~ 2,147,483,647
	float pwmStep_Dutycycle_ns = 0;
	float pwmDutyCycle_ns = 0;

	/*
	 * 下列计算时暂时不减去1，最后设置时才减去1
	 * 备注，PSC都是16bit，暂时只管16bit ARR寄存器的TIM，而32bit的ARR以后再说
	 */

	/* 计算有效范围内的PSC和ARR */
	for(uint16_t i = 1; ; i++){
		/* 这里可以用二分法更快, 得考虑一下 */
		PSC = i;			// N 时间复杂度, pwm=1Hz时，N最大1525
		ARR = (float)product_PSC_ARR / PSC / pwmFrequency + 0.5;	// 注意对ARR进行了四舍五入
		if(ARR <= tim_count_max){
			break;
		}
	}

	/* 计算实际pwm参数 */
	float pwmFrequency_float = (float)product_PSC_ARR / PSC /ARR;
	pwmT_ns = (float)1000000000 / pwmFrequency_float;			// 计算PWM周期，单位ns
	pwmStep_Dutycycle_ns = pwmT_ns / ARR;						// 计算PWM占空比步幅，单位ns
	pwmDutyCycle_ns = fmap(pwmDutyCycle, 0, 100, 0, pwmT_ns);	// 映射0-100%到0-pwmT_ns
//	pwmDutyCycle_ns = pwmDutyCycle_ns - fmod(pwmDutyCycle_ns, pwmStep_Dutycycle_ns);	// 对浮点型进行取模运算
	pwmDutyCycle = fmap(pwmDutyCycle_ns, 0, pwmT_ns, 0, 100);

	/* 计算有效范围内的CCR */
	CCR = pwmDutyCycle_ns / pwmStep_Dutycycle_ns + 0.5;			// 注意对CCR进行了四舍五入

	/* 更新pwmSetInfo */
	pwmSetInfo.pwm_Dutycycle = pwmDutyCycle;
	pwmSetInfo.pwmStep_Dutycycle = fmap(pwmStep_Dutycycle_ns, 0, pwmT_ns, 0, 100);
	pwmSetInfo.pwm_Frequency = pwmFrequency_float;

	/* 更新TIMx要修改的的寄存器 */
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



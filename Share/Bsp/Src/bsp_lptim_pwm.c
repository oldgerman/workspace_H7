/*
 * bsp_lptim_pwm.c
 *
 *  Created on: Aug 30, 2022
 *      Author: OldGerman
 */
#include "bsp_config.h"
#ifdef EN_BSP_LPTIM_PWM
#include "bsp.h"

/**
 * @brief  开启或关闭tim pwm的通道
 * @param  htim:				htim句柄指针
 * @param  Channel:				tim的通道
 * @param  enable:				true:打开对应通道的pwm; false:关闭对应通道的pwm
 * @retval HAL Status
 */
HAL_StatusTypeDef bsp_LPTIMx_PWM_En(LPTIM_HandleTypeDef *hlptim, bool enable){
	if(enable){
		if(HAL_LPTIM_PWM_Start(hlptim, hlptim->Instance->ARR, hlptim->Instance->CMP) != HAL_OK)
			return HAL_ERROR;
	}
	else{
		if( HAL_LPTIM_PWM_Stop(hlptim) != HAL_OK)
			return HAL_ERROR;
	}
	return HAL_OK;
}

/**
 * @brief  设置lptim pwm的频率和占空比
 * @notice PWM占空比为0%时输出LPTIM PWM极性相反电平
 * @param  hlptim:				hlptim句柄指针
 * @param  LptimClockFreq:		lptim的时钟源频率，单位Hz （这个可以在CubeMX的时钟树配置并看到频率，因为从hlptim获取LPTIMx时钟源频率很麻烦，所以麻烦你一下~）
 * @param  pwmFrequency: 		pwm频率，  范围 1 ~ LptimClockFreq	单位: Hz
 * @param  pwmDutyCycle: 		pwm占空比，范围 0.0... ~ 100.0...，	单位: %
 * @retval pwmSet_InfoTypeDef 	经计算后的情况
 */
pwmSet_InfoTypeDef bsp_LPTIMx_PWM_Set(
		LPTIM_HandleTypeDef *hlptim,
		uint32_t LptimClockFreq,
		uint32_t pwmFrequency,
		float pwmDutyCycle)
{
	/* 确定定时器位数 */
	uint32_t lptim_count_max = 0xffff;				// 16bit  0 ~ 65535

	/* 声明或初始化局部变量 */
	pwmSet_InfoTypeDef pwmSetInfo = {0};
	pwmSetInfo.pwm_Frequency_Expect = pwmFrequency;
	pwmSetInfo.pwm_Dutycycle_Expect = pwmDutyCycle;
	uint32_t product_PSC_ARR = LptimClockFreq;			// PSC和ARR的乘积总是等于TIM所挂在总线的时钟频率
	uint32_t ARR = 0;
	uint32_t PSC = 0;
	uint32_t CMP = 0;
	float pwmT_ns = 0;								// stm32 float: -2,147,483,648 ~ 2,147,483,647
	float pwmStep_Dutycycle_ns = 0;
	float pwmDutyCycle_ns = 0;

	/* 关闭PWM后进行寄存器配置，实测若不关闭会产生设定值异常（似乎是访问冲突） */
	if(HAL_LPTIM_PWM_Stop(hlptim) != HAL_OK){
		pwmSetInfo.hal_Status = HAL_ERROR;
		return pwmSetInfo;
	}
	else
		pwmSetInfo.hal_Status = HAL_OK;

	/*
	 * 下列计算时暂时不减去1，最后设置时才减去1
	 * 备注：
	 * PSC是PRESC[2:0] 3 位域000~111对应的分频范围1~128（8种分频值）
	 * ARR是16bit
	 */

	/* 检测pwmFrequency的有效范围 */
	if(pwmFrequency * 2 > LptimClockFreq)
		pwmFrequency = LptimClockFreq /2;
	/* 计算有效范围内的PSC和ARR */
	for(uint16_t i = 0; i < 8; i++){
		PSC = twoNthPower(i);
		ARR = (float)product_PSC_ARR / PSC / pwmFrequency + 0.5;	// 注意对ARR进行了四舍五入
		if(ARR <= lptim_count_max){
			break;
		}
	}

	/* 计算实际pwm参数 */
	float pwmFrequency_float = (float)product_PSC_ARR / PSC / ARR;
	pwmT_ns = (float)1000000000 / pwmFrequency_float;			// 计算PWM周期，单位ns
	pwmStep_Dutycycle_ns = pwmT_ns / ARR;						// 计算PWM占空比步幅，单位ns
	pwmDutyCycle_ns = fmap(pwmDutyCycle, 0, 100, 0, pwmT_ns);	// 映射0-100%到0-pwmT_ns
	pwmDutyCycle_ns = pwmDutyCycle_ns - fmod(pwmDutyCycle_ns, pwmStep_Dutycycle_ns);	// pwmDutyCycle_ns - pwmDutyCycle_ns取pwmStep_Dutycycle_ns模运算的值 = pwmStep_Dutycycle_ns整倍数
	pwmDutyCycle = fmap(pwmDutyCycle_ns, 0, pwmT_ns, 0, 100);

	/* 计算有效范围内的CMP */
	CMP = pwmDutyCycle_ns / pwmStep_Dutycycle_ns + 0.5;			// 注意对CMP进行了四舍五入

	/* 更新pwmSetInfo */
	pwmSetInfo.pwm_Dutycycle = pwmDutyCycle;
	pwmSetInfo.pwmStep_Dutycycle = fmap(pwmStep_Dutycycle_ns, 0, pwmT_ns, 0, 100);
	pwmSetInfo.pwm_Frequency = pwmFrequency_float;

	/* 更新LPTIMx要修改的寄存器 */
	// ARR
	__HAL_LPTIM_AUTORELOAD_SET(hlptim, ARR - 1);
	// CMP
	__HAL_LPTIM_COMPARE_SET(hlptim, CMP - 1);
	// PRESC[2:0] 3 位域
	uint32_t tmpcfgr = hlptim->Instance->CFGR;	// Get the LPTIMx CFGR value
	PSC = ((uint32_t)(twoNthPowerOfNth(PSC) - 1) << LPTIM_CFGR_PRESC_Pos) & LPTIM_CFGR_PRESC_Msk;	// 得到PSC在CFGR的3bit段值,注意得到000~111要将值twoNthPowerOfNth(PSC)减去1
	tmpcfgr &= ~(LPTIM_CFGR_PRESC_Msk);			// 归零CFGR原有的2幂次分频器的值， LPTIM_CFGR_PRESC_Msk = ...111000000000
	tmpcfgr |= PSC;								// 设置现有的PSC到CFGR的2幂次分频器
	hlptim->Instance->CFGR = tmpcfgr;

	return 	pwmSetInfo;
}

#endif

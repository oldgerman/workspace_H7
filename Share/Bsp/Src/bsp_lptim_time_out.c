/*
 * bsp_lptim_wkup.c
 *
 *  Created on: Aug 31, 2022
 *      Author: PSA
 */


#include "bsp_config.h"
#ifdef EN_BSP_LPTIM_TIME_OUT
#include "bsp.h"


/* 指向某个CubeMX初始化的hlptim Handle */
static LPTIM_HandleTypeDef *hlptimx = NULL;
/* 保存 TIM定时中断到后执行的回调函数指针 */
static void (*s_LPTIM_TimeOut_CallBack)(void);

//返回2的N次幂
static uint32_t twoNthPower(uint8_t Nth){
	return 1 << Nth;
}

//返回某个数（2 N次幂）的N
static uint8_t twoNthPowerOfNth(uint32_t num){
	uint8_t Nth = 0;
	for(; Nth < 32; Nth++){
		if((num >> Nth) == 0){
			break;
		}
	}
	return Nth;
}

/**
 * @brief  开启或关闭hlptimx的TimeOut_IT
 * @param  enable				true:打开; false:关闭
 * @retval HAL Status
 */
HAL_StatusTypeDef bsp_LPTIMx_TimeOut_En(bool enable){
	if(hlptimx == NULL)
		return HAL_ERROR;

	if(enable){
		if(HAL_LPTIM_TimeOut_Start_IT(hlptimx, hlptimx->Instance->ARR, hlptimx->Instance->CMP) != HAL_OK)
			return HAL_ERROR;
	}
	else{
		if( HAL_LPTIM_TimeOut_Stop_IT(hlptimx) != HAL_OK)
			return HAL_ERROR;
	}
	return HAL_OK;
}


/**
 * @brief  设置lptim timeOut中断的触发时间
 * @param  hlptim				hlptim句柄指针
 * @param  LptimClockFreq		lptim的时钟源频率，单位Hz （这个可以在CubeMX的时钟树配置并看到频率，因为从hlptim获取LPTIMx时钟源频率很麻烦，所以麻烦你一下~）
 * @param  time 				超时设定的时间，单位us
 * @param  pCallBack 			超时时间到后，被执行的函数
 * @retval uint32_t 			经计算后的实际延时时间，单位ms
 */
uint32_t bsp_LPTIMx_TimeOut_Set(
		LPTIM_HandleTypeDef *hlptim,
		uint32_t LptimClockFreq,
		uint32_t time,
		void (*pCallBack)(void))
{
	/* 保存回调函数 */
	s_LPTIM_TimeOut_CallBack = pCallBack;

	/* 保存handle */
	hlptimx = hlptim;

	/* 关闭TimeOut中断 */
	HAL_LPTIM_TimeOut_Stop_IT(hlptimx);

	/* 确定定时器位数 */
	uint32_t lptim_count_max = 0xffff;				// 16bit  0 ~ 65535

	/* 声明或初始化局部变量 */
	uint32_t timeCal = 0;
	uint32_t PSC = 0;
	uint32_t CMP = 0;		// 硬汉哥实际测试发现溢出中断与ARR寄存器无关，全部由CMP寄存器决定（本ID又实测雀食如此，因此以下计算省去ARR）
	uint32_t PSC_Max = 0;
	uint32_t CNT_TimeMax = 0;
	float timeToFrequency_float = 0;					//stm32 float: -2,147,483,648 ~ 2,147,483,647
	/*
	 * 下列计算时暂时不减去1，最后设置时才减去1
	 */

	/* 检测time的有效范围 */
	// 计数PSC位域最大值 128
	PSC_Max = twoNthPower(LPTIM_CFGR_PRESC_Msk >> LPTIM_CFGR_PRESC_Pos);
	// 计算LPTIM计数器最大能计数的时间，单位us，当 LptimClockFreq = 32768Hz 时，计算结果为256,000,000(us)，即256秒
	CNT_TimeMax = (float)1000000 / (LptimClockFreq / PSC_Max) * twoNthPower(16);
	if(time > CNT_TimeMax)
		time = CNT_TimeMax;

	// 计算time对应的频率，单位Hz，当time = 256s时，频率为0.00390625Hz
	timeToFrequency_float = (float)1000000 / time;

	/* 计算有效范围内的CMR，与ARR无关 */
	for(uint16_t i = 0; i < 8; i++){
		PSC = twoNthPower(i);
		CMP = (float)LptimClockFreq / PSC / timeToFrequency_float + 0.5;	// 注意对CMP进行了四舍五入
		if(CMP <= lptim_count_max){
			break;
		}
	}

	/* 计算实际延时时间 */
	timeCal = (float)1000000 / (LptimClockFreq / PSC) * CMP;

	/* 更新LPTIMx要修改的寄存器 */
	// CMP
	__HAL_LPTIM_COMPARE_SET(hlptimx, CMP - 1);

	// PRESC[2:0] 3 位域
	uint32_t tmpcfgr = hlptimx->Instance->CFGR;	// Get the LPTIMx CFGR value
	PSC = ((uint32_t)(twoNthPowerOfNth(PSC) - 1) << LPTIM_CFGR_PRESC_Pos) & LPTIM_CFGR_PRESC_Msk;	// 得到PSC在CFGR的3bit段值,注意得到000~111要将值twoNthPowerOfNth(PSC)减去1
	tmpcfgr &= ~(LPTIM_CFGR_PRESC_Msk);			// 归零CFGR原有的2幂次分频器的值， LPTIM_CFGR_PRESC_Msk = ...111000000000
	tmpcfgr |= PSC;								// 设置现有的PSC到CFGR的2幂次分频器
	hlptimx->Instance->CFGR = tmpcfgr;

	return 	timeCal;
}



/**
 * @brief   需要被重载HAL_LPTIM_CompareMatchCallback()调用
 * @@param  hlptim LPTIM handle
 * @retval  None
 */
void bsp_lptim_timeOut_LPTIM_CompareMatchCallback(LPTIM_HandleTypeDef *hlptim) {
	if(hlptim == hlptimx) {
		__HAL_LPTIM_CLEAR_FLAG(hlptimx, LPTIM_FLAG_CMPM);	/* 清除比较匹配中断 */
		HAL_LPTIM_TimeOut_Stop_IT(hlptimx);					/* 关闭溢出中断 */
		s_LPTIM_TimeOut_CallBack();							/* 执行保存的回调函数*/
	}
}

#endif

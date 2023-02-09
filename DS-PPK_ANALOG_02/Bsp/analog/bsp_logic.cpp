/*
 * bsp_logic.cpp
 *
 *  Created on: Feb 9, 2023
 *      Author: OldGerman
 */

#include "bsp_analog.h"
#include "bsp_logic.h"
#include "tim.h"
#include "bsp.h"	//提供fmap()
#include <stdlib.h> //abs()

static void logicCCRCompensation(uint32_t * ccr);

static float logic_k_ccr = 1, logic_b_ccr = 0;	//用作ccr一次函数补偿

osThreadId_t logicInitTaskHandle;
const uint32_t logicInitTaskStackSize = 256 * 4;
const osThreadAttr_t logicInitTask_attributes = {
    .name = "logicInitTask",
    .stack_size = logicInitTaskStackSize,
    .priority = (osPriority_t) osPriorityLow,
};

static void threadLogicInit(void* argument)
{
	HAL_TIM_Base_Start(&htim15);
	bsp_logicVoltageCal();
	/* 设置VCCB电平电压为3.3V */
	bsp_logicSetVoltageLevel(3300.0f);
	printf("[logicInitTask]: Task deleted");
	osThreadTerminate(logicInitTaskHandle);	//删除 auto sw 初始化任务
}


/**
 * @brief	设置逻辑电平转换器的外部输入端电压
 * @param	mV				:电压
 * @param	compensation	:是否开启补偿算法
 * @retval	PWM DAC 的定时器的CCR寄存器值
 */
uint32_t bsp_logicSetVoltageLevel(float mV, bool compensation)
{
	/**
	 * y\ =\ \left[\frac{0.5}{40.2}-\frac{x-0.5}{330}\right]330+0.5
	 *
	 * TPS63000
	 * 		V(FB) = 500mV = 0.5
	 * 		V(OUT): Adjustable Output Voltage Options from 1.2 V to 5.5 V
	 *
	 * 		Feedback resistor:
	 *
	 *    		V(OUT)---T--------VCCB
	 * 	  				 R1
	 * 	  		V(FB)----+---R3---V(PWM)
	 * 	  			 	 R2
	 * 	  			 	 |
	 * 	  				GND
	 *
	 * 			R1 = 330K
	 * 			R2 = 40.2K
	 * 			R3 = 215K
	 *
	 * 		VCCB = V(OUT) = (V(FB)/R2 - (V(PWM) - V(FB))/R3) * R1 + V(FB)
	 * 		V(PWM) = (V(FB)/R2 - (V(OUT) - V(FB))/R1) * R3 + V(FB)
	 */
	const float VFB = LOGIC_FB_VFB_mV;
	const float R1 = LOGIC_FB_R1_KR, R2 = LOGIC_FB_R2_KR, R3 = LOGIC_FB_R3_KR;
	float VOUT, VPWM, DutyCycle;

	/* 检查mV有效范围 */
//	if(mV > LOGIC_LEVEL_HIGH_MAX_mV || mV < LOGIC_LEVEL_HIGH_MIN_mV){
//		return;
//	}

	VOUT = mV;
	VPWM = (VFB / R2 - (VOUT - VFB) / R1) * R3 + VFB;
	DutyCycle = VPWM / 2500.0f;	//3000mV PWM 经二阶RC滤波器后再接反馈电阻的负载，100%占空比时只有2.5V左右

	uint32_t CCR = fmap(DutyCycle, 0.0f, 1.0f, 0.0f, 255.0f);
	if(compensation)
	{
		logicCCRCompensation(&CCR);
	}

	if(CCR > 255){
		CCR = 255;
	}

	htim15.Instance->CCR1 = CCR;

	//STOP TIM 会将引脚状态变为开漏，V(PWM)电压会接近于V(FB)而不是0V
//		HAL_TIM_PWM_Stop(&htim15, TIM_CHANNEL_1);

	HAL_TIM_PWM_Start(&htim15, TIM_CHANNEL_1);

	return CCR;
}

/* 校准逻辑电压调节范围 */
void bsp_logicVoltageCal()
{
	float  ccr_h, ccr_l;					//CCR寄存器高，低
	float v_mh, v_ml;						//测量电压高，低
	const float v_eh = 5.0, v_el = 2.0;    //期望电压高，低，单位V

	float vlogic_old = adc2_values.float_el.val_vlogic;
	float vlogic_new = vlogic_old;

	ccr_l = bsp_logicSetVoltageLevel(v_el * 1000, false);
	do{
    	osDelay(1000);
    	vlogic_new = adc2_values.float_el.val_vlogic;
	}while( abs(vlogic_new - vlogic_old) <= 1e-6 );
	v_ml = vlogic_new; //等待电压稳定后获取v_ml
	vlogic_old = vlogic_new;

	ccr_h = bsp_logicSetVoltageLevel(v_eh * 1000, false);
	do{
    	osDelay(1000);
    	vlogic_new = adc2_values.float_el.val_vlogic;
	}
    while( abs(vlogic_new - vlogic_old) <= 1e-6 );

	v_mh = vlogic_new; //等待电压稳定后 获取 v_mh

	//计算实际ccr与v_ex的y=kx+b
	float k_m;
	float b_m;
	k_m = (v_mh - v_ml) / (ccr_h - ccr_l);
	b_m = v_mh - k_m * ccr_h;

	float ccr_eh, ccr_el;			//期望电压对应的期望CCR
	ccr_eh = (v_eh - b_m) / k_m; 	//期望电压带入测量得到的一次函数内得到期望CCR
	ccr_el = (v_el - b_m) / k_m;

	//计算期望ccr与实际ccr的y=kx+b
	float k_ccr;
	float b_ccr;
	k_ccr = (ccr_eh - ccr_el) / (ccr_h - ccr_l);
	b_ccr = ccr_eh - k_ccr * ccr_h;

	logic_k_ccr = k_ccr;
	logic_b_ccr = b_ccr;
}

/* 修正ccr */
static void logicCCRCompensation(uint32_t * ccr)
{
	float ccr_x = *ccr;
	int32_t ccr_y;
	ccr_y = logic_k_ccr * ccr_x + logic_b_ccr;
	if(ccr_y < 0) {
		*ccr = 0;
	}else{
		*ccr = ccr_y;
	}
}

void bsp_logicInit()
{
    logicInitTaskHandle = osThreadNew(threadLogicInit, nullptr, &logicInitTask_attributes);
}

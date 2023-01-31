/*
 * bsp_adc1.cpp
 *
 *  Created on: Jan 31, 2023
 *      Author: PSA
 */

#include "bsp_analog.h"
#include "adc.h"
#include "lptim.h"
#include "bsp_lptim_pwm.h"

const uint8_t adc1_chx_num_regular = 6;		//adc1规则通道数
const uint8_t adc1_chx_num_inject = 0;		//adc1注入通道数

extern float vref;


/* 方便Cache类的API操作，做32字节对齐 */
const uint16_t adc1_data_num = 1000;
ALIGN_32BYTES(__attribute__((section (".RAM_D2_Array"))) uint16_t adc1_data[adc1_data_num]);

float adc1_value;

void bsp_adc1Start()
{
	/* 启动ADC的DMA方式传输 */
	if (HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc1_data, adc1_data_num) != HAL_OK)
	{
	    Error_Handler();
	}
}

void bsp_adc1Init()
{
	/* 校准ADC，采用偏移校准 */
	if (HAL_ADCEx_Calibration_Start(
			&hadc1,
			ADC_CALIB_OFFSET,
			ADC_DIFFERENTIAL_ENDED) 	//差分模式校准
			!= HAL_OK)
	{
		Error_Handler();
	}
	/**
	 * TODO: 	ADC线性校准
	 * @armfly	STM32H7的ADC支持偏移校准和线性度校准。如果使用线性度校准的话，特别要注意此贴的问题：
	 * 			http://www.armbbs.cn/forum.php?mod=viewthread&tid=91436
	 * 			现在STM32H7Cube库已经修改了溢出时间，看起来是够了
	 */
	/* 配置DMA回调函数 */
//	bsp_DMA_Set(&hdma_adc1, bsp_DMA__XferCpltCallback, bsp_DMA_XferM1CpltCallback);
	/* 开启ADC */
	bsp_adc1Start();
}

void bsp_adc1GetValues()
{
	SCB_InvalidateDCache_by_Addr((uint32_t *)adc1_data, sizeof(adc1_data));
	adc1_value = 0;
	for(int i = 0; i < adc1_data_num; i++){
		adc1_value += adc1_data[i];
	}
	adc1_value /= adc1_data_num;
	adc1_value = (adc1_value - 32767) / 32767 * vref; // 单位V, VDOUT
	printf("IA_SE_OUT: %.6f\r\n", adc1_value);
}



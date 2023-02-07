/*
 * bsp_adc2.cpp
 *
 *  Created on: Jan 30, 2023
 *      Author: PSA
 */

#ifndef ANALOG_BSP_ADC2_CPP_
#define ANALOG_BSP_ADC2_CPP_

#include "bsp_analog.h"
#include "adc.h"

float vref = 2.5000;	//单位：V，REF5025基准电压，TODO：待预留给外部更高精度仪器校准

/* 方便Cache类的API操作，做32字节对齐 */
ALIGN_32BYTES(__attribute__((section (".RAM_D2_Array"))) adc2_chx_values_t adc2_data);

volatile adc2_chx_values_t adc2_values;

void bsp_adc2Start()
{
	/* 校准ADC，采用偏移校准 */
	if (HAL_ADCEx_Calibration_Start(&hadc2, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED) != HAL_OK)
	{
		Error_Handler();
	}

	/**
	 * TODO: 	ADC线性校准
	 * @armfly	STM32H7的ADC支持偏移校准和线性度校准。如果使用线性度校准的话，特别要注意此贴的问题：
	 * 			http://www.armbbs.cn/forum.php?mod=viewthread&tid=91436
	 * 			现在STM32H7Cube库已经修改了溢出时间，看起来是够了
	 */

	/* 启动ADC的DMA方式传输 */
	if (HAL_ADC_Start_DMA(&hadc2, (uint32_t *)adc2_data.uint_arr, adc2_chx_num_regular) != HAL_OK)
	{
	    Error_Handler();
	}
}

void bsp_adc2Init(){
	bsp_adc2Start();
}

void bsp_adc2GetValues()
{
	adc2_values.float_el.val_vldo = vref * adc2_data.uint_el.val_vldo / 65536 / (5.49f / (5.49f + 10.0f));
	adc2_values.float_el.val_vin = vref * adc2_data.uint_el.val_vin / 65536 / (10.0f / (10.0f + 8.2f));	//VIN（VSYS）
	adc2_values.float_el.val_vntc = vref * adc2_data.uint_el.val_vntc / 65536 ;
	adc2_values.float_el.val_vbb = vref * adc2_data.uint_el.val_vbb / 65536 / (5.49f / (5.49f + 10.0f));
	adc2_values.float_el.val_vlogic = vref * adc2_data.uint_el.val_vlogic / 65536 / (6.8f / (6.8f + 8.2f));
	adc2_values.float_el.val_vref_ia = vref * adc2_data.uint_el.val_vref_ia / 65536 ;

	SCB_InvalidateDCache_by_Addr((uint32_t *)adc2_data.uint_arr, sizeof(adc2_data));
//	printf("adc2 values: %.6f, %.6f, %.6f, %.6f, %.6f, %.6f\r\n",
	printf("[adc2 values] %.6f, %.6f, %.6f, %.6f, %.6f, %.6f\r\n",
			adc2_values.float_el.val_vldo,
			adc2_values.float_el.val_vin,
			adc2_values.float_el.val_vntc,
			adc2_values.float_el.val_vbb,
			adc2_values.float_el.val_vlogic,
			adc2_values.float_el.val_vref_ia);
	/**
	 * TODO:	NTC查表 + 线性差值 得到温度
	 */
}

#endif /* ANALOG_BSP_ADC2_CPP_ */

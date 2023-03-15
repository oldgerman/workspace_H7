/*
 * bsp_adc1.cpp
 *
 *  Created on: Jan 31, 2023
 *      Author: OldGerman
 */

#include "bsp_analog.h"
#include "adc.h"
#include "lptim.h"
#include "bsp_lptim_pwm.h"
#include "bsp_timestamp.h"

const uint8_t adc1_chx_num_regular = 6;		//adc1规则通道数
const uint8_t adc1_chx_num_inject = 0;		//adc1注入通道数

extern float vref;

/* 方便Cache类的API操作，做32字节对齐 */
ALIGN_32BYTES(__attribute__((section (".RAM_D2_Array"))) uint16_t adc1_data[adc1_data_num]);

float adc1_value;

/* ADC1 任务调度频率 */
TickType_t xFrequency_adc1Task = 500;

const uint32_t adc1TaskStackSize = 256 * 4;
osThreadId_t adc1TaskHandle;
void threadAdc1Update(void* argument){
	TickType_t xLastWakeTime;
	/* 获取当前的系统时间 */
	xLastWakeTime = xTaskGetTickCount();
	for(;;){
		bsp_adc1GetValues();
		vTaskDelayUntil(&xLastWakeTime, xFrequency_adc1Task);
	}
};
const osThreadAttr_t adc1Task_attributes = {
    .name = "adc1Task",
    .stack_size = adc1TaskStackSize,
    .priority = (osPriority_t) osPriorityHigh,
};

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
//			ADC_DIFFERENTIAL_ENDED) 	//差分模式校准
			ADC_SINGLE_ENDED) 	//单端模式校准
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

//	adc1TaskHandle = osThreadNew(threadAdc1Update, nullptr, &adc1Task_attributes);
}

void bsp_adc1GetValues()
{
	SCB_InvalidateDCache_by_Addr((uint32_t *)adc1_data, sizeof(adc1_data));
	adc1_value = 0;
#if 0
	for(int i = 0; i < adc1_data_num; i++){
		adc1_value += adc1_data[i];
	}
	adc1_value /= adc1_data_num;
#else
	adc1_value = adc1_data[0];
#endif
//	adc1_value = (adc1_value - 32767) / 32767 * vref; // 单位V, IA_SE_OUT
	adc1_value = adc1_value / 65535 * vref;
	float mA = 0;
	float mv_drop_sample_res = (adc1_value - adc2_values.float_el.val_vref_ia) * 1000.0f / ina_gain;
	float res_sample = 0;

	uint32_t bs = 0;
	uint32_t cs = 0;
	bs = timestamp.bs;		//当前使用的缓冲区
	cs = timestamp.auto_sw[bs].cs;	//当前auto_sw 缓冲区的游标

	switch (timestamp.auto_sw[bs].range[cs].swx) {
		case 0b00000000:
			res_sample = res_val_sample.rs_0uA_100uA;
			break;
		case 0b00000001:
			res_sample = res_val_sample.rs_100uA_1mA;
			break;
		case 0b00000011:
			res_sample = res_val_sample.rs_1mA_10mA;
			break;
		case 0b00000111:
			res_sample = res_val_sample.rs_10mA_100mA;
			break;
		case 0b00001111:
			res_sample = res_val_sample.rs_100mA_2A;
			break;
		default:
			break;
	}
	mA = mv_drop_sample_res / res_sample;
//	printf("[IA_SE_OUT] %.6f\r\n", adc1_value);
	printf("mA: %.6f\r\n", mA);
}



/*
 * bsp_adc3.cpp
 *
 *  Created on: Jan 30, 2023
 *      Author: PSA
 */


#include "bsp_analog.h"
#include "adc.h"
#include "lptim.h"
#include "bsp_lptim_pwm.h"

const uint8_t adc3_chx_num_regular = 6;		//adc3规则通道数
const uint8_t adc3_chx_num_inject = 0;		//adc3注入通道数

extern float vref;

volatile uint32_t adc_callback_cnt = 0;
/**
 * 采样率 100KHz，数据帧 4byte，每秒 400KB 数据需要发送
 *
 * 缓冲区大小配置有讲究
 * 	 过大：滞后性明显，刷新或传输数据更新频率慢，一旦更新数据又过多
 * 	 过小：频繁进出中断，降低性能
 *
 * USB：USB FS 1号BULK端点Tx Fifo 为 2048byte，每秒400KB需要发送 200次
 * WIFI：ESP32-C3 一次最多发送 4092 byte，每秒400KB需要发送 101次，减少为每次发送3200 byte，每秒 128 次
 *
 * 将ADC1 和ADC3 的DMA缓冲区都设置为 2000 点，这样每秒传输完成和半传输完成中断的次数加起来就是 100次
 */
const uint16_t adc3_data_num = adc1_adc3_buffer_size;	// 2K点缓冲区
/* 方便Cache类的API操作，做32字节对齐 */
ALIGN_32BYTES(__attribute__((section (".RAM_D2_Array"))) uint16_t adc3_data[adc3_data_num]);

float adc3_value;

pwmSet_InfoTypeDef lptim1_pwm_set_info;

/*
	ADC 数值获取：DMA方式
	HAL_ADC_ConvCpltCallback()     // 半传输完成中断
	HAL_ADC_ConvHalfCpltCallback() // 传输完成中断
	HAL_ADC_Start_DMA()
	HAL_ADC_Stop_DMA()

	不需要向DMA句柄注册半传输和传输完成回调函数，即使注册了也不会执行
	HAL_DMA_RegisterCallback(hdma, HAL_DMA_XFER_CPLT_CB_ID, XferCpltCallback);			//!< M0 Full transfer
	HAL_DMA_RegisterCallback(hdma, HAL_DMA_XFER_M1CPLT_CB_ID, XferM1CpltCallback);		//!< M1 Full Transfer
 */

/**
  * @brief  Conversion DMA half-transfer callback in non-blocking mode.
  * @param hadc ADC handle
  * @retval None
  */
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
	if(hadc == &hadc1)
	{
		timestamp.buffer_select = TS_BUF_1ST;
		timestamp.dma_adc1[TS_BUF_1ST] = bsp_timestamp_get();

        osSemaphoreRelease(sem_adc_dma);
        adc_callback_cnt++;
	}
	else if(hadc == &hadc3)
	{
	}
}
/**
  * @brief  Conversion complete callback in non-blocking mode.
  * @param hadc ADC handle
  * @retval None
  */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	if(hadc == &hadc1)
	{
		timestamp.buffer_select = TS_BUF_2ND;
		timestamp.dma_adc1[TS_BUF_2ND] = bsp_timestamp_get();

        osSemaphoreRelease(sem_adc_dma);
        adc_callback_cnt++;
	}
	else if(hadc == &hadc3)
	{
	}
}



void bsp_adc3Stop()
{
	/* 启动ADC的DMA方式传输 */
	if (HAL_ADC_Stop_DMA(&hadc3) != HAL_OK)
	{
	    Error_Handler();
	}

	/* 关闭触发ADC转换的定时器 */
	bsp_LPTIMx_PWM_En(&hlptim1, false);
}


void bsp_adc3Start()
{
	/* 启动ADC的DMA方式传输 */
	if (HAL_ADC_Start_DMA(&hadc3, (uint32_t *)adc3_data, adc3_data_num) != HAL_OK)
	{
	    Error_Handler();
	}

	/* 配置触发ADC的定时器 */
//	bsp_LPTIMx_PWM_Set(&hlptim1, 6400000, 1000, 50);		//100Hz
	bsp_LPTIMx_PWM_Set(&hlptim1, 6400000, 100000, 50);	//100KHz

	/* 开启定时器触发ADC转换 */
	bsp_LPTIMx_PWM_En(&hlptim1, true);
}

void bsp_adc3Init()
{
	/* 校准ADC，采用偏移校准 */
	if (HAL_ADCEx_Calibration_Start(
			&hadc3,
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


	/* 开启ADC */
	bsp_adc3Start();
}

void bsp_adc3GetValues()
{
	SCB_InvalidateDCache_by_Addr((uint32_t *)adc3_data, sizeof(adc3_data));
	adc3_value = 0;
#if 0
	for(int i = 0; i < adc3_data_num; i++){
		adc3_value += adc3_data[i];
	}
	adc3_value /= adc3_data_num;
#else
	adc3_value = adc3_data[0];
#endif
	adc3_value = (adc3_value - 32767) / 32767 * vref * 2; // 单位V, VDOUT
	printf("[VDOUT] %.6f\r\n", adc3_value);

	static uint32_t adc_callback_cnt_old = 0;
	printf("[adc_callback_cnt] %.lu times 1s\r\n", adc_callback_cnt - adc_callback_cnt_old);
	adc_callback_cnt_old = adc_callback_cnt;
}


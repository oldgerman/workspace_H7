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


/* 方便Cache类的API操作，做32字节对齐 */
const uint16_t adc3_data_num = 1000;
ALIGN_32BYTES(__attribute__((section (".RAM_D2_Array"))) uint16_t adc3_data[adc3_data_num]);

float adc3_value;

pwmSet_InfoTypeDef lptim1_pwm_set_info;

/**
 * @brief  单独设置DMA，双缓冲中断模式
 * @param  hdma				  	DMA句柄，支持BDMA、DMA1、DMA2，注意DMA能访问的内存域，建议用BDMA以节省其他高级DMA
 * @param  SrcAddress: 			 The source memory Buffer address
 * @param  DstAddress:		 	 The destination memory Buffer address
 * @param  SecondMemAddress: 	 The second memory Buffer address in case of multi buffer Transfer
 * @param  DataLength: 			 The length of data to be transferred from source to destination
 * @param  XferCpltCallback      M0缓冲区传输完成回调
 * @param  XferM1CpltCallback  	 M1缓冲区传输完成回调
 * @retval HAL status
 */
void bsp_DMA_Set(
		DMA_HandleTypeDef *hdma,
//		uint32_t SrcAddress, uint32_t DstAddress, uint32_t SecondMemAddress, uint32_t DataLength,
		void (*XferCpltCallback)(DMA_HandleTypeDef * hdma),
		void (*XferM1CpltCallback)(DMA_HandleTypeDef * hdma))
{

	/* 注册回调函数 */
	HAL_DMA_RegisterCallback(hdma, HAL_DMA_XFER_CPLT_CB_ID, XferCpltCallback);			/*!< M0 Full transfer     */
	HAL_DMA_RegisterCallback(hdma, HAL_DMA_XFER_M1CPLT_CB_ID, XferM1CpltCallback);		/*!< M1 Full Transfer  */
//	/* 使能DMA请求发生器，注意CubeMX不会在自动生成的代码中调用此函数，需要用户自行在别处调用 */
//	status += HAL_DMAEx_EnableMuxRequestGenerator (hdma);
//
//	/* 启动DMA双缓冲传输，若不使能DMA请求发生器，则DMA无法开始传输 */
//	status += HAL_DMAEx_MultiBufferStart_IT(hdma, SrcAddress, DstAddress, SecondMemAddress, DataLength);

}

//回调函数示例模板
void bsp_DMA__XferCpltCallback(DMA_HandleTypeDef * hdma){
	/* 当前使用的缓冲 0 */
	/*
	 	armfly V7：43.2.4 DMA中断处理
		1、当前正在使用缓冲 1，此时可以动态修改缓冲 0 的数据。 比如缓冲区 0 是 IO_Toggle，缓冲区 1 是 IO_Toggle1，那么此时就可以修改 IO_Toggle。
		2、变量所在的 SRAM 区已经通过 MPU 配置为 WT 模式，更新变量 IO_Toggle 会立即写入。
		3、不配置 MPU 的话，也可以通过 Cahce 的函数 SCB_CleanDCache_by_Addr 做 Clean 操作。
	*/
	if(hdma == &hdma_adc1){
	}
	if(hdma == &hdma_adc3){
	}
}

void bsp_DMA_XferM1CpltCallback(DMA_HandleTypeDef * hdma){
	/* 当前使用的缓冲 1 */
	/*
	    armfly V7：43.2.4 DMA中断处理
		1、当前正在使用缓冲 0，此时可以动态修改缓冲 1 的数据。 比如缓冲区 0 是 IO_Toggle，缓冲区 1 是 IO_Toggle1，那么此时就可以修改 IO_Toggle1。
		2、变量所在的 SRAM 区已经通过 MPU 配置为 WT 模式，更新变量 IO_Toggle 会立即写入。
		3、不配置 MPU 的话，也可以通过 Cahce 的函数 SCB_CleanDCache_by_Addr 做 Clean 操作。
	*/
	if(hdma == &hdma_adc1){
	}
	if(hdma == &hdma_adc3){
	}
}


void bsp_adc3Start()
{
	/* 启动ADC的DMA方式传输 */
	if (HAL_ADC_Start_DMA(&hadc3, (uint32_t *)adc3_data, adc3_data_num) != HAL_OK)
	{
	    Error_Handler();
	}
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

	/* 配置触发ADC的定时器 */
	bsp_LPTIMx_PWM_Set(&hlptim1, 6400000, 2000, 50);
	/* 配置DMA回调函数 */
//	bsp_DMA_Set(&hdma_adc3, bsp_DMA__XferCpltCallback, bsp_DMA_XferM1CpltCallback);
	/* 开启ADC */
	bsp_adc3Start();
}

void bsp_adc3GetValues()
{
	SCB_InvalidateDCache_by_Addr((uint32_t *)adc3_data, sizeof(adc3_data));
	adc3_value = 0;
	for(int i = 0; i < adc3_data_num; i++){
		adc3_value += adc3_data[i];
	}
	adc3_value /= adc3_data_num;
	adc3_value = (adc3_value - 32767) / 32767 * vref * 2; // 单位V, VDOUT
	printf("VDOUT: %.6f\r\n", adc3_value);
}


/*
 * bsp_tim_dma.h
 *
 *  Created on: Sep 6, 2022
 *      Author: OldGerman
 */

#include "bsp_config.h"
#ifdef EN_BSP_TIM_DMA
#include "bsp.h"
#include "bsp_tim_dma.h"

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
HAL_StatusTypeDef bsp_Tim_DMA_DMA_Set(
		DMA_HandleTypeDef *hdma, uint32_t SrcAddress, uint32_t DstAddress, uint32_t SecondMemAddress, uint32_t DataLength,
		void (*XferCpltCallback)(DMA_HandleTypeDef * hdma),
		void (*XferM1CpltCallback)(DMA_HandleTypeDef * hdma))
{
	HAL_StatusTypeDef status = HAL_OK;

	/* 注册回调函数 */
	status += HAL_DMA_RegisterCallback(hdma, HAL_DMA_XFER_CPLT_CB_ID, XferCpltCallback);			/*!< M0 Full transfer     */
	status += HAL_DMA_RegisterCallback(hdma, HAL_DMA_XFER_M1CPLT_CB_ID, XferM1CpltCallback);		/*!< M1 Full Transfer  */

	/* 使能DMA请求发生器，注意CubeMX不会在自动生成的代码中调用此函数，需要用户自行在别处调用 */
	status += HAL_DMAEx_EnableMuxRequestGenerator (hdma);

	/* 启动DMA双缓冲传输，若不使能DMA请求发生器，则DMA无法开始传输 */
    /*
        1、此函数会开启DMA的TC，TE和DME中断
        2、如果用户配置了回调函数DMA_Handle.XferHalfCpltCallback，那么函数HAL_DMA_Init会开启半传输完成中断。
        3、如果用户使用了DMAMUX的同步模式，此函数会开启同步溢出中断。
        4、如果用户使用了DMAMUX的请求发生器，此函数会开始请求发生器溢出中断。
    */
	status += HAL_DMAEx_MultiBufferStart_IT(hdma, SrcAddress, DstAddress, SecondMemAddress, DataLength);

    /* 用不到的中断可以直接关闭 */
    //DMA1_Stream1->CR &= ~DMA_IT_DME;
    //DMA1_Stream1->CR &= ~DMA_IT_TE;
    //DMAMUX1_RequestGenerator0->RGCR &= ~DMAMUX_RGxCR_OIE;

	return status;
}

/**
 * @brief  单独设置PWM，占空比建议固定为50%
 * @param  htim					htim句柄指针
 * @param  Channel:				tim的通道
 * @param  pwmFrequency 		pwm频率，  范围 1 ~ LptimClockFreq	单位: Hz
 * @param  pwmDutyCycle  		pwm占空比，范围 0.0... ~ 100.0...，	单位: %
 * @retval pwmSet_InfoTypeDef 	经计算后的情况
 */
pwmSet_InfoTypeDef bsp_Tim_DMA_PWM_Set(
		TIM_HandleTypeDef* htim,
		uint32_t Channel,
		uint32_t pwmFrequency,
		float pwmDutyCycle)
{
	/* 配置LPTIM触发DMAMUX
	 * 占空比建议固定为50%，方便使用数组合成LPTIM PWM 1/2 周期整数倍数的GPIO脉冲
	 */
	return bsp_TIMx_PWM_Set(htim, Channel, pwmFrequency, pwmDutyCycle);
}

/**
 * @brief  打开或关闭LPTIM以开关DMA PWM，仅通过开关LPTIM PWM就行
 * @param  htim:				htim句柄指针
 * @param  Channel:				tim的通道
 * @param  enable:				true:打开对应通道的pwm; false:关闭对应通道的pwm
 * @retval HAL Status
 */
HAL_StatusTypeDef bsp_Tim_DMA_PWM_En(TIM_HandleTypeDef* htim, uint32_t Channel, bool enable){
	 return bsp_TIMx_PWM_En(htim, Channel, enable);
}

#if 0
//回调函数示例模板
void bsp_Tim_DMA_PWM_XferCpltCallback(DMA_HandleTypeDef * hdma){
	/* 当前使用的缓冲 0 */
	/*
	 	armfly V7：43.2.4 DMA中断处理
		1、当前正在使用缓冲 1，此时可以动态修改缓冲 0 的数据。 比如缓冲区 0 是 IO_Toggle，缓冲区 1 是 IO_Toggle1，那么此时就可以修改 IO_Toggle。
		2、变量所在的 SRAM 区已经通过 MPU 配置为 WT 模式，更新变量 IO_Toggle 会立即写入。
		3、不配置 MPU 的话，也可以通过 Cahce 的函数 SCB_CleanDCache_by_Addr 做 Clean 操作。
	*/
}

void bsp_Tim_DMA_PWM_XferM1CpltCallback(DMA_HandleTypeDef * hdma){
	/* 当前使用的缓冲 1 */
	/*
	    armfly V7：43.2.4 DMA中断处理
		1、当前正在使用缓冲 0，此时可以动态修改缓冲 1 的数据。 比如缓冲区 0 是 IO_Toggle，缓冲区 1 是 IO_Toggle1，那么此时就可以修改 IO_Toggle1。
		2、变量所在的 SRAM 区已经通过 MPU 配置为 WT 模式，更新变量 IO_Toggle 会立即写入。
		3、不配置 MPU 的话，也可以通过 Cahce 的函数 SCB_CleanDCache_by_Addr 做 Clean 操作。
	*/
}

else{

#endif

#endif	/* EN_BSP_TIM_DMA */

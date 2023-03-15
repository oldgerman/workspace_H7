/**
  ******************************************************************************
  * @file        bsp_dmamux_req_gen.cpp
  * @author      OldGerman
  * @created on  Mar 2, 2023
  * @brief       
  ******************************************************************************
  * @attention
  *
  * Copyright (C) 2022 OldGerman.
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see https://www.gnu.org/licenses/.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "bsp_config.h"
#ifdef EN_BSP_DMAMUX_REQ_GEN
#include "bsp.h"
#include "bsp_dmamux_req_gen.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Function implementations --------------------------------------------------*/
/**
 * @brief  设置DMA Request Generator单缓冲中断模式的参数
 * @param  hdma				  	 DMA句柄，支持BDMA、DMA1、DMA2，注意DMA能访问的内存域，建议用BDMA以节省其他高级DMA
 * @param  SrcAddress: 			 (内存)源缓冲区地址 The source memory Buffer address
 * @param  DstAddress:		 	 (外设)目标地址     The destination memory Buffer address
 * @param  DataLength: 			 (内存)源缓冲区的长度，单位取决于CubeMX配置
 * @param  XferHalfCpltCallback  缓冲区半传输完成回调
 * @param  XferCpltCallback  	 缓冲区传输完成回调
 * @retval HAL status
 */
HAL_StatusTypeDef bsp_DMA_REQ_GEN_Start_IT(
		DMA_HandleTypeDef *hdma,
		uint32_t SrcAddress,
		uint32_t DstAddress,
		uint32_t DataLength,
		void (*XferHalfCpltCallback)(DMA_HandleTypeDef * hdma),
		void (*XferCpltCallback)(DMA_HandleTypeDef * hdma))
{
	uint8_t status = HAL_OK;

	/* 注册回调函数 */
	status += (uint8_t)(HAL_DMA_RegisterCallback(hdma, HAL_DMA_XFER_HALFCPLT_CB_ID, XferHalfCpltCallback));
	status += (uint8_t)(HAL_DMA_RegisterCallback(hdma, HAL_DMA_XFER_CPLT_CB_ID, XferCpltCallback));

	/* 使能DMA请求发生器，注意CubeMX不会在自动生成的代码中调用此函数，需要用户自行在别处调用 */
	status += (uint8_t)(HAL_DMAEx_EnableMuxRequestGenerator (hdma));

	/* 启动DMA双缓冲传输，若不使能DMA请求发生器，则DMA无法开始传输 */
	status += (uint8_t)(HAL_DMA_Start_IT(hdma, SrcAddress, DstAddress, DataLength));

	return (HAL_StatusTypeDef)status;
}

/**
 * @brief  设置DMA Request Generator双缓冲中断模式的参数
 * @param  hdma				  	 DMA句柄，支持BDMA、DMA1、DMA2，注意DMA能访问的内存域，建议用BDMA以节省其他高级DMA
 * @param  SrcAddress: 			 源M0缓冲区地址 The source memory Buffer address
 * @param  DstAddress:		 	 目标地址     The destination memory Buffer address
 * @param  SecondMemAddress: 	 源M1缓冲区地址 The second memory Buffer address in case of multi buffer Transfer
 * @param  DataLength: 			 DMA从源向目标传输的长度，单位取决于CubeMX配置
 * @param  XferM0CpltCallback      M0缓冲区传输完成回调
 * @param  XferM1CpltCallback  	 M1缓冲区传输完成回调
 * @retval HAL status
 */
HAL_StatusTypeDef bsp_DMA_REQ_GEN_MultiBufferStart_IT(
		DMA_HandleTypeDef *hdma,
		uint32_t SrcAddress,
		uint32_t DstAddress,
		uint32_t SecondMemAddress,
		uint32_t DataLength,
		void (*XferM0CpltCallback)(DMA_HandleTypeDef * hdma),
		void (*XferM1CpltCallback)(DMA_HandleTypeDef * hdma))
{
	uint8_t status = HAL_OK;

	/* 注册回调函数 */
	status += (uint8_t)(HAL_DMA_RegisterCallback(hdma, HAL_DMA_XFER_CPLT_CB_ID, XferM0CpltCallback));			/*!< M0 Full transfer */
	status += (uint8_t)(HAL_DMA_RegisterCallback(hdma, HAL_DMA_XFER_M1CPLT_CB_ID, XferM1CpltCallback));		/*!< M1 Full Transfer */

	/* 使能DMA请求发生器，注意CubeMX不会在自动生成的代码中调用此函数，需要用户自行在别处调用 */
	status += (uint8_t)(HAL_DMAEx_EnableMuxRequestGenerator (hdma));

	/* 启动DMA双缓冲传输，若不使能DMA请求发生器，则DMA无法开始传输 */
	status += (uint8_t)(HAL_DMAEx_MultiBufferStart_IT(hdma, SrcAddress, DstAddress, SecondMemAddress, DataLength));

	return (HAL_StatusTypeDef)status;
}

#if 0
//回调函数示例模板
void bsp_DMA_REQ_GEN_XferCpltCallback(DMA_HandleTypeDef * hdma){
	/* 当前使用的缓冲 0 */
	/*
	 	armfly V7：43.2.4 DMA中断处理
		1、当前正在使用缓冲 1，此时可以动态修改缓冲 0 的数据。 比如缓冲区 0 是 IO_Toggle，缓冲区 1 是 IO_Toggle1，那么此时就可以修改 IO_Toggle。
		2、变量所在的 SRAM 区已经通过 MPU 配置为 WT 模式，更新变量 IO_Toggle 会立即写入。
		3、不配置 MPU 的话，也可以通过 Cahce 的函数 SCB_CleanDCache_by_Addr 做 Clean 操作。
	*/
	if(hdma == &hdma_dma_generator0){

	}
}

void bsp_DMA_REQ_GEN_XferM1CpltCallback(DMA_HandleTypeDef * hdma){
	/* 当前使用的缓冲 1 */
	/*
	    armfly V7：43.2.4 DMA中断处理
		1、当前正在使用缓冲 0，此时可以动态修改缓冲 1 的数据。 比如缓冲区 0 是 IO_Toggle，缓冲区 1 是 IO_Toggle1，那么此时就可以修改 IO_Toggle1。
		2、变量所在的 SRAM 区已经通过 MPU 配置为 WT 模式，更新变量 IO_Toggle 会立即写入。
		3、不配置 MPU 的话，也可以通过 Cahce 的函数 SCB_CleanDCache_by_Addr 做 Clean 操作。
	*/
	if(hdma == &hdma_dma_generator0){

	}
}
#endif


#endif

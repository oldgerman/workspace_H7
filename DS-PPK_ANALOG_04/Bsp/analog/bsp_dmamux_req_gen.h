/**
  ******************************************************************************
  * @file        bsp_dmamux_req_gen.h
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef BSP_DMAMUX_REQ_GEN_H_
#define BSP_DMAMUX_REQ_GEN_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal_conf.h"

/* Exported types ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
HAL_StatusTypeDef bsp_DMA_REQ_GEN_Start_IT(
		DMA_HandleTypeDef *hdma,
		uint32_t SrcAddress,
		uint32_t DstAddress,
		uint32_t DataLength,
		void (*XferHalfCpltCallback)(DMA_HandleTypeDef * hdma),
		void (*XferCpltCallback)(DMA_HandleTypeDef * hdma));

HAL_StatusTypeDef bsp_DMA_REQ_GEN_MultiBufferStart_IT(
		DMA_HandleTypeDef *hdma,
		uint32_t SrcAddress,
		uint32_t DstAddress,
		uint32_t SecondMemAddress,
		uint32_t DataLength,
		void (*XferCpltCallback)(DMA_HandleTypeDef * hdma),
		void (*XferM1CpltCallback)(DMA_HandleTypeDef * hdma));

#ifdef __cplusplus
}
#endif

#endif /* BSP_DMAMUX_REQ_GEN_H_ */

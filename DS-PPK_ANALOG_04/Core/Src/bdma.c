/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    bdma.c
  * @brief   This file provides code for the configuration
  *          of all the requested memory to memory DMA transfers.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "bdma.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure DMA                                                              */
/*----------------------------------------------------------------------------*/

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
DMA_HandleTypeDef hdma_bdma_generator0;

/**
  * Enable DMA controller clock
  * Configure DMA for memory to memory transfers
  *   hdma_bdma_generator0
  */
void MX_BDMA_Init(void)
{

  /* Local variables */
  HAL_DMA_MuxRequestGeneratorConfigTypeDef pRequestGeneratorConfig = {0};

  /* DMA controller clock enable */
  __HAL_RCC_BDMA_CLK_ENABLE();

  /* Configure DMA request hdma_bdma_generator0 on BDMA_Channel0 */
  hdma_bdma_generator0.Instance = BDMA_Channel0;
  hdma_bdma_generator0.Init.Request = BDMA_REQUEST_GENERATOR0;
  hdma_bdma_generator0.Init.Direction = DMA_PERIPH_TO_MEMORY;
  hdma_bdma_generator0.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_bdma_generator0.Init.MemInc = DMA_MINC_ENABLE;
  hdma_bdma_generator0.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  hdma_bdma_generator0.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
  hdma_bdma_generator0.Init.Mode = DMA_CIRCULAR;
  hdma_bdma_generator0.Init.Priority = DMA_PRIORITY_VERY_HIGH;
  if (HAL_DMA_Init(&hdma_bdma_generator0) != HAL_OK)
  {
    Error_Handler();
  }

  /* Configure the DMAMUX request generator for the selected BDMA channel */
  pRequestGeneratorConfig.SignalID = HAL_DMAMUX2_REQ_GEN_LPTIM3_OUT;
  pRequestGeneratorConfig.Polarity = HAL_DMAMUX_REQ_GEN_RISING;
  pRequestGeneratorConfig.RequestNumber = 1;
  if (HAL_DMAEx_ConfigMuxRequestGenerator(&hdma_bdma_generator0, &pRequestGeneratorConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /* DMA interrupt init */
  /* DMAMUX2_OVR_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMAMUX2_OVR_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMAMUX2_OVR_IRQn);
  /* BDMA_Channel0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(BDMA_Channel0_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(BDMA_Channel0_IRQn);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */


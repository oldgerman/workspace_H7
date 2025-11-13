/*
*********************************************************************************************************
*
*    模块名称 : emmc驱动模块
*    文件名称 : bsp_emmc.c
*    版    本 : V1.1
*    说    明 : emmc存储器底层驱动。根据 stm32h750b_discovery_mmc.c文件修改。
*
*    修改记录 :
*        版本号  日期        作者        说明
*        V1.0    2019-08-13  armfly      正式发布
*        V1.1    2020-09-17  baiyongbin  优化emmc驱动，改为4线模式
*
*    Copyright (C), 2018-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

/*
    1. 修改卡插入检测GPIO相关配置
    2. void SDMMC2_IRQHandler(void) 这个中断服务程序放到本文件末尾
*/

/**
  ******************************************************************************
  * @file    stm32h750b_discovery_mmc.c
  * @author  MCD Application Team
  * @brief   This file includes the EMMC driver mounted on STM32H750B-DISCOVERY
  *          board.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* File Info : -----------------------------------------------------------------
                                   User NOTES
1. How To use this driver:
--------------------------
   - This driver is used to drive the EMMC mounted on STM32H750B-DISCOVERY
     board.
   - This driver does not need a specific component driver for the EMMC device
     to be included with.

2. Driver description:
---------------------
  + Initialization steps:
     o Initialize the external EMMC memory using the BSP_MMC_Init() function. This
       function includes the MSP layer hardware resources initialization and the
       SDIO interface configuration to interface with the external EMMC. It
       also includes the EMMC initialization sequence.
     o The function BSP_MMC_GetCardInfo() is used to get the MMC information
       which is stored in the structure "HAL_MMC_CardInfoTypedef".

  + Micro MMC card operations
     o The micro MMC card can be accessed with read/write block(s) operations once
       it is ready for access. The access can be performed whether using the polling
       mode by calling the functions BSP_MMC_ReadBlocks()/BSP_MMC_WriteBlocks(), or by DMA
       transfer using the functions BSP_MMC_ReadBlocks_DMA()/BSP_MMC_WriteBlocks_DMA()
     o The DMA transfer complete is used with interrupt mode. Once the MMC transfer
       is complete, the MMC interrupt is handled using the function BSP_MMC_IRQHandler(),
       the DMA Tx/Rx transfer complete are handled using the functions
       MMC_DMA_Tx_IRQHandler()/MMC_DMA_Rx_IRQHandler() that should be defined by user.
       The corresponding user callbacks are implemented by the user at application level.
     o The MMC erase block(s) is performed using the function BSP_MMC_Erase() with specifying
       the number of blocks to erase.
     o The MMC runtime status is returned when calling the function BSP_MMC_GetStatus().

------------------------------------------------------------------------------*/

/* Includes ------------------------------------------------------------------*/
#include "bsp_emmc.h"
#include "nvic_prio_cfg.h"
#include "sdmmc.h"
/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32H750B_DISCOVERY
  * @{
  */

/** @defgroup STM32H750B_DISCOVERY_MMC STM32H750B_DISCOVERY_MMC
  * @{
  */

/** @defgroup STM32H750B_DISCOVERY_MMC_Exported_Variables Exported Variables
  * @{
  */

extern MMC_HandleTypeDef hmmc2;
/**
  * @}
  */

/** @defgroup STM32H750B_DISCOVERY_MMC_Exported_Functions  Exported Functions
  * @{
  */

/**
  * @brief  Initializes the MMC card device.
  * @retval MMC status
  */
uint8_t BSP_MMC_Init(void)
{
  uint8_t mmc_state = MMC_OK;

  BSP_MMC_DeInit();
    
  /* uMMC device interface configuration */
  hmmc2.Instance = SDMMC2;
  hmmc2.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  hmmc2.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  hmmc2.Init.BusWide = SDMMC_BUS_WIDE_4B;
  hmmc2.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_ENABLE;
  /* if CLKDIV = 0 then SDMMC Clock frequency = SDMMC Kernel Clock
     else SDMMC Clock frequency = SDMMC Kernel Clock / [2 * CLKDIV].
     200MHz / (2*2) = 50MHz
     200MHz / (2*3) = 33MHz 
  */
  hmmc2.Init.ClockDiv = 4;		/* 2019-12-13 2 -> 3   2021-03-22由3改为4*/
  if (HAL_MMC_Init(&hmmc2) != HAL_OK)
  {
    mmc_state = MMC_ERROR;
  }
  /* USER CODE BEGIN SDMMC2_Init 2 */

  /* SDIO的四线方式或8线方式 */
//  HAL_MMC_ConfigWideBusOperation(&hmmc2, SDMMC_BUS_WIDE_4B);
  HAL_MMC_ConfigWideBusOperation(&hmmc2, SDMMC_BUS_WIDE_8B);
  /* USER CODE END SDMMC2_Init 2 */

  return mmc_state;
}

/**
  * @brief  DeInitializes the MMC card device.
  * @retval MMC status
  */
uint8_t BSP_MMC_DeInit(void)
{

  uint8_t mmc_state = MMC_OK;

  hmmc2.Instance = SDMMC2;

  /* HAL MMC deinitialization */
  if (HAL_MMC_DeInit(&hmmc2) != HAL_OK)
  {
    mmc_state = MMC_ERROR;
  }

  /* Msp MMC deinitialization */
  hmmc2.Instance = SDMMC2;
  BSP_MMC_MspDeInit(&hmmc2);

  return mmc_state;
}

/**
  * @brief  Reads block(s) from a specified address in an MMC card, in polling mode.
  * @param  pData: Pointer to the buffer that will contain the data to transmit
  * @param  ReadAddr: Address from where data is to be read
  * @param  NumOfBlocks: Number of MMC blocks to read
  * @param  Timeout: Timeout for read operation
  * @retval MMC status
  */
uint8_t BSP_MMC_ReadBlocks(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks, uint32_t Timeout)
{

  if (HAL_MMC_ReadBlocks(&hmmc2, (uint8_t *)pData, ReadAddr, NumOfBlocks, Timeout) == HAL_OK)
  {
    return MMC_OK;
  }
  else
  {
    return MMC_ERROR;
  }
}

/**
  * @brief  Writes block(s) to a specified address in an MMC card, in polling mode.
  * @param  pData: Pointer to the buffer that will contain the data to transmit
  * @param  WriteAddr: Address from where data is to be written
  * @param  NumOfBlocks: Number of MMC blocks to write
  * @param  Timeout: Timeout for write operation
  * @retval MMC status
  */
uint8_t BSP_MMC_WriteBlocks(uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks, uint32_t Timeout)
{

  if (HAL_MMC_WriteBlocks(&hmmc2, (uint8_t *)pData, WriteAddr, NumOfBlocks, Timeout) == HAL_OK)
  {
    return MMC_OK;
  }
  else
  {
    return MMC_ERROR;
  }
}

/**
  * @brief  Reads block(s) from a specified address in an MMC card, in DMA mode.
  * @param  pData: Pointer to the buffer that will contain the data to transmit
  * @param  ReadAddr: Address from where data is to be read
  * @param  NumOfBlocks: Number of MMC blocks to read
  * @retval MMC status
  */
uint8_t BSP_MMC_ReadBlocks_DMA(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks)
{

  if (HAL_MMC_ReadBlocks_DMA(&hmmc2, (uint8_t *)pData, ReadAddr, NumOfBlocks) == HAL_OK)
  {
    return MMC_OK;
  }
  else
  {
    return MMC_ERROR;
  }
}

/**
  * @brief  Writes block(s) to a specified address in an MMC card, in DMA mode.
  * @param  pData: Pointer to the buffer that will contain the data to transmit
  * @param  WriteAddr: Address from where data is to be written
  * @param  NumOfBlocks: Number of MMC blocks to write
  * @retval MMC status
  */
uint8_t BSP_MMC_WriteBlocks_DMA(uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks)
{

  if (HAL_MMC_WriteBlocks_DMA(&hmmc2, (uint8_t *)pData, WriteAddr, NumOfBlocks) == HAL_OK)
  {
    return MMC_OK;
  }
  else
  {
    return MMC_ERROR;
  }
}

/**
  * @brief  Erases the specified memory area of the given MMC card.
  * @param  StartAddr: Start byte address
  * @param  EndAddr: End byte address
  * @retval MMC status
  */
uint8_t BSP_MMC_Erase(uint32_t StartAddr, uint32_t EndAddr)
{

  if (HAL_MMC_Erase(&hmmc2, StartAddr, EndAddr) == HAL_OK)
  {
    return MMC_OK;
  }
  else
  {
    return MMC_ERROR;
  }
}

/**
  * @brief  Initializes the MMC MSP.
  * @param  hmmc: MMC handle
  * @retval None
  */
void BSP_MMC_MspInit(MMC_HandleTypeDef *hmmc2)
{

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    if(hmmc2->Instance==SDMMC2)
    {
    /* USER CODE BEGIN SDMMC2_MspInit 0 */

    /* USER CODE END SDMMC2_MspInit 0 */

    /** Initializes the peripherals clock
    */
      PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SDMMC;
      PeriphClkInitStruct.PLL2.PLL2M = 25;
      PeriphClkInitStruct.PLL2.PLL2N = 200;
      PeriphClkInitStruct.PLL2.PLL2P = 2;
      PeriphClkInitStruct.PLL2.PLL2Q = 2;
      PeriphClkInitStruct.PLL2.PLL2R = 1;
      PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_0;
      PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
      PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
      PeriphClkInitStruct.SdmmcClockSelection = RCC_SDMMCCLKSOURCE_PLL2;
      if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
      {
        Error_Handler();
      }

      /* SDMMC2 clock enable */
      __HAL_RCC_SDMMC2_CLK_ENABLE();

      __HAL_RCC_GPIOB_CLK_ENABLE();
      __HAL_RCC_GPIOD_CLK_ENABLE();
      __HAL_RCC_GPIOC_CLK_ENABLE();
      /**SDMMC2 GPIO Configuration
      PB8     ------> SDMMC2_D4
      PB4 (NJTRST)     ------> SDMMC2_D3
      PB3 (JTDO/TRACESWO)     ------> SDMMC2_D2
      PD7     ------> SDMMC2_CMD
      PB9     ------> SDMMC2_D5
      PD6     ------> SDMMC2_CK
      PC7     ------> SDMMC2_D7
      PC6     ------> SDMMC2_D6
      PB14     ------> SDMMC2_D0
      PB15     ------> SDMMC2_D1
      */
      GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
      GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
      GPIO_InitStruct.Alternate = GPIO_AF10_SDIO2;
      HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

      GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_3|GPIO_PIN_14|GPIO_PIN_15;
      GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
      GPIO_InitStruct.Alternate = GPIO_AF9_SDIO2;
      HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

      GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_6;
      GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
      GPIO_InitStruct.Alternate = GPIO_AF11_SDIO2;
      HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

      GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_6;
      GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
      GPIO_InitStruct.Alternate = GPIO_AF10_SDIO2;
      HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

      /* SDMMC2 interrupt Init */
      HAL_NVIC_SetPriority(SDMMC2_IRQn, 5, 0);
      HAL_NVIC_EnableIRQ(SDMMC2_IRQn);
    /* USER CODE BEGIN SDMMC2_MspInit 1 */

    /* USER CODE END SDMMC2_MspInit 1 */
    }
}

/**
  * @brief  DeInitializes the MMC MSP.
  * @param  hmmc: MMC handle
  * @retval None
  */
void BSP_MMC_MspDeInit(MMC_HandleTypeDef *hmmc2)
{

    if(hmmc2->Instance==SDMMC2)
    {
    /* USER CODE BEGIN SDMMC2_MspDeInit 0 */

    /* USER CODE END SDMMC2_MspDeInit 0 */
      /* Peripheral clock disable */
      __HAL_RCC_SDMMC2_CLK_DISABLE();

      /**SDMMC2 GPIO Configuration
      PB8     ------> SDMMC2_D4
      PB4 (NJTRST)     ------> SDMMC2_D3
      PB3 (JTDO/TRACESWO)     ------> SDMMC2_D2
      PD7     ------> SDMMC2_CMD
      PB9     ------> SDMMC2_D5
      PD6     ------> SDMMC2_CK
      PC7     ------> SDMMC2_D7
      PC6     ------> SDMMC2_D6
      PB14     ------> SDMMC2_D0
      PB15     ------> SDMMC2_D1
      */
      HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8|GPIO_PIN_4|GPIO_PIN_3|GPIO_PIN_9
                            |GPIO_PIN_14|GPIO_PIN_15);

      HAL_GPIO_DeInit(GPIOD, GPIO_PIN_7|GPIO_PIN_6);

      HAL_GPIO_DeInit(GPIOC, GPIO_PIN_7|GPIO_PIN_6);

      /* SDMMC2 interrupt Deinit */
      HAL_NVIC_DisableIRQ(SDMMC2_IRQn);
    /* USER CODE BEGIN SDMMC2_MspDeInit 1 */

    /* USER CODE END SDMMC2_MspDeInit 1 */
    }
}

/**
  * @brief  Handles MMC card interrupt request.
  * @retval None
  */
//void BSP_MMC_IRQHandler(void)
//{
//  HAL_MMC_IRQHandler(&hmmc2);
//}
//
//void SDMMC2_IRQHandler(void)
//{
//  HAL_MMC_IRQHandler(&hmmc2);
//}

/**
  * @brief  Gets the current MMC card data status.
  * @retval Data transfer state.
  *          This value can be one of the following values:
  *            @arg  MMC_TRANSFER_OK: No data transfer is acting
  *            @arg  MMC_TRANSFER_BUSY: Data transfer is acting
  *            @arg  MMC_TRANSFER_ERROR: Data transfer error
  */
uint8_t BSP_MMC_GetCardState(void)
{
  return ((HAL_MMC_GetCardState(&hmmc2) == HAL_MMC_CARD_TRANSFER) ? MMC_TRANSFER_OK : MMC_TRANSFER_BUSY);
}

/**
  * @brief  Get MMC information about specific MMC card.
  * @param  CardInfo: Pointer to HAL_MMC_CardInfoTypedef structure
  * @retval None
  */
void BSP_MMC_GetCardInfo(BSP_MMC_CardInfo *CardInfo)
{
  HAL_MMC_GetCardInfo(&hmmc2, CardInfo);
}

/**
  * @brief MMC Abort callbacks
  * @param hmmc: MMC handle
  * @retval None
  */
void HAL_MMC_AbortCallback(MMC_HandleTypeDef *hmmc)
{
  BSP_MMC_AbortCallback();
}

/**
  * @brief Tx Transfer completed callbacks
  * @param hmmc: MMC handle
  * @retval None
  */
//void HAL_MMC_TxCpltCallback(MMC_HandleTypeDef *hmmc)
//{
//  BSP_MMC_WriteCpltCallback();
//}

/**
  * @brief Rx Transfer completed callbacks
  * @param hmmc: MMC handle
  * @retval None
  */
//void HAL_MMC_RxCpltCallback(MMC_HandleTypeDef *hmmc)
//{
//  BSP_MMC_ReadCpltCallback();
//}

/**
  * @brief BSP MMC Abort callbacks
  * @retval None
  */
__weak void BSP_MMC_AbortCallback(void)
{
}

/**
  * @brief BSP Tx Transfer completed callbacks
  * @retval None
  */
__weak void BSP_MMC_WriteCpltCallback(void)
{
}

/**
  * @brief BSP Rx Transfer completed callbacks
  * @retval None
  */
__weak void BSP_MMC_ReadCpltCallback(void)
{
}
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
//#define printf_ok            printf
#define printf_ok(...)

#define printf_err            printf
//#define printf_err(...)

#define STORAGE_LUN_NBR                  1        // 3
//#define STORAGE_BLK_NBR                  0x10000
//#define STORAGE_BLK_SIZ                  0x200

#define LUN_SDRAM    0
#define LUN_SD        0
//#define LUN_NAND    2

HAL_MMC_CardInfoTypeDef g_emmcInfo;
/**
  * @brief  Returns the medium capacity.
  * @param  lun: Logical unit number
  * @param  block_num: Number of total block number
  * @param  block_size: Block size
  * @retval Status (0: OK / -1: Error)
  */
int8_t STORAGE_GetCapacity(uint8_t lun, uint32_t * block_num,
                           uint16_t * block_size)
{
    int8_t ret = -1;

    switch (lun)
    {
        case LUN_SD:
            {
                BSP_MMC_GetCardInfo(&g_emmcInfo);
                *block_num = g_emmcInfo.LogBlockNbr - 1;
                *block_size = g_emmcInfo.LogBlockSize;
                ret = 0;

                printf_ok("STORAGE_GetCapacity ^%d, %d\r\n", *block_num, *block_size);
            }
            break;
    }
    return ret;
}

/**
  * @brief  Checks whether the medium is ready.
  * @param  lun: Logical unit number
  * @retval Status (0: OK / -1: Error)
  */
int8_t STORAGE_IsReady(uint8_t lun)
{
    int8_t ret = -1;

    switch (lun)
    {
        case LUN_SD:
            {
                if (BSP_MMC_GetCardState() == MMC_TRANSFER_OK)
                {
                    ret = 0;
                }
            }
            break;
    }
    return ret;
}


//uint32_t        WRITE_DATA_BUF         [512U/4 * 100]    __attribute__((section(".RAM_D1_Array")));
//uint32_t        READ_DATA_BUF         [512U/4 * 100]    __attribute__((section(".RAM_D1_Array")));
/**
  * @brief  Returns the medium capacity.
  *        先以16位的数据宽度写入数据，再读取出来一一进行比较，随后以8位的数据宽度写入，
*                用以验证NBL0和NBL1两个引脚的连接是否正常。
  * @param  sdmmc_bus_wide: SMMC位宽
  * @param  write_block_num: 写块数量
  * @param  read_Block_Num:  读块数量
  * @param  test_times  测试次数
  * @retval Status (0: OK / -1: Error)
  */
#if 0
uint8_t eMMC_Test(uint32_t sdmmc_bus_wide, uint32_t write_block_num, uint32_t read_Block_Num, uint32_t test_times)
{
    uint32_t last_sdmmc_bus_wide = sdmmc_bus_wide;
    uint32_t i = 0;         // 计数变量
    uint32_t ExecutionTime_Begin;       // 开始时间
    uint32_t ExecutionTime_End;         // 结束时间
    uint32_t ExecutionTime;             // 执行时间
    float    ExecutionSpeed;            // 执行速度

    printf("*****************************************************************************************************\r\n");
    printf("进行 eMMC 读写速度测试>>>\r\n");

    // 获取信息
    int8_t ret = 0;
    uint32_t block_num;
    uint16_t block_size;
    ret = STORAGE_GetCapacity(0, &block_num, &block_size);
    printf("eMMC容量 = %dMB, 块数量 = %ld, 块大小 = %d\r\n", block_num*block_size/1024/1024, block_num, block_size);
//    ret += STORAGE_IsReady(0);
//    if(ret != 0)
//    {
//        printf("eMMC初始化失败！\r\n");
//        return 1;
//    }

    // 初始化写入缓冲区
    uint32_t read_buf_size = sizeof(READ_DATA_BUF);
    uint32_t write_buf_size = sizeof(WRITE_DATA_BUF);
    uint32_t write_buf_elements_num = sizeof(WRITE_DATA_BUF)/sizeof(WRITE_DATA_BUF[0]);
    for(i = 0; i < write_buf_elements_num; i++) {
        WRITE_DATA_BUF[i] = i; // 128x100 = 12800
    }

// 写入 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/**
 * 纯裸机读写，没有使用文件系统。
8线SDIO，DMA方式，50MHz时钟频率SDR，读出操作每次200个block，写操作每次100个block，读写都是测试100次，求平均
 */
    ExecutionTime_Begin     = HAL_GetTick();    // 获取 systick 当前时间，单位ms

    for(uint32_t j = 0; j < test_times; j++) {
        BSP_MMC_WriteBlocks(WRITE_DATA_BUF, 0/*addr*/, write_block_num/*num of blocks*/, 5000/*timeout*/);
    }

    ExecutionTime_End       = HAL_GetTick();                                            // 获取 systick 当前时间，单位ms

    ExecutionTime  = ExecutionTime_End - ExecutionTime_Begin;               // 计算擦除时间，单位ms
    float total_write_buf_size_MB = (float)write_buf_size* test_times /1024/1024;
    ExecutionSpeed = total_write_buf_size_MB / ExecutionTime*1000;     // 计算速度，单位 MB/S
    printf("8位写入数据完毕，大小：%.2f MB，耗时: %d ms, 写入速度：%.2f MB/s\r\n",
            total_write_buf_size_MB,
            ExecutionTime,
            ExecutionSpeed);

// 读取   >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    ExecutionTime_Begin     = HAL_GetTick();    // 获取 systick 当前时间，单位ms

    for(uint32_t j = 0; j < test_times; j++) {
        BSP_MMC_ReadBlocks(READ_DATA_BUF, 0/*addr*/, read_Block_Num/*num of blocks*/, 5000/*timeout*/);
    }

    ExecutionTime_End       = HAL_GetTick();                                            // 获取 systick 当前时间，单位ms

    ExecutionTime  = ExecutionTime_End - ExecutionTime_Begin;               // 计算擦除时间，单位ms
    float total_read_buf_size_MB =  (float)read_buf_size* test_times /1024/1024;
    ExecutionSpeed = total_read_buf_size_MB /ExecutionTime*1000;     // 计算速度，单位 MB/S

    printf("8位读取数据完毕，大小：%.2f MB，耗时: %d ms, 读取速度：%.2f MB/s\r\n",
            total_read_buf_size_MB,
            ExecutionTime,
            ExecutionSpeed);

// 数据校验 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>C

    printf("*****************************************************************************************************\r\n");
    printf("进行数据校验>>>\r\n");

    for(i = 0; i < write_buf_elements_num; i++ )
    {
        if( WRITE_DATA_BUF[i] != READ_DATA_BUF[i] )      //检测数据，若不相等，跳出函数,返回检测失败结果。
        {
            printf("校验出错，终止测试！\r\n");
            return ERROR;    // 返回失败标志
        }
    }
    printf("数据校验通过，终止测试！\r\n");

    // 还原 SMMC 位宽
    HAL_MMC_ConfigWideBusOperation(&hmmc2, last_sdmmc_bus_wide);

    return SUCCESS;  // 返回成功标志
}
#endif



/**
  ******************************************************************************
  * @file    MMC/MMC_ReadWrite_IT/Src/main.c
  * @author  MCD Application Team
  * @brief   This example describes how to configure and use MMC through
  *          the STM32H7xx HAL API.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/** @addtogroup STM32H7xx_HAL_Examples
  * @{
  */

/** @addtogroup MMC_ReadWrite_IT
  * @{
  */

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define DATA_SIZE              ((uint32_t)0x06400000U) /* Data Size 100Mo */

/* ------ Buffer Size ------ */
#define BLOCKSIZE               512U
#define BUFFER_SIZE            ((uint32_t)0x00040000U) /* 256Ko */

#define NB_BUFFER              DATA_SIZE / BUFFER_SIZE
#define NB_BLOCK_BUFFER        BUFFER_SIZE / BLOCKSIZE /* Number of Block (512o) by Buffer */
#define BUFFER_WORD_SIZE       (BUFFER_SIZE>>2)        /* Buffer size in Word */


#define MMC_TIMEOUT            ((uint32_t)0x00100000U)
#define ADDRESS                ((uint32_t)0x00000400U) /* MMC Address to write/read data */
#define DATA_PATTERN           ((uint32_t)0xB5F3A5F3U) /* Data pattern to write */
/* Size of buffers */
#define BUFFERSIZE                 (COUNTOF(aTxBuffer) - 1)
#define COUNTOF(__BUFFER__)        (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
__IO uint8_t RxCplt, TxCplt;

/******** MMC Transmission Buffer definition *******/
uint8_t aTxBuffer[BUFFER_WORD_SIZE*4] __attribute__((section(".RAM_D1_Array")));
/**************************************************/

/******** MMC Receive Buffer definition *******/
uint8_t aRxBuffer[BUFFER_WORD_SIZE*4] __attribute__((section(".RAM_D1_Array")));
/**************************************************/

/* UART handler declaration, used for printf */
//UART_HandleTypeDef UartHandle;

//__IO uint8_t step = 0;
uint32_t start_time = 0;
uint32_t stop_time = 0;
/* Private function prototypes -----------------------------------------------*/

static void My_Error_Handler(void);
static uint8_t Wait_MMCCARD_Ready(void);


#ifdef __GNUC__ /* __GNUC__ */
 #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
 #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

/* Private functions ---------------------------------------------------------*/
void Fill_Buffer(uint32_t *pBuffer, uint16_t BufferLength, uint32_t Offset);

/**
 * @brief  Main program
 * @param  None
 * @retval None
 */
void eMMC_Test_IT(uint32_t sdmmc_bus_wide)
{
    /* Output a message on Hyperterminal using printf function */
    printf("####################################################### \r\n");
    printf("#        eMMC Example: Write Read with IT mode       # \r\n");
    printf("####################################################### \r\n");

    uint32_t last_sdmmc_bus_wide = sdmmc_bus_wide;
    uint32_t index = 0;
    HAL_MMC_CardCIDTypeDef pCID;
    HAL_MMC_CardCSDTypeDef pCSD;

    if(HAL_MMC_ConfigWideBusOperation(&hmmc2,last_sdmmc_bus_wide) != HAL_OK)
    {
        My_Error_Handler();
    }

    if(HAL_MMC_Erase(&hmmc2, ADDRESS, ADDRESS+BUFFERSIZE) != HAL_OK)
    {
        My_Error_Handler();
    }
    if(Wait_MMCCARD_Ready() != HAL_OK)
    {
        My_Error_Handler();
    }

    HAL_MMC_GetCardCID(&hmmc2, &pCID);
    HAL_MMC_GetCardCSD(&hmmc2, &pCSD);

    uint8_t step = 0;
    while(1) {
    switch(step)
    {
        case 0:
        {
            /*##- 4 - Initialize Transmission buffer #####################*/
            for (index = 0; index < BUFFERSIZE; index++)
            {
                aTxBuffer[index] = DATA_PATTERN + index;
            }
            SCB_CleanDCache_by_Addr((uint32_t*)aTxBuffer, BUFFER_WORD_SIZE*4);
            printf(" ****************** Start Write test ******************* \r\n");
            printf(" - Buffer size to write: %lu MB   \r\n", (DATA_SIZE>>20));
            index = 0;
            start_time = HAL_GetTick();
            step++;
        }
        break;
        case 1:
        {
            TxCplt = 0;

            if(Wait_MMCCARD_Ready() != HAL_OK)
            {
                My_Error_Handler();
            }
            /*##- 5 - Start Transmission buffer #####################*/
            if(HAL_MMC_WriteBlocks_IT(&hmmc2, aTxBuffer, ADDRESS, NB_BLOCK_BUFFER) != HAL_OK)
            {
                My_Error_Handler();
            }
            step++;
        }
        break;
        case 2:
        {
            if(TxCplt != 0)
            {
                /* Transfer of Buffer completed */
                index++;
                if(index<NB_BUFFER)
                {
                    /* More data need to be trasnfered */
                    step--;
                }
                else
                {
                    stop_time = HAL_GetTick();
                    printf(" - Write Time(ms): %lu  -  Write Speed: %02.2f MB/s  \r\n", stop_time - start_time, (float)((float)(DATA_SIZE>>10)/(float)(stop_time - start_time)));
                    /* All data are transfered */
                    step++;
                }
            }
        }
        break;
        case 3:
        {
            /*##- 6 - Initialize Reception buffer #####################*/
            for (index = 0; index < BUFFERSIZE; index++)
            {
                aRxBuffer[index] = 0;
            }
            SCB_CleanDCache_by_Addr((uint32_t*)aRxBuffer, BUFFER_WORD_SIZE*4);
            printf(" ******************* Start Read test ******************* \r\n");
            printf(" - Buffer size to read: %lu MB   \r\n", (DATA_SIZE>>20));
            start_time = HAL_GetTick();
            index = 0;
            step++;
        }
        break;
        case 4:
        {
            if(Wait_MMCCARD_Ready() != HAL_OK)
            {
                My_Error_Handler();
            }
            /*##- 7 - Initialize Reception buffer #####################*/
            RxCplt = 0;
            if(HAL_MMC_ReadBlocks_IT(&hmmc2, aRxBuffer, ADDRESS, NB_BLOCK_BUFFER) != HAL_OK)
            {
                My_Error_Handler();
            }
            step++;
        }
        break;
        case 5:
        {
            if(RxCplt != 0)
            {
                /* Transfer of Buffer completed */
                index++;
                if(index<NB_BUFFER)
                {
                    /* More data need to be trasnfered */
                    step--;
                }
                else
                {
                    stop_time = HAL_GetTick();
                    printf(" - Read Time(ms): %lu  -  Read Speed: %02.2f MB/s  \r\n", stop_time - start_time, (float)((float)(DATA_SIZE>>10)/(float)(stop_time - start_time)));
                    /* All data are transfered */
                    step++;
                }
            }
        }
        break;
        case 6:
        {
            /*##- 8 - Check Reception buffer #####################*/
            index=0;
            printf(" ********************* Check data ********************** \r\n");
            while((index<BUFFERSIZE) && (aRxBuffer[index] == aTxBuffer[index]))
            {
                index++;
            }

            if(index != BUFFERSIZE)
            {
                printf(" - Check data Error !!!!   \r\n");
                My_Error_Handler();
            }
            printf(" - Check data OK  \r\n");
            step = 88; //!!!!
        }
        break;
        default :
            My_Error_Handler();
        }
        if(step == 88)  //!!!!
            break;
    }
    // 还原 SMMC 位宽
    HAL_MMC_ConfigWideBusOperation(&hmmc2, last_sdmmc_bus_wide);
}


/**
 * @brief  Main program
 * @param  None
 * @retval None
 */
void eMMC_Test_DMA(uint32_t sdmmc_bus_wide)
{
    /* Output a message on Hyperterminal using printf function */
    printf("####################################################### \r\n");
    printf("#        eMMC Example: Write Read with DMA mode       # \r\n");
    printf("####################################################### \r\n");

    uint32_t last_sdmmc_bus_wide = sdmmc_bus_wide;
    uint32_t index = 0;
    HAL_MMC_CardCIDTypeDef pCID;
    HAL_MMC_CardCSDTypeDef pCSD;

    if(HAL_MMC_ConfigWideBusOperation(&hmmc2,last_sdmmc_bus_wide) != HAL_OK)
    {
        My_Error_Handler();
    }

    if(HAL_MMC_Erase(&hmmc2, ADDRESS, ADDRESS+BUFFERSIZE) != HAL_OK)
    {
        My_Error_Handler();
    }
    if(Wait_MMCCARD_Ready() != HAL_OK)
    {
        My_Error_Handler();
    }

    HAL_MMC_GetCardCID(&hmmc2, &pCID);
    HAL_MMC_GetCardCSD(&hmmc2, &pCSD);

    uint8_t step = 0;
    while(1) {
    switch(step)
    {
        case 0:
        {
            /*##- 4 - Initialize Transmission buffer #####################*/
            for (index = 0; index < BUFFERSIZE; index++)
            {
                aTxBuffer[index] = DATA_PATTERN + index;
            }
            SCB_CleanDCache_by_Addr((uint32_t*)aTxBuffer, BUFFER_WORD_SIZE*4);
            printf(" ****************** Start Write test ******************* \r\n");
            printf(" - Buffer size to write: %lu MB   \r\n", (DATA_SIZE>>20));
            index = 0;
            start_time = HAL_GetTick();
            step++;
        }
        break;
        case 1:
        {
            TxCplt = 0;

            if(Wait_MMCCARD_Ready() != HAL_OK)
            {
                My_Error_Handler();
            }
            /*##- 5 - Start Transmission buffer #####################*/
            if(HAL_MMC_WriteBlocks_DMA(&hmmc2, aTxBuffer, ADDRESS, NB_BLOCK_BUFFER) != HAL_OK)
            {
                My_Error_Handler();
            }
            step++;
        }
        break;
        case 2:
        {
            if(TxCplt != 0)
            {
                /* Transfer of Buffer completed */
                index++;
                if(index<NB_BUFFER)
                {
                    /* More data need to be trasnfered */
                    step--;
                }
                else
                {
                    stop_time = HAL_GetTick();
                    printf(" - Write Time(ms): %lu  -  Write Speed: %02.2f MB/s  \r\n", stop_time - start_time, (float)((float)(DATA_SIZE>>10)/(float)(stop_time - start_time)));
                    /* All data are transfered */
                    step++;
                }
            }
        }
        break;
        case 3:
        {
            /*##- 6 - Initialize Reception buffer #####################*/
            for (index = 0; index < BUFFERSIZE; index++)
            {
                aRxBuffer[index] = 0;
            }
            SCB_CleanDCache_by_Addr((uint32_t*)aRxBuffer, BUFFER_WORD_SIZE*4);
            printf(" ******************* Start Read test ******************* \r\n");
            printf(" - Buffer size to read: %lu MB   \r\n", (DATA_SIZE>>20));
            start_time = HAL_GetTick();
            index = 0;
            step++;
        }
        break;
        case 4:
        {
            if(Wait_MMCCARD_Ready() != HAL_OK)
            {
                My_Error_Handler();
            }
            /*##- 7 - Initialize Reception buffer #####################*/
            RxCplt = 0;
            if(HAL_MMC_ReadBlocks_DMA(&hmmc2, aRxBuffer, ADDRESS, NB_BLOCK_BUFFER) != HAL_OK)
            {
                My_Error_Handler();
            }
            step++;
        }
        break;
        case 5:
        {
            if(RxCplt != 0)
            {
                /* Transfer of Buffer completed */
                index++;
                if(index<NB_BUFFER)
                {
                    /* More data need to be trasnfered */
                    step--;
                }
                else
                {
                    stop_time = HAL_GetTick();
                    printf(" - Read Time(ms): %lu  -  Read Speed: %02.2f MB/s  \r\n", stop_time - start_time, (float)((float)(DATA_SIZE>>10)/(float)(stop_time - start_time)));
                    /* All data are transfered */
                    step++;
                }
            }
        }
        break;
        case 6:
        {
            /*##- 8 - Check Reception buffer #####################*/
            index=0;
            printf(" ********************* Check data ********************** \r\n");
            while((index<BUFFERSIZE) && (aRxBuffer[index] == aTxBuffer[index]))
            {
                index++;
            }

            if(index != BUFFERSIZE)
            {
                printf(" - Check data Error !!!!   \r\n");
                My_Error_Handler();
            }
            printf(" - Check data OK  \r\n");
            step = 88; //!!!!
        }
        break;
        default :
            My_Error_Handler();
        }
        if(step == 88)  //!!!!
            break;
    }
    // 还原 SMMC 位宽
    HAL_MMC_ConfigWideBusOperation(&hmmc2, last_sdmmc_bus_wide);
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void My_Error_Handler(void)
{
  printf(" - Error \r\n");
  while(1)
  {
  }
}



/**
 * @brief Rx Transfer completed callbacks
 * @param hmmc: MMC handle
 * @retval None
 */
void HAL_MMC_RxCpltCallback(MMC_HandleTypeDef *hmmc)
{
    RxCplt=1;
}

/**
 * @brief Tx Transfer completed callbacks
 * @param hmmc: MMC handle
 * @retval None
 */
void HAL_MMC_TxCpltCallback(MMC_HandleTypeDef *hmmc)
{
    TxCplt=1;
}

/**
 * @brief MMC error callbacks
 * @param hmmc: MMC handle
 * @retval None
 */
void HAL_MMC_ErrorCallback(MMC_HandleTypeDef *hmmc)
{
    Error_Handler();
}


/**
 * @brief  Fills the goal 32-bit length buffer
 * @param  pBuffer: pointer on the Buffer to fill
 * @param  BufferLength: length of the buffer to fill
 * @param  Offset: first value to fill on the Buffer
 * @retval None
 */
void Fill_Buffer(uint32_t *pBuffer, uint16_t BufferLength, uint32_t Offset)
{
    uint16_t IndexTmp;

    /* Put in global buffer same values */
    for ( IndexTmp = 0; IndexTmp < BufferLength; IndexTmp++ )
    {
        pBuffer[IndexTmp] = IndexTmp + Offset;
    }
}


/**
 * @brief  Wait MMC Card ready status
 * @param  None
 * @retval None
 */
static uint8_t Wait_MMCCARD_Ready(void)
{
    uint32_t loop = MMC_TIMEOUT;

    /* Wait for the Erasing process is completed */
    /* Verify that MMC card is ready to use after the Erase */
    while(loop > 0)
    {
        loop--;
        if(HAL_MMC_GetCardState(&hmmc2) == HAL_MMC_CARD_TRANSFER)
        {
            return HAL_OK;
        }
    }
    return HAL_ERROR;
}

#ifdef  USE_FULL_ASSERT

/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while (1)
    {
    }
}
#endif

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

/**
  ******************************************************************************
  * @file    quadspi.h
  * @brief   This file contains all the function prototypes for
  *          the quadspi.c file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __QUADSPI_H__
#define __QUADSPI_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern QSPI_HandleTypeDef hqspi;

/* USER CODE BEGIN Private defines */

uint8_t CSP_QUADSPI_Init(void);
uint8_t CSP_QSPI_EraseSector(uint32_t EraseStartAddress ,uint32_t EraseEndAddress);
uint8_t CSP_QSPI_WriteMemory(uint8_t* buffer, uint32_t address, uint32_t buffer_size);
uint8_t CSP_QSPI_EnableMemoryMappedMode(void);
uint8_t CSP_QSPI_Erase_Chip (void);

/* USER CODE END Private defines */

void MX_QUADSPI_Init(void);

/* USER CODE BEGIN Prototypes */
/* W25Qxx不同容量�?2^n Byte表示*/
#ifndef QSPI_W25Q16_SIZE_2N
#define QSPI_W25Q16_SIZE_2N 21
#endif
#ifndef QSPI_W25Q32_SIZE_2N
#define QSPI_W25Q32_SIZE_2N 22
#endif
#ifndef QSPI_W25Q64_SIZE_2N
#define QSPI_W25Q64_SIZE_2N 23
#endif
#ifndef QSPI_W25Q128_SIZE_2N
#define QSPI_W25Q128_SIZE_2N 24
#endif
#ifndef QSPI_W25Q256_SIZE_2N
#define QSPI_W25Q256_SIZE_2N 25
#endif
#ifndef QSPI_W25Q512_SIZE_2N
#define QSPI_W25Q512_SIZE_2N 26
#endif

/*W25Qxx memory parameters*/
#define QSPI_FLASH_SIZE_2N    		QSPI_W25Q64_SIZE_2N    /* Flash大小�?2^QSPI_W25Q64_SIZE = 8MB*/
#define QSPI_FLASH_SIZE_MB			(1 << (QSPI_FLASH_SIZE_2N - 20))
#define QSPI_FLASH_SIZE_MBits		(QSPI_FLASH_SIZE_MB * 8)
#define MEMORY_FLASH_SIZE			(QSPI_FLASH_SIZE_MB * 1024 * 1024)
#define MEMORY_BLOCK_SIZE			0x10000   /* 65536 => 1024 sectors of 64KBytes//	1024�?64KB大小的BLOCK, W25Q系列适用*/
#define MEMORY_SECTOR_SIZE			0x1000    /* 4096 Byte => 4KB //	每个块有16个SECTOR*/
#define MEMORY_PAGE_SIZE			0x100    /* 262144 pages of 256 bytes */

/*超时时间ms*/
//#define W25Q_TIMEOUT 10000	//.c用HAL_QPSI_TIMEOUT_DEFAULT_VALUE 5000

/*W25Qxx commands */
#define WRITE_ENABLE_CMD 0x06
#define READ_STATUS_REG_CMD 0x05
#define READ_FLAG_STATUS_REG_CMD 0x05//0x70
#define WRITE_STATUS_REG_CMD 0x01
#define SECTOR_ERASE_CMD 0x20		//! 20h 21h
#define CHIP_ERASE_CMD 0xC7
#define QUAD_IN_FAST_PROG_CMD 0x32	//! 32h 34h
//#define READ_CONFIGURATION_REG_CMD 0x61//0x15	//不支�?
//#define WRITE_CONFIGURATION_REG_CMD 0x65//		//不支�?
#define QUAD_READ_IO_CMD 0xEB		//! EBh ECh
#define QUAD_OUT_FAST_READ_CMD 0x6B	//! 64JV: Fast Read Quad Output 6Bh //256JV 6CH

#define QPI_ENABLE_CMD 0x35			//64JV不支持ADS位，但是支持这个命令
//#define DISABLE_QIP_MODE 0xf5		//W25Q不支持，�?.c没用�?

#define DUMMY_CLOCK_CYCLES_READ_QUAD 6
#define RESET_ENABLE_CMD 0x66
#define RESET_EXECUTE_CMD 0x99

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __QUADSPI_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

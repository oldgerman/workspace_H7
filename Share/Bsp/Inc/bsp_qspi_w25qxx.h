/*
 * bsp_qspi_w25qxx.h
 *
 *  Created on: 2022年8月20日
 *      Author: OldGerman (过气德国佬)
 *
 *  Modified:
 *      2022/08/21: 适配W25Q16~W25Q512
 */

#ifndef BSP_QSPI_W25QXX_H_
#define BSP_QSPI_W25QXX_H_

#ifdef __cplusplus
extern "C" {
#endif
/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */
/*
 * 备注CSP前缀：
 * 	在计算机科学中，交談循序程式（英語：Communicating sequential processes，縮寫為CSP），
 * 	又譯為通信顺序进程、交換訊息的循序程式，是一種形式語言，用來描述並行性系統間進行互動的模式
 */
uint8_t CSP_QUADSPI_Init(void);
uint8_t CSP_QSPI_EraseSector(uint32_t EraseStartAddress ,uint32_t EraseEndAddress);
uint8_t CSP_QSPI_WriteMemory(uint8_t* buffer, uint32_t address, uint32_t buffer_size);
uint8_t CSP_QSPI_EnableMemoryMappedMode(void);
uint8_t CSP_QSPI_Erase_Chip (void);
//以下是External Laoder工程没有用到的函数
uint8_t CSP_QSPI_ReadID(uint32_t *uiID);
uint8_t CSP_QSPI_ReadBuffer(uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize);
uint8_t CSP_QSPI_WriteBuffer(uint8_t *_pBuf, uint32_t _uiWriteAddr, uint16_t _usWriteSize);
uint8_t CSP_QSPI_EraseOneSector(uint32_t EraseStartAddress);
/* USER CODE END Private defines */

/* USER CODE BEGIN Prototypes */


/* W25Qxx不同容量以幂运算 2^n Byte表示*/
#define QSPI_W25Q16_SIZE_2N 21		/* 2^21 = 2097152 Byte => 2 MB */
#define QSPI_W25Q32_SIZE_2N 22		/* 2^22 = 4194304 Byte => 4 MB */
#define QSPI_W25Q64_SIZE_2N 23		/* 2^23 = 8388608 Byte => 8 MB */
#define QSPI_W25Q128_SIZE_2N 24		/* 2^24 = 16777216 Byte => 16 MB */
#define QSPI_W25Q256_SIZE_2N 25		/* 2^25 = 33554432 Byte => 32 MB */

/*W25Qxx memory parameters*/
#define QSPI_FLASH_SIZE_2N    		QSPI_W25Q64_SIZE_2N    				/* Flash大小：幂运算 2^QSPI_W25Q64_SIZE = 8388608, 单位Byte*/
#define QSPI_FLASH_SIZE_MB			(1 << (QSPI_FLASH_SIZE_2N - 20))	/* Flash大小：以MB为单位 */
#define QSPI_FLASH_SIZE_Mb			(QSPI_FLASH_SIZE_MB * 8)			/* Flash大小：以Mb为单位 */
#define MEMORY_FLASH_SIZE			(QSPI_FLASH_SIZE_MB * 1024 * 1024)	/* Flash大小：以Byte为单位*/
#define MEMORY_BLOCK_SIZE			0x10000   /* BLOCK大小：	0x10000 => 65536 Bytes 	=> 64 KB*/
#define MEMORY_SECTOR_SIZE			0x1000    /* SECTOR大小：	0x1000 	=> 4096 Bytes 	=> 4KB 	每个BLOCK有16个SECTOR*/
#define MEMORY_PAGE_SIZE			0x100     /* PAGE大小：	0x100 	=> 256 Bytes 			每个SECTOR有16个PAGE*/

/*超时时间ms*/
//#define W25Q_TIMEOUT 10000	//ST示例代码用HAL_QPSI_TIMEOUT_DEFAULT_VALUE为5000

/*读取命令时空周期个数*/
#define DUMMY_CLOCK_CYCLES_READ_QUAD 6	//空周期个数，QUAD SPI读取数据时用到

/*W25Qxx 配置命令 */
#define WRITE_ENABLE_CMD 0x06
#define READ_STATUS_REG_CMD 0x05
#define READ_FLAG_STATUS_REG_CMD 0x05//0x70
#define WRITE_STATUS_REG_CMD 0x01
#define CHIP_ERASE_CMD 0xC7
#define QPI_ENABLE_CMD 0x35			//64JV不支持ADS位，但是支持这个命令
#define RESET_ENABLE_CMD 0x66
#define RESET_EXECUTE_CMD 0x99
#define READ_ID_CMD2          0x9F         /* 读取ID命令 */

/*W25Qxx 读写擦除命令 */
/* W25Q128JV及以下容量只支持24bit地址命令 */
/* 注意EBh/ECh命令地址阶段用4个io比6Bh/6CH命令地址阶段用1个io更快，因此用前者*/
#define SECTOR_ERASE_CMD 0x20					/* 24bit地址扇区擦除指令, 擦除粒度为4KB */
#define QUAD_IN_FAST_PROG_CMD 0x32				/* 24bit地址的4线快速写入命令 */
#define QUAD_READ_IO_CMD 0xEB					/* 24bit地址的4线快速读取命令(1-4-4) */
//#define QUAD_OUT_FAST_READ_CMD 0x6B				/* 24bit地址的4线快速读取命令(1-1-4) */
/* W25Q256JV及以上容量支持32bit地址命令 */
#define SECTOR_ERASE_4_BYTE_ADDR_CMD 0x21		/* 32bit地址扇区擦除指令, 擦除粒度为4KB */
#define QUAD_IN_FAST_4_BYTE_ADDR_PROG_CMD 0x34	/* 32bit地址的4线快速写入命令 */
#define QUAD_READ_IO_4_BYTE_ADDR_CMD 0xEC		/* 32bit地址的4线快速读取命令(1-4-4) */
//#define QUAD_OUT_FAST_READ_4_BYTE_ADDR_CMD 0x6C	/* 32bit地址的4线快速读取命令(1-1-4) */

/* 兼容不同容量 CMPT "compatible"*/
#if (QSPI_FLASH_SIZE_Mb <= 128)
/* W25Q16 W25Q32 W25Q64 W25Q128 */
#define CMPT_QSPI_ADDRESS_BITS			QSPI_ADDRESS_24_BITS			/* 24位地址 */
#define CMPT_CMD_SUBSECTOR_ERASE		SECTOR_ERASE_CMD				/* 24bit地址方式的扇区擦除命令，扇区大小4KB*/
#define CMPT_CMD_QUAD_IN_FAST_PROG		QUAD_IN_FAST_PROG_CMD			/* 24bit地址的4线快速写入命令 */
#define CMPT_CMD_QUAD_INOUT_FAST_READ	QUAD_READ_IO_CMD				/* 24bit地址的4线快速读取命令 */
#else
/* W25Q256 W25Q512 优先使用支持32bit地址的命令	*/
#define CMPT_QSPI_ADDRESS_BITS			QSPI_ADDRESS_32_BITS				/* 32位地址 */
#define CMPT_CMD_SUBSECTOR_ERASE		SECTOR_ERASE_4_BYTE_ADDR_CMD		/* 32bit地址方式的扇区擦除命令，扇区大小4KB*/
#define CMPT_CMD_QUAD_IN_FAST_PROG		QUAD_IN_FAST_4_BYTE_ADDR_PROG_CMD	/* 32bit地址的4线快速写入命令 */
#define CMPT_CMD_QUAD_INOUT_FAST_READ	QUAD_READ_IO_4_BYTE_ADDR_CMD		/* 32bit地址的4线快速读取命令 */
#endif

/* JEDEC Manufacturer ID */
#define MF_ID	(OxEF)

/* JEDEC Device ID: Memory type and Capacity */
#define MTC_W25Q80_BV 		(0x4014)	/* W25Q80BV*/
#define MTC_W25Q16_BV_CL_CV	(0x4015) 	/* W25Q16BV W25Q16CL W25Q16CV */
#define MTC_W25Q16_DW		(0x6015)	/* W25Q16DW*/
#define MTC_W25Q32_BV		(0X4016)	/* W25Q32BV*/
#define MTC_W25Q32_DW		(0x6016) 	/* W25Q32DW*/
#define MTC_W25Q64_BV_CV 	(0x4017)	/* W25Q648V W25Q64CV */
#define MTC_W25Q64_DW		(0x4017)	/* W25Q64DW */
#define MTC_W25Q128_BV		(0x4018)	/* W25Q128BV	*/
#define MTC_W25Q256_FV		(TBD)		/* W25Q256FV*/

/* USER CODE END Prototypes */
#ifdef __cplusplus
}
#endif

#endif	/* BSP_QSPI_W25QXX_H_ */

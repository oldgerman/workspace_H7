/**
  ******************************************************************************
  * @file        frtos_spi_psram.h
  * @author      OldGerman
  * @created on  Mar 12, 2023
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
  *
  * @note
  * ##5.2. 页面大小
  * 页面大小为 1K (CA[9:0])。默认突发设置是以连续方式跨越页面边界的线性突发模式。
  * 但请注意， 跨页边界的突发操作的最大输入时钟频率为 84 MHz。也可以通过 wrap
  * 边界切换命令设置 32 字 节 wrap (CA[4:0]) 模式，这种模式不允许跨越页面边界
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef FRTOS_SPI_FRTOS_SPI_PSRAM_H_
#define FRTOS_SPI_FRTOS_SPI_PSRAM_H_

#include "frtos_spi.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "string.h"

/* Exported types ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

#ifdef __cplusplus

using namespace ns_frtos_spi;

class FRTOS_SPIDev_PSRAM{
public:
	static void init(FRTOS_SPICmd *frtos_spi_cmd){
		ptr_frtos_spi_cmd = frtos_spi_cmd;
	}

	/* 设置线性突发或32byte Wrap模式 */
	static void setPageMode(uint32_t ){
		;//暂时用不上
		spi_transaction_t trans = {
				.instr = INSTR_READ_FAST,					/* 命令 */
				.instr_bytes = 1,							/* 命令字节数 */
				.cmd_transfer_mode = TRANSFER_MODE_POLL,	/* 指令轮询传输 */
		};
		ptr_frtos_spi_cmd->busTransferCmd(&trans);
	}
	/** @brief: 	DMA模式快速读
	  *	@param: 	addr 以字节寻址的地址位, 24bit地址
	  * @param: 	size 读出个数
	  */
	static void readFast_DMA(uint32_t addr, uint16_t size, uint8_t* buf)
	{
		spi_transaction_t trans = {
				.instr = INSTR_READ_FAST,					/* 命令 */
				.addr = addr,								/* 地址 */
				.tx_buffer = txDataBuffer,
				.rx_buffer = buf,							/* 数据 */
				.instr_bytes = 1,							/* 命令字节数 */
				.addr_bytes = 3,							/* 地址字节数 */
				.dummy_bytes = 1,							/* 等待字节数 */
				.data_bytes = size,							/* 数据字节数*/
				.cmd_transfer_mode = TRANSFER_MODE_POLL,	/* 指令轮询传输 */
				.data_transfer_mode = TRANSFER_MODE_DMA		/* 数据DMA传输 */
		};
		ptr_frtos_spi_cmd->busTransferExtCmdAndData(&trans);
	}
	/** @brief:	 	DMA模式写
	  * @param: 	addr 以字节寻址的地址位, 24bit地址
	  * @param: 	size 写入个数
	  */
	static void write_DMA(uint32_t addr, uint16_t size, uint8_t* buf)
	{
		spi_transaction_t trans = {
				.instr = INSTR_WRITE,						/* 命令 */
				.addr = addr,								/* 地址 */
				.tx_buffer = buf,							/* 数据 */
				.rx_buffer = rxDataBuffer,					/* 数据 */
				.instr_bytes = 1,							/* 命令字节数 */
				.addr_bytes = 3,							/* 地址字节数 */
				.data_bytes = size,							/* 数据字节数*/
				.cmd_transfer_mode = TRANSFER_MODE_POLL,	/* 指令轮询传输 */
				.data_transfer_mode = TRANSFER_MODE_DMA		/* 数据DMA传输 */
		};
		ptr_frtos_spi_cmd->busTransferExtCmdAndData(&trans);
	}
	/** @brief: 	轮询模式读
	  *	@param: 	addr 以字节寻址的地址位, 24bit地址
	  * @param: 	size 读出个数
	  */
	static void read_Polling(uint32_t addr, uint16_t size, uint8_t* buf)
	{
		spi_transaction_t trans = {
				.instr = INSTR_READ,						/* 命令 */
				.addr = addr,								/* 地址 */
				.rx_buffer = buf,							/* 数据 */
				.instr_bytes = 1,							/* 命令字节数 */
				.addr_bytes = 3,							/* 地址字节数 */
				.data_bytes = size,							/* 数据字节数*/
				.cmd_transfer_mode = TRANSFER_MODE_POLL,	/* 指令轮询传输 */
				.data_transfer_mode = TRANSFER_MODE_POLL	/* 数据DMA传输 */
		};
		ptr_frtos_spi_cmd->busTransferExtCmdAndData(&trans);
	}

	/** @brief:	 	轮询模式写
	  * @param: 	addr 以字节寻址的地址位, 24bit地址
	  * @param: 	size 写入个数
	  */
	static void write_Polling(uint32_t addr, uint16_t size, uint8_t* buf)
	{
		spi_transaction_t trans = {
				.instr = INSTR_WRITE,						/* 命令 */
				.addr = addr,								/* 地址 */
				.tx_buffer = buf,							/* 数据 */
				.instr_bytes = 1,							/* 命令字节数 */
				.addr_bytes = 3,							/* 地址字节数 */
				.data_bytes = size,							/* 数据字节数*/
				.cmd_transfer_mode = TRANSFER_MODE_POLL,	/* 指令轮询传输 */
				.data_transfer_mode = TRANSFER_MODE_POLL	/* 数据DMA传输 */
		};
		ptr_frtos_spi_cmd->busTransferExtCmdAndData(&trans);
	}

private:
	static const uint8_t INSTR_READ 		= 0x03U;	// 读指令 ‘h03（最高频率 33 MHz）
	static const uint8_t INSTR_READ_FAST 	= 0x0BU;	// 快速读指令 ‘h0B （最高频率 133 MHz）
	static const uint8_t INSTR_READ_ID 		= 0x9FU;	// 读 ID 指令 ‘h9F
	static const uint8_t INSTR_WRITE 		= 0x02U;	// 写指令 ‘h02

	static FRTOS_SPICmd 		*ptr_frtos_spi_cmd;
	static const uint16_t 		BLOCK_SIZE = 8192;		// block buffer size in bytes
	static uint8_t  			txDataBuffer[BLOCK_SIZE];
	static uint8_t  			rxDataBuffer[BLOCK_SIZE];
};


}
#endif

#endif /* FRTOS_SPI_FRTOS_SPI_PSRAM_H_ */

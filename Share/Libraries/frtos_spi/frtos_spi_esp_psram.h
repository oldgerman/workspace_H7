/*
 * frtos_spi_esp_psram.h
 *
 *  Created on: 2022年12月06日
 *      @Author: OldGerman (过气德国佬)
 */

#ifndef INC_FRTOS_SPI_ESP_PSRAM_H
#define INC_FRTOS_SPI_ESP_PSRAM_H
#include "frtos_spi.h"
#ifdef INC_FRTOS_SPI_H
#ifdef __cplusplus
extern "C" {
#endif

/* 私有包含 Private includes -------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* 导出类型 Exported types ---------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* 导出常量 Exported constants -----------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* 导出的宏 Exported macro ---------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* 导出函数原型 Exported functions prototypes --------------------------------*/
/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* 私有定义 Private defines --------------------------------------------------*/
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */
#ifdef __cplusplus

using namespace namespace_frtos_spi_t;

class FRTOS_SPIDev_ESP_PSRAM64H :public FRTOS_SPICmd{
public:
	FRTOS_SPIDev_ESP_PSRAM64H(FRTOS_SPIBase *pSPIBase,
			GPIO_TypeDef *SF_CS_GPIOx,
			uint16_t SF_CS_GPIO_Pin,
			uint32_t BaudRatePrescaler,
			uint32_t CLKPhase,
			uint32_t CLKPolarity)
	:FRTOS_SPICmd(pSPIBase, SF_CS_GPIOx, SF_CS_GPIO_Pin, BaudRatePrescaler, CLKPhase, CLKPolarity)
	{}
	FRTOS_SPIDev_ESP_PSRAM64H(FRTOS_SPIBase *pSPIBase,
			uint32_t BaudRatePrescaler,
			uint32_t CLKPhase,
			uint32_t CLKPolarity)
	:FRTOS_SPICmd(pSPIBase, BaudRatePrescaler, CLKPhase, CLKPolarity)
	{}

	/* 设置线性突发或32byte Wrap模式 */
	void SPISRAM_SETPAGE_Mode(uint32_t ){
		;//暂时用不上
	}
	/** @brief: 	DMA模式快速读
	  *	@param: 	addr 以字节寻址的地址位
	  * @param: 	size 读出个数
	  */
	uint16_t SPISRAM_FastRead_DMA(uint32_t addr, uint16_t size, uint8_t* buf)
	{
		spi_transaction_t trans = {
				.instr = INSTR_READ_FAST,					/* 命令 */
				.addr = addr,								/* 地址 */
				.rx_buffer = buf,							/* 数据 */
				.instr_bytes = 1,							/* 命令字节数 */
				.addr_bytes = 3,							/* 地址字节数 */
				.dummy_bytes = 1,							/* 等待字节数 */
				.data_bytes = size,							/* 数据字节数*/
				.cmd_transfer_mode = TRANSFER_MODE_POLL,	/* 指令轮询传输 */
				.data_transfer_mode = TRANSFER_MODE_DMA		/* 数据DMA传输 */
		};
		busSetCS(0);			/* 片选拉低 */
		busTransferCmd(&trans);
		busReceive(&trans);
		busSetCS(1); 			/* 片选拉高 */
		return 0;
	}
	/** @brief:	 	DMA模式写
	  * @param: 	addr 以字节寻址的地址位
	  * @param: 	size 写入个数
	  */
	uint16_t SPISRAM_Write_DMA(uint32_t addr, uint16_t size, uint8_t* buf)
	{
		spi_transaction_t trans = {
				.instr = INSTR_WRITE,						/* 命令 */
				.addr = addr,								/* 地址 */
				.tx_buffer = buf,							/* 数据 */
				.instr_bytes = 1,							/* 命令字节数 */
				.addr_bytes = 3,							/* 地址字节数 */
				.data_bytes = size,							/* 数据字节数*/
				.cmd_transfer_mode = TRANSFER_MODE_POLL,	/* 指令轮询传输 */
				.data_transfer_mode = TRANSFER_MODE_DMA		/* 数据DMA传输 */
		};
		busSetCS(0);			/* 片选拉低 */
		busTransferCmd(&trans);
		busTransmit(&trans); 	/* 片选拉高 */
		busSetCS(1);
		return 0;
	}
	/** @brief: 	轮询模式读
	  *	@param: 	addr 以字节寻址的地址位
	  * @param: 	size 读出个数
	  */
	uint16_t SPISRAM_Read_Polling(uint32_t addr, uint16_t size, uint8_t* buf)
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
		busSetCS(0);			/* 片选拉低 */
		busTransferCmd(&trans);
		busReceive(&trans);
		busSetCS(1);			/* 片选拉高 */
		return 0;
	}

	/** @brief:	 	轮询模式写
	  * @param: 	addr 以字节寻址的地址位
	  * @param: 	size 写入个数
	  */
	uint16_t SPISRAM_Write_Polling(uint32_t addr, uint16_t size, uint8_t* buf)
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
		busSetCS(0);			/* 片选拉低 */
		busTransferCmd(&trans);
		busTransmit(&trans); 	/* 片选拉高 */
		busSetCS(1);
		return 0;
	}
private:
	static const uint8_t INSTR_READ 		= 0x03U;	// 读指令 ‘h03（最高频率 33 MHz）
	static const uint8_t INSTR_READ_FAST 	= 0x0BU;	// 快速读指令 ‘h0B （最高频率 133 MHz）
	static const uint8_t INSTR_READ_ID 		= 0x9FU;	// 读 ID 指令 ‘h9F
	static const uint8_t INSTR_WRITE 		= 0x02U;	// 写指令 ‘h02
};

#endif	/* __cplusplus */
#endif	/* INC_FRTOS_SPI_H */
#endif /* INC_FRTOS_SPI_ESP_PSRAM_H */

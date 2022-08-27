/*
 * bsp_qspi_w25qxx.c
 *
 *  Created on: 2022年8月20日
 *      Author: OldGerman (过气德国佬)
 */
#include "bsp_config.h"
#ifdef EN_BSP_QSPI_W25QXX

#include "bsp_qspi_w25qxx.h"
#include "quadspi.h"
static uint8_t QSPI_WriteEnable(void);
static uint8_t QSPI_AutoPollingMemReady(void);
static uint8_t QSPI_Configuration(void);
static uint8_t QSPI_ResetChip(void);

/* QUADSPI init function */
uint8_t CSP_QUADSPI_Init(void) {
	//prepare QSPI peripheral for ST-Link Utility operations
	if (HAL_QSPI_DeInit(&hqspi) != HAL_OK) {
		return HAL_ERROR;
	}

	MX_QUADSPI_Init();

	if (QSPI_ResetChip() != HAL_OK) {
		return HAL_ERROR;
	}

	HAL_Delay(1);

	if (QSPI_AutoPollingMemReady() != HAL_OK) {
		return HAL_ERROR;
	}

	if (QSPI_WriteEnable() != HAL_OK) {

		return HAL_ERROR;
	}

	if (QSPI_Configuration() != HAL_OK) {
		return HAL_ERROR;
	}

	if (QSPI_AutoPollingMemReady() != HAL_OK) {
		return HAL_ERROR;
	}
	return HAL_OK;
}


uint8_t CSP_QSPI_Erase_Chip(void) {
	QSPI_CommandTypeDef sCommand = {0};


	if (QSPI_WriteEnable() != HAL_OK) {
		return HAL_ERROR;
	}


	/* Erasing Sequence --------------------------------- */
	sCommand.Instruction = CHIP_ERASE_CMD;
	sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	sCommand.AddressSize = CMPT_QSPI_ADDRESS_BITS;
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
	sCommand.AddressMode = QSPI_ADDRESS_NONE;
	sCommand.Address = 0;
	sCommand.DataMode = QSPI_DATA_NONE;
	sCommand.DummyCycles = 0;


	if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK) {
		return HAL_ERROR;
	}

	if (QSPI_AutoPollingMemReady() != HAL_OK) {
				return HAL_ERROR;
			}

	return HAL_OK;
}

uint8_t QSPI_AutoPollingMemReady(void) {
	QSPI_CommandTypeDef sCommand = {0};
	QSPI_AutoPollingTypeDef sConfig = {0};

	/* Configure automatic polling mode to wait for memory ready ------ */
	sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	sCommand.Instruction = READ_FLAG_STATUS_REG_CMD;
	sCommand.AddressMode = QSPI_ADDRESS_NONE;
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DataMode = QSPI_DATA_1_LINE;
	sCommand.DummyCycles = 0;
	sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	sConfig.Match = 0x00;
	sConfig.Mask = 0x01;
	sConfig.MatchMode = QSPI_MATCH_MODE_AND;
	sConfig.StatusBytesSize = 1;
	sConfig.Interval = 0x10;
	sConfig.AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE;

	if (HAL_QSPI_AutoPolling(&hqspi, &sCommand, &sConfig,
	HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
		return HAL_ERROR;
	}

	return HAL_OK;
}

static uint8_t QSPI_WriteEnable(void) {
	QSPI_CommandTypeDef sCommand = {0};
	QSPI_AutoPollingTypeDef sConfig = {0};

	QSPI_AutoPollingMemReady();

	/* Enable write operations ------------------------------------------ */
	sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	sCommand.Instruction = WRITE_ENABLE_CMD;
	sCommand.AddressMode = QSPI_ADDRESS_NONE;
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DataMode = QSPI_DATA_NONE;
	sCommand.DummyCycles = 0;
	sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK) {
		return HAL_ERROR;
	}

	/* Configure automatic polling mode to wait for write enabling ---- */
	sConfig.Match = 0x02;
	sConfig.Mask = 0x02;
	sConfig.MatchMode = QSPI_MATCH_MODE_AND;
	sConfig.StatusBytesSize = 1;
	sConfig.Interval = 0x10;
	sConfig.AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE;

	sCommand.Instruction = READ_STATUS_REG_CMD;
	sCommand.DataMode = QSPI_DATA_1_LINE;
	if (HAL_QSPI_AutoPolling(&hqspi, &sCommand, &sConfig,
	HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
		return HAL_ERROR;
	}

	return HAL_OK;
}

/**
  * @brief 擦除指定的地址范围的扇区，其中每个扇区大小4KB
  * @param EraseStartAddress	扇区开始地址，以4KB为单位的地址，比如0，4096, 8192等
  * @param EraseEndAddress		扇区结束地址，以4KB为单位的地址，比如0，4096, 8192等
  * @retval HAL Status
  */
uint8_t CSP_QSPI_EraseSector(uint32_t EraseStartAddress, uint32_t EraseEndAddress) {

	QSPI_CommandTypeDef sCommand = {0};

	EraseStartAddress = EraseStartAddress
			- EraseStartAddress % MEMORY_SECTOR_SIZE;

	/* Erasing Sequence -------------------------------------------------- */
	sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	sCommand.AddressSize = CMPT_QSPI_ADDRESS_BITS;
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
	sCommand.Instruction = CMPT_CMD_SUBSECTOR_ERASE;
	sCommand.AddressMode = QSPI_ADDRESS_1_LINE;

	sCommand.DataMode = QSPI_DATA_NONE;
	sCommand.DummyCycles = 0;

	while (EraseEndAddress >= EraseStartAddress) {
		sCommand.Address = (EraseStartAddress & 0x0FFFFFFF);

		if (QSPI_WriteEnable() != HAL_OK) {
			return HAL_ERROR;
		}

		if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
				!= HAL_OK) {
			return HAL_ERROR;
		}
		EraseStartAddress += MEMORY_SECTOR_SIZE;

		if (QSPI_AutoPollingMemReady() != HAL_OK) {
			return HAL_ERROR;
		}
	}

	return HAL_OK;
}

/**
  * @brief 擦除指定的地址的一个扇区，扇区大小4KB
  * @param EraseStartAddress	扇区开始地址，以4KB为单位的地址，比如0，4096, 8192等
  * @retval HAL Status
  */
uint8_t CSP_QSPI_EraseOneSector(uint32_t EraseStartAddress) {
#if 0
	/*这个方法擦除W25Q64JV耗时测试: 254571ms*/
	return CSP_QSPI_EraseSector(EraseStartAddress, EraseStartAddress + MEMORY_SECTOR_SIZE);
#else
	/*安富莱这个写法擦除W25Q64JV耗时测试: 123937ms，比上面的写法快约一倍*/
	QSPI_CommandTypeDef sCommand={0};

	/* 写使能 */
	if (QSPI_WriteEnable() != HAL_OK) {
		return HAL_ERROR;
	}

	/* 基本配置 */
	sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    /* 1线方式发送指令 */
	sCommand.AddressSize       = CMPT_QSPI_ADDRESS_BITS;     /* 地址位数 */
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* 无交替字节 */
	sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* W25Q256JV不支持DDR */
	sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* DDR模式，数据输出延迟 */
	sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;	 /* 每次传输都发指令 */

	/* 擦除配置 */
	sCommand.Instruction = CMPT_CMD_SUBSECTOR_ERASE;   /* 扇区擦除命令，扇区大小4KB*/
	sCommand.AddressMode = QSPI_ADDRESS_1_LINE;  /* 地址发送是1线方式 */
	sCommand.Address     = EraseStartAddress;        /* 扇区首地址，保证是4KB整数倍 */
	sCommand.DataMode    = QSPI_DATA_NONE;       /* 无需发送数据 */
	sCommand.DummyCycles = 0;                    /* 无需空周期 */

	if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return HAL_ERROR;
	}

	if (QSPI_AutoPollingMemReady() != HAL_OK) {
		return HAL_ERROR;
	}

	return HAL_OK;
#endif
}


uint8_t CSP_QSPI_WriteMemory(uint8_t* buffer, uint32_t address,uint32_t buffer_size) {

	QSPI_CommandTypeDef sCommand = {0};
	uint32_t end_addr, current_size, current_addr;

	/* Calculation of the size between the write address and the end of the page */
	current_addr = 0;


	//
	while (current_addr <= address) {
		current_addr += MEMORY_PAGE_SIZE;
	}
	current_size = current_addr - address;

	/* Check if the size of the data is less than the remaining place in the page */
	if (current_size > buffer_size) {
		current_size = buffer_size;
	}

	/* Initialize the adress variables */
	current_addr = address;
	end_addr = address + buffer_size;

	sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	sCommand.AddressSize = CMPT_QSPI_ADDRESS_BITS;
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
	sCommand.Instruction = CMPT_CMD_QUAD_IN_FAST_PROG;
	sCommand.AddressMode = QSPI_ADDRESS_1_LINE;
	sCommand.DataMode = QSPI_DATA_4_LINES;
	sCommand.NbData = buffer_size;
	sCommand.Address = address;
	sCommand.DummyCycles = 0;

	/* Perform the write page by page */
	do {
		sCommand.Address = current_addr;
		sCommand.NbData = current_size;

		if (current_size == 0) {
			return HAL_OK;
		}

		/* Enable write operations */
		if (QSPI_WriteEnable() != HAL_OK) {
			return HAL_ERROR;
		}

		/* Configure the command */
		if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
				!= HAL_OK) {

			return HAL_ERROR;
		}

		/* Transmission of the data */
		if (HAL_QSPI_Transmit(&hqspi, buffer, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {

			return HAL_ERROR;
		}

		/* Configure automatic polling mode to wait for end of program */
		if (QSPI_AutoPollingMemReady() != HAL_OK) {
			return HAL_ERROR;
		}

		/* Update the address and size variables for next page programming */
		current_addr += current_size;
		buffer += current_size;
		current_size =
				((current_addr + MEMORY_PAGE_SIZE) > end_addr) ?
						(end_addr - current_addr) : MEMORY_PAGE_SIZE;
	} while (current_addr <= end_addr);

	return HAL_OK;

}


uint8_t CSP_QSPI_EnableMemoryMappedMode(void) {
	QSPI_CommandTypeDef sCommand = {0};
	QSPI_MemoryMappedTypeDef sMemMappedCfg = {0};

	/* Enable Memory-Mapped mode-------------------------------------------------- */
	/* 基本配置 */
	sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;		/* 1线方式发送指令 */
	sCommand.AddressSize = CMPT_QSPI_ADDRESS_BITS;	 		/* 地址位数 */
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;	/* 无交替字节 */
	sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;				/* W25Q256JV不支持DDR */
	sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;	/* DDR模式，数据输出延迟 */
	sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;			/* 每次传输都发指令 */
	sCommand.NbData = 0;
	sCommand.Address = 0;
	/* 全部采用4线 */
	sCommand.Instruction = CMPT_CMD_QUAD_INOUT_FAST_READ;	/* 快速读取命令 */
	sCommand.AddressMode = QSPI_ADDRESS_4_LINES;			/* 4个地址线 */
	sCommand.DataMode 	 = QSPI_DATA_4_LINES;				/* 4个数据线 */
	sCommand.DummyCycles = DUMMY_CLOCK_CYCLES_READ_QUAD;	/* 空周期 */

	/* 关闭溢出计数 */
	sMemMappedCfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
	sMemMappedCfg.TimeOutPeriod = 0;

	if (HAL_QSPI_MemoryMapped(&hqspi, &sCommand, &sMemMappedCfg) != HAL_OK) {
		return HAL_ERROR;
	}
	return HAL_OK;
}

/*Send reset in 1,2 and 4 lines*/
uint8_t QSPI_ResetChip() {
	QSPI_CommandTypeDef sCommand = {0};
	uint32_t temp = 0;
	/* Erasing Sequence -------------------------------------------------- */

	/* Enable Reset */
	sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;		/* 1线方式发送指令 */
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;	/* 无交替字节 */
	sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;			/* 每次传输都发指令 */
	sCommand.Instruction = RESET_ENABLE_CMD;				/* 66h */
	sCommand.AddressMode = QSPI_ADDRESS_NONE;				/* 无地址 */
	sCommand.Address = 0;
	sCommand.DataMode = QSPI_DATA_NONE;
	sCommand.DummyCycles = 0;

	if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK) {
		return HAL_ERROR;
	}
	for (temp = 0; temp < 0x2f; temp++) {
		__NOP();
	}

	/* Reset Device */
	sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;		/* 1线方式发送指令 */
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;	/* 无交替字节 */
	sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;			/* 每次传输都发指令 */
	sCommand.Instruction = RESET_EXECUTE_CMD;				/* 99h */
	sCommand.AddressMode = QSPI_ADDRESS_NONE;				/* 无地址 */
	sCommand.Address = 0;
	sCommand.DataMode = QSPI_DATA_NONE;
	sCommand.DummyCycles = 0;

	if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK) {
		return HAL_ERROR;
	}

	return HAL_OK;
}

/*Enable quad mode and set dummy cycles count*/
uint8_t QSPI_Configuration(void) {
#if 1 /*W25QxxJV不需要配置，出厂默认配置就是QUAD SPI模式*/
	return HAL_OK;
#else /*ST示例代码读取了Status和Config寄存器，设置了QUAD SPI模式和空周期，再写入Status和Config寄存器，这里没用到，仅作参考*/
	QSPI_CommandTypeDef sCommand = {0};
	uint8_t test_buffer[4] = { 0 };
	/*read status register*/
	sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	sCommand.Instruction = READ_STATUS_REG_CMD;
	sCommand.AddressMode = QSPI_ADDRESS_NONE;
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DataMode = QSPI_DATA_1_LINE;
	sCommand.DummyCycles = 0;
	sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
	sCommand.NbData = 1;

	if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK) {
		return HAL_ERROR;
	}
	if (HAL_QSPI_Receive(&hqspi, test_buffer,
	HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
		return HAL_ERROR;
	}
	/*read configuration register*/
	sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	sCommand.Instruction = READ_CONFIGURATION_REG_CMD;
	sCommand.AddressMode = QSPI_ADDRESS_NONE;
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DataMode = QSPI_DATA_1_LINE;
	sCommand.DummyCycles = 0;
	sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
	sCommand.NbData = 1;

	if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK) {
		return HAL_ERROR;
	}
	if (HAL_QSPI_Receive(&hqspi, &(test_buffer[1]),
	HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
		return HAL_ERROR;
	}
	/*modify buffer to enable quad mode*/
	test_buffer[0] |= 0x40;

	/*set dummy cycles*/
	test_buffer[1] = 0x40;

	if (QSPI_WriteEnable() != HAL_OK) {
		return HAL_ERROR;
	}

	sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	sCommand.AddressSize = CMPT_QSPI_ADDRESS_BITS;
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
	sCommand.Instruction = WRITE_STATUS_REG_CMD;
	sCommand.AddressMode = QSPI_ADDRESS_NONE;
	sCommand.DataMode = QSPI_DATA_1_LINE;
	sCommand.DummyCycles = 0;
	sCommand.NbData = 1;

	if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK) {
		return HAL_ERROR;
	}

	if (HAL_QSPI_Transmit(&hqspi, test_buffer,
	HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
		Error_Handler();
		return HAL_ERROR;
	}

	if (QSPI_WriteEnable() != HAL_OK) {
		return HAL_ERROR;
	}

	sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	sCommand.AddressSize = CMPT_QSPI_ADDRESS_BITS;
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
	sCommand.Instruction = WRITE_CONFIGURATION_REG_CMD;
	sCommand.AddressMode = QSPI_ADDRESS_NONE;
	sCommand.DataMode = QSPI_DATA_1_LINE;
	sCommand.DummyCycles = 0;
	sCommand.NbData = 1;

	if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK) {
		return HAL_ERROR;
	}

	if (HAL_QSPI_Transmit(&hqspi, &(test_buffer[1]),
	HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
		Error_Handler();
		return HAL_ERROR;
	}
	return HAL_OK;
#endif
}

/**
  * @brief Read Flash Chip JEDEC ID
  * @param uiID : 数据源缓冲区。
  * 			  32bit ID MF_ID + Device ID,
  * 			  For example: 00EF4017, MF_ID (OxEF), MTC_W25Q64_BV_CV (0x4017)
  * @retval HAL Status
  */
uint8_t CSP_QSPI_ReadID(uint32_t *uiID)
{
	QSPI_CommandTypeDef s_command = {0};
	uint8_t buf[3];

	/* 基本配置 */
	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    /* 1线方式发送指令 */
	s_command.AddressSize       = CMPT_QSPI_ADDRESS_BITS;     /* 地址位数 */
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* 无交替字节 */
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* W25Q256JV不支持DDR */
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* DDR模式，数据输出延迟 */
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;	  /* 每次传输都发指令 */

	/* 读取JEDEC ID */
	s_command.Instruction = READ_ID_CMD2;         /* 读取ID命令 */
	s_command.AddressMode = QSPI_ADDRESS_NONE;    /* 1线地址 */
	s_command.DataMode = QSPI_DATA_1_LINE;        /* 1线地址 */
	s_command.DummyCycles = 0;                    /* 无空周期 */
	s_command.NbData = 3;                         /* 读取三个数据 */

	if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return HAL_ERROR;
	}

	if (HAL_QSPI_Receive(&hqspi, buf, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return HAL_ERROR;
	}

	*uiID = (buf[0] << 16) | (buf[1] << 8 ) | buf[2];

	return HAL_OK;
}

/**
  * @brief 连续读取若干字节，字节个数不能超出芯片容量
  * @param _pBuf : 数据源缓冲区。
  * @param _uiReadAddr ：起始地址。
  * @param _usSize ：数据个数, 可以大于PAGE_SIZE, 但是不能超出芯片总容量。
  * @retval HAL Status
  */
uint8_t CSP_QSPI_ReadBuffer(uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize)
{

	QSPI_CommandTypeDef sCommand = {0};


	/* 基本配置 */
	sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    	/* 1线方式发送指令 */
	sCommand.AddressSize       = CMPT_QSPI_ADDRESS_BITS;     	/* 地址位数 */
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  	/* 无交替字节 */
	sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      	/* W25Q256JV不支持DDR */
	sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  	/* DDR模式，数据输出延迟 */
	sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;		/* 每次传输要发指令 */

	/* 读取数据 */
	sCommand.Instruction = CMPT_CMD_QUAD_INOUT_FAST_READ;	/* 4线快速读取命令 */
	sCommand.DummyCycles = 6;                    /* 空周期 */
	sCommand.AddressMode = QSPI_ADDRESS_4_LINES; /* 4线地址 */
	sCommand.DataMode    = QSPI_DATA_4_LINES;    /* 4线数据 */
	sCommand.NbData      = _uiSize;              /* 读取的数据大小 */
	sCommand.Address     = _uiReadAddr;          /* 读取数据的起始地址 */

	if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return HAL_ERROR;
	}

	/* 读取 */
	if (HAL_QSPI_Receive(&hqspi, _pBuf, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return HAL_ERROR;
	}

	return HAL_OK;
}

/**
  * @brief 页编程，页大小256字节，任意页都可以写入
  * @param _pBuf : 数据源缓冲区；
  * @param _uiWriteAddr ：目标区域首地址，即页首地址，比如0， 256, 512等。
  * @param _usWriteSize ：数据个数，不能超过页面大小，范围1 - 256。
  * @retval HAL Status
  */
uint8_t CSP_QSPI_WriteBuffer(uint8_t *_pBuf, uint32_t _uiWriteAddr, uint16_t _usWriteSize)
{
	QSPI_CommandTypeDef sCommand={0};

	/* 写使能 */
	if (QSPI_WriteEnable() != HAL_OK) {
		return HAL_ERROR;
	}

	/* 基本配置 */
	sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    /* 1线方式发送指令 */
	sCommand.AddressSize       = CMPT_QSPI_ADDRESS_BITS;     /* 地址位数 */
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* 无交替字节 */
	sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* W25Q256JV不支持DDR */
	sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* DDR模式，数据输出延迟 */
	sCommand.SIOOMode          = QSPI_SIOO_INST_ONLY_FIRST_CMD;	 /* 仅发送一次命令 */

	/* 写序列配置 */
	sCommand.Instruction = CMPT_CMD_QUAD_IN_FAST_PROG;/* 4线快速写入命令 */
	sCommand.DummyCycles = 0;                    /* 不需要空周期 */
	sCommand.AddressMode = QSPI_ADDRESS_1_LINE;  /* 1线地址方式 */
	sCommand.DataMode    = QSPI_DATA_4_LINES;    /* 4线数据方式 */
	sCommand.NbData      = _usWriteSize;         /* 写数据大小 */
	sCommand.Address     = _uiWriteAddr;         /* 写入地址 */

	if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return HAL_ERROR;
	}

	/* 启动传输 */
	if (HAL_QSPI_Transmit(&hqspi, _pBuf, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return HAL_ERROR;
	}

	if (QSPI_AutoPollingMemReady() != HAL_OK) {
		return HAL_ERROR;
	}

	return HAL_OK;
}
#endif /* EN_BSP_QSPI_W25QXX */

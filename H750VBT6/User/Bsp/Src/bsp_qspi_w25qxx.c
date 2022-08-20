#include "bsp.h"
#include "quadspi.h"


static void QSPI_WriteEnable(QSPI_HandleTypeDef *qspiHandle);
static void QSPI_AutoPollingMemReady(QSPI_HandleTypeDef *qspiHandle);

void bsp_InitQSPI_W25Q256(void){
#if (QSPI_FLASH_SIZE_MBit > 128)
	/*读取register-3的ADS位看使用的地址模式*/;
	/*输入4字节模式（B7h）”或“退出4字节模式”（E9h）指令*/;
#endif
}

/*
 *********************************************************************************************************
 *	函 数 名: QSPI_EraseSector
 *	功能说明: 擦除指定的扇区，扇区大小4KB
 *	形    参: _uiSectorAddr : 扇区地址，以4KB为单位的地址，比如0，4096, 8192等
 *	返 回 值: 无
 *********************************************************************************************************
 */
void QSPI_EraseSector(uint32_t _uiSectorAddr)
{
	QSPI_CommandTypeDef sCommand={0};

	/* 写使能 */
	QSPI_WriteEnable(&hqspi);

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
	sCommand.Address     = _uiSectorAddr;        /* 扇区首地址，保证是4KB整数倍 */    
	sCommand.DataMode    = QSPI_DATA_NONE;       /* 无需发送数据 */  
	sCommand.DummyCycles = 0;                    /* 无需空周期 */  

	if (HAL_QSPI_Command(&hqspi, &sCommand, W25Q_TIMEOUT) != HAL_OK)
	{
		Error_Handler();
	}

	QSPI_AutoPollingMemReady(&hqspi);
}

/*
 *********************************************************************************************************
 *	函 数 名: QSPI_EraseChip
 *	功能说明: 整个芯片擦除
 *	形    参: 无
 *	返 回 值: 无
 *********************************************************************************************************
 */
void QSPI_EraseChip(void)
{
	QSPI_CommandTypeDef sCommand={0};

	/* 写使能 */
	QSPI_WriteEnable(&hqspi);

	/* 基本配置 */
	sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    /* 1线方式发送指令 */
	sCommand.AddressSize       = CMPT_QSPI_ADDRESS_BITS;     /* 地址位数 */
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* 无交替字节 */
	sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* W25Q256JV不支持DDR */
	sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* DDR模式，数据输出延迟 */
	sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;	 /* 每次传输都发指令 */	

	/* 擦除配置 */
	sCommand.Instruction = BULK_ERASE_CMD;       /* 整个芯片擦除命令*/       
	sCommand.AddressMode = QSPI_ADDRESS_1_LINE;  /* 地址发送是1线方式 */       
	sCommand.Address     = 0;                    /* 地址 */    
	sCommand.DataMode    = QSPI_DATA_NONE;       /* 无需发送数据 */  
	sCommand.DummyCycles = 0;                    /* 无需空周期 */  

	if (HAL_QSPI_Command(&hqspi, &sCommand, W25Q_TIMEOUT) != HAL_OK)
	{
		Error_Handler();
	}

	QSPI_AutoPollingMemReady(&hqspi);
}

/*
 *********************************************************************************************************
 *	函 数 名: QSPI_WriteBuffer
 *	功能说明: 页编程，页大小256字节，任意页都可以写入
 *	形    参: _pBuf : 数据源缓冲区；
 *			  _uiWriteAddr ：目标区域首地址，即页首地址，比如0， 256, 512等。
 *			  _usWriteSize ：数据个数，不能超过页面大小，范围1 - 256。
 *	返 回 值: 1:成功， 0：失败
 *********************************************************************************************************
 */
uint8_t QSPI_WriteBuffer(uint8_t *_pBuf, uint32_t _uiWriteAddr, uint16_t _usWriteSize)
{
	QSPI_CommandTypeDef sCommand={0};

	/* 写使能 */
	QSPI_WriteEnable(&hqspi);

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

	if (HAL_QSPI_Command(&hqspi, &sCommand, W25Q_TIMEOUT) != HAL_OK)
	{
		//return 0;
		Error_Handler();
	}

	/* 启动传输 */
	if (HAL_QSPI_Transmit(&hqspi, _pBuf, W25Q_TIMEOUT) != HAL_OK)
	{
		//return 0;
		Error_Handler();

	}

	QSPI_AutoPollingMemReady(&hqspi);

	return 1;
}

/*
 *********************************************************************************************************
 *	函 数 名: QSPI_ReadBuffer
 *	功能说明: 连续读取若干字节，字节个数不能超出芯片容量。
 *	形    参: _pBuf : 数据源缓冲区。
 *			  _uiReadAddr ：起始地址。
 *			  _usSize ：数据个数, 可以大于PAGE_SIZE, 但是不能超出芯片总容量。
 *	返 回 值: 无
 *********************************************************************************************************
 */
void QSPI_ReadBuffer(uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize)
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

	if (HAL_QSPI_Command(&hqspi, &sCommand, W25Q_TIMEOUT) != HAL_OK)
	{
		Error_Handler();
	}

	/* 读取 */
	if (HAL_QSPI_Receive(&hqspi, _pBuf, W25Q_TIMEOUT) != HAL_OK)
	{
		Error_Handler();
	}	
}

/*
 *********************************************************************************************************
 *	函 数 名: QSPI_WriteEnable
 *	功能说明: 写使能
 *	形    参: qspiHandle  QSPI_HandleTypeDef句柄。
 *	返 回 值: 无
 *********************************************************************************************************
 */
static void QSPI_WriteEnable(QSPI_HandleTypeDef *qspiHandle)
{
	QSPI_CommandTypeDef     sCommand = {0};

	/* 基本配置 */
	sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    /* 1线方式发送指令 */
	sCommand.AddressSize       = CMPT_QSPI_ADDRESS_BITS;     /* 地址位数 */
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* 无交替字节 */
	sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* W25Q256JV不支持DDR */
	sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* DDR模式，数据输出延迟 */
	sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;	 /* 每次传输都发指令 */

	/* 写使能 */
	sCommand.Instruction       = WRITE_ENABLE_CMD;  /* 写使能指令 */
	sCommand.AddressMode       = QSPI_ADDRESS_NONE; /* 无需地址 */
	sCommand.DataMode          = QSPI_DATA_NONE;    /* 无需数据 */
	sCommand.DummyCycles       = 0;                 /* 空周期  */

	if (HAL_QSPI_Command(&hqspi, &sCommand, W25Q_TIMEOUT) != HAL_OK)
	{
		Error_Handler();
	}	
}

/*
 *********************************************************************************************************
 *	函 数 名: QSPI_AutoPollingMemReady
 *	功能说明: 等待QSPI Flash就绪，主要用于Flash擦除和页编程时使用
 *	形    参: qspiHandle  QSPI_HandleTypeDef句柄
 *	返 回 值: 无
 *********************************************************************************************************
 */
static void QSPI_AutoPollingMemReady(QSPI_HandleTypeDef *qspiHandle)
{
	QSPI_CommandTypeDef     sCommand = {0};
	QSPI_AutoPollingTypeDef sConfig = {0};


	/* 基本配置 */
	sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    /* 1线方式发送指令 */
	sCommand.AddressSize       = CMPT_QSPI_ADDRESS_BITS;     /* 地址位数 */
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* 无交替字节 */
	sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* W25Q256JV不支持DDR */
	sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* DDR模式，数据输出延迟 */
	sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;	 /* 每次传输都发指令 */

	/* 读取状态*/
	sCommand.Instruction       = READ_STATUS_REG_CMD; /* 读取状态命令 */
	sCommand.AddressMode       = QSPI_ADDRESS_NONE;   /* 无需地址 */
	sCommand.DataMode          = QSPI_DATA_1_LINE;    /* 1线数据 */
	sCommand.DummyCycles       = 0;                   /* 无需空周期 */

	/* 屏蔽位设置的bit0，匹配位等待bit0为0，即不断查询状态寄存器bit0，等待其为0 */
	sConfig.Mask            = 0x01;
	sConfig.Match           = 0x00;
	sConfig.MatchMode       = QSPI_MATCH_MODE_AND;
	sConfig.StatusBytesSize = 1;
	sConfig.Interval        = 0x10;
	sConfig.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

	if (HAL_QSPI_AutoPolling(&hqspi, &sCommand, &sConfig, W25Q_TIMEOUT) != HAL_OK)
	{
		Error_Handler();
	}
}

/*
 *********************************************************************************************************
 *    函 数 名: QSPI_MemoryMapped
 *    功能说明: QSPI内存映射，地址 0x90000000
 *    形    参: 无
 *    返 回 值: 无
 *********************************************************************************************************
 */
void QSPI_MemoryMapped(void)
{
	QSPI_CommandTypeDef s_command = {0};
	QSPI_MemoryMappedTypeDef s_mem_mapped_cfg = {0};

	/* 基本配置 */
	s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;      /* 1线方式发送指令 */
	s_command.AddressSize       = CMPT_QSPI_ADDRESS_BITS;     /* 地址位数 */
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* 无交替字节 */
	s_command.DdrMode = QSPI_DDR_MODE_DISABLE;                /* W25Q256JV不支持DDR */
	s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;   /* DDR模式，数据输出延迟 */
	s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;            /* 每次传输都发指令 */

	/* 全部采用4线 */
	s_command.Instruction = CMPT_CMD_QUAD_INOUT_FAST_READ;			/* 快速读取命令 */
	s_command.AddressMode = QSPI_ADDRESS_4_LINES;                 /* 4个地址线 */
	s_command.DataMode = QSPI_DATA_4_LINES;                       /* 4个数据线 */
	s_command.DummyCycles = 6;                                    /* 空周期 */

	/* 关闭溢出计数 */
	s_mem_mapped_cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
	s_mem_mapped_cfg.TimeOutPeriod = 0;

	if (HAL_QSPI_MemoryMapped(&hqspi, &s_command, &s_mem_mapped_cfg) != HAL_OK)
	{
		Error_Handler();
	}
}

/*
 *********************************************************************************************************
 *	函 数 名: sf_ReadID
 *	功能说明: 读取器件ID
 *	形    参: 无
 *	返 回 值: 32bit的器件ID (最高8bit填0，有效ID位数为24bit）
 *********************************************************************************************************
 */
uint32_t QSPI_ReadID(void)
{
	uint32_t uiID;
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

	if (HAL_QSPI_Command(&hqspi, &s_command, W25Q_TIMEOUT) != HAL_OK)
	{
		//       Error_Handler();
	}

	if (HAL_QSPI_Receive(&hqspi, buf, W25Q_TIMEOUT) != HAL_OK)
	{
		Error_Handler();
	}

	uiID = (buf[0] << 16) | (buf[1] << 8 ) | buf[2];

	return uiID;
}

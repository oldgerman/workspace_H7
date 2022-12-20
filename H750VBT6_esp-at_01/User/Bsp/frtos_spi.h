/*
 * frtos_spi.h
 *
 *  Created on: 2022年12月06日
 *      @Author: OldGerman (过气德国佬)
 *		@Modified: 使用C++封装一次安富莱的bsp_spi_bus，可以创建多路SPI总线的实例，
 *					构造时指向多个设备实例，支持同一路总线的设备互斥
 *		@Note：仅支持全双工？
 */

#ifndef INC_FRTOS_SPI_H
#define INC_FRTOS_SPI_H
#include "stm32h7xx_hal_conf.h"
#ifdef HAL_SPI_MODULE_ENABLED
#ifdef __cplusplus
#include <list>
#include <vector>
#include <algorithm>
extern "C" {
#endif

/* 私有包含 Private includes -------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "spi.h"
/* USER CODE END Includes */

/* 导出类型 Exported types ---------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* 导出常量 Exported constants -----------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* 导出的宏 Exported macro ---------------------------------------------------*/
/* USER CODE BEGIN EM */
#define RTOS_EN 1	//	0不使用RTOS，修改为1使用RTOS
#if RTOS_EN
#include "cmsis_os.h"
#include "semphr.h"
#else
#define osDelay
#endif

/* 重定义下SPI SCK时钟，方便移植 */
#define SPI_BAUDRATEPRESCALER_100M      SPI_BAUDRATEPRESCALER_2			/* 100M */
#define SPI_BAUDRATEPRESCALER_50M       SPI_BAUDRATEPRESCALER_4			/* 50M */
#define SPI_BAUDRATEPRESCALER_12_5M     SPI_BAUDRATEPRESCALER_8			/* 12.5M */
#define SPI_BAUDRATEPRESCALER_6_25M     SPI_BAUDRATEPRESCALER_16		/* 6.25M */
#define SPI_BAUDRATEPRESCALER_3_125M    SPI_BAUDRATEPRESCALER_32		/* 3.125M */
#define SPI_BAUDRATEPRESCALER_1_5625M   SPI_BAUDRATEPRESCALER_64		/* 1.5625M */
#define SPI_BAUDRATEPRESCALER_781_25K   SPI_BAUDRATEPRESCALER_128		/* 781.25K */
#define SPI_BAUDRATEPRESCALER_390_625K  SPI_BAUDRATEPRESCALER_256		/* 390.625K */

#define	SPI_BUFFER_SIZE		(4 * 1024)				/*  */

/* USER CODE END EM */

/* 导出函数原型 Exported functions prototypes --------------------------------*/

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* 私有定义 Private defines --------------------------------------------------*/
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */
#ifdef __cplusplus

#ifndef FRTOS_SPI_ENUMS
#define FRTOS_SPI_ENUMS
namespace namespace_frtos_spi_t
{
/* transfer mode */
typedef enum {
	TRANSFER_MODE_NONE = 0,	/* 不传输 */
	TRANSFER_MODE_DMA,    	/* DMA方式  */
	TRANSFER_MODE_INT,    	/* 中断方式 */
	TRANSFER_MODE_POLL,   	/* 查询方式 */
}transfer_mode_t;
/* transfer state */
typedef enum {
	TRANSFER_STATE_WAIT,
	TRANSFER_STATE_COMPLETE,
	TRANSFER_STATE_ERROR
}transfer_state_t;

/**
 * This structure describes one SPI transaction. The descriptor should not be modified until the transaction finishes.
 */
typedef  struct {
    uint16_t instr;
    union {								///< Address data, of which the length is set in the ``address_bits`` of spi_device_interface_config_t.
    	uint32_t addr;					///<  <b>NOTE: this field, used to be "address" in ESP-IDF 2.1 and before, is re-written to be used in a new way in ESP-IDF3.0.</b>
    	uint8_t addr_data[4];			///< Example: write 0x123400 and address_bits=24 to send address of 0x12, 0x34, 0x00 (in previous version, you may have to write 0x12340000).
    };
    union {
        uint8_t *tx_buffer;     	 	///< Pointer to transmit buffer, or NULL for no MOSI phase
        uint8_t tx_data[4];         	///< If SPI_TRANS_USE_TXDATA is set, data set here is sent directly from this variable.
    };
    uint16_t tx_size;                	///< Total data size transmited, in bytes
    union {
    	uint8_t *rx_buffer;            	///< Pointer to receive buffer, or NULL for no MISO phase. Written by 4 bytes-unit if DMA is used.（这个和H7 DMA SPI需要四字节对齐一样）
        uint8_t rx_data[4];         	///< If SPI_TRANS_USE_RXDATA is set, data is received directly to this variable
    };
    uint16_t rx_size;                	///< SPI如果工作在全双工，RX数据大小不能比TX大，Total data length received, should be not greater than ``length`` in full-duplex mode (0 defaults this to the value of ``length``).

    uint8_t instr_bytes;           		///< The command length in this transaction, in bytes.
    uint8_t addr_bytes;           		///< The address length in this transaction, in bytes.
    uint8_t dummy_bytes;             	///< The dummy length in this transaction, in bytes.

    transfer_mode_t cmd_transfer_mode;	/// 发送命令序列使用的模式
    transfer_mode_t data_transfer_mode;	/// 数据阶段使用的模式

    uint32_t timeout;					/// 超时时间
    uint8_t*user;                     	///< User-defined variable. Can be used to store eg transaction ID.
}spi_transaction_t ;        			//the rx data should start from a 32-bit aligned address to get around dma issue.

}
#endif

using namespace namespace_frtos_spi_t;

/* 仅支持标准SPI */
class FRTOS_SPIBase{
public:
	FRTOS_SPIBase(SPI_HandleTypeDef &hspi, uint8_t*pRxData,uint8_t*pTxData, uint16_t sizeBuf)
	:_hspi(hspi), _pRxData(pRxData), _pTxData(pTxData), _size(sizeBuf){
#if RTOS_EN
		_SPISemphr = nullptr;
#else
		_SPISemphr = 0;
#endif
		wTransferState = 0;
	}


	void baseInitBus();
	void baseSetParam(uint32_t _BaudRatePrescaler, uint32_t _CLKPhase, uint32_t _CLKPolarity);
	void baseTransfer(transfer_mode_t spiTransMode);
	void baseTransmitExt(transfer_mode_t spiTransMode, uint8_t* pTxData, uint16_t size);
	void baseReceiveExt(transfer_mode_t spiTransMode, uint8_t* pRxData, uint16_t size);
	void baseTransferExt(transfer_mode_t spiTransMode, uint8_t* pTxData, uint8_t* pRxData, uint16_t size);
	void baseEnter();
	void baseExit();
	uint8_t baseBusy();

	void baseInitSemphr();
	static void CpltCallback(SPI_HandleTypeDef *);
#if RTOS_EN
	SemaphoreHandle_t * getSemphr() { return &_SPISemphr; }
#endif

private:
	SPI_HandleTypeDef  & _hspi;
public:
	uint8_t *_pRxData;
	uint8_t *_pTxData;
	uint32_t g_spiLen;	//buf缓冲区迭代下标，DMA、BDMA都仅支持1~65535个，但为了检测超出65535，使用uin32_t类型
	volatile uint32_t wTransferState;
private:
	/*
	 * 在构造时将提前定义的全局buffer的地址和大小作为参数传入，而不使用malloc申请，
	 * 因此可以将buffer使用__attribute__编译到任意RAM区
	 */

	const uint16_t _size;

	/* 备份SPI几个关键传输参数，波特率，相位，极性. 如果不同外设切换，需要重新Init SPI参数 */
	uint32_t s_BaudRatePrescaler;
	uint32_t s_CLKPhase;
	uint32_t s_CLKPolarity;

	uint8_t g_spi_busy;

#if RTOS_EN
	SemaphoreHandle_t 	_SPISemphr;		//SPIx信号量，即多个任务用比如FRToSSPI1，占用SPI1总线，那么_SPISemphr保证每次SPI1资源只被一个任务使用
	StaticSemaphore_t 	_xSemphrBuffer;  //用来保存信号量结构体，为啥不用指针？
#else
	bool 					_SPISemphr;		//不跑RTOS，_SPISemphr没有实际用处
#endif

};


/* SPI 命令序列类 */
class FRTOS_SPICmd{
public:
	/* 使用软件CS */
	FRTOS_SPICmd(FRTOS_SPIBase *pSPIBase,
			GPIO_TypeDef *SF_CS_GPIOx,
			uint16_t SF_CS_GPIO_Pin,
			uint32_t BaudRatePrescaler,
			uint32_t CLKPhase,
			uint32_t CLKPolarity);

	/* 使用硬件NSS */
	FRTOS_SPICmd(FRTOS_SPIBase *pSPIBase,
			uint32_t BaudRatePrescaler,
			uint32_t CLKPhase,
			uint32_t CLKPolarity);

	~FRTOS_SPICmd(){}

	void busReceive(spi_transaction_t * pTransaction_t){
		_pSPIBase->baseReceiveExt(pTransaction_t->data_transfer_mode,
				pTransaction_t->rx_buffer,
				pTransaction_t->rx_size);
	}
	void  busTransmit(spi_transaction_t * pTransaction_t){
		_pSPIBase->baseTransmitExt(pTransaction_t->data_transfer_mode,
				pTransaction_t->tx_buffer,
				pTransaction_t->tx_size);
	}

	void  busTransfer(spi_transaction_t * pTransaction_t){
		_pSPIBase->baseTransferExt(pTransaction_t->data_transfer_mode,
				pTransaction_t->tx_buffer,
				pTransaction_t->rx_buffer,
				pTransaction_t->tx_size);
	}
	void busTransferCmd(spi_transaction_t * pTransaction_t){
			/* 复位缓冲区游标 */
			_pSPIBase->g_spiLen = 0;
			/* 命令阶段*/
			for(int8_t i = pTransaction_t->instr_bytes - 1; i >= 0; i--){
				(_pSPIBase->_pTxData)[_pSPIBase->g_spiLen++] = (pTransaction_t->instr & (0xFF << i * 8)) >> i * 8;
			}
			/* 地址阶段 */
			for(int8_t i = pTransaction_t->addr_bytes - 1; i >= 0; i--){
				(_pSPIBase->_pTxData)[_pSPIBase->g_spiLen++] = (pTransaction_t->addr & (0xFF << i * 8)) >> i * 8;
			}
			/* 等待阶段 */
			for(int8_t i = pTransaction_t->dummy_bytes - 1; i >= 0; i--){
				(_pSPIBase->_pTxData)[_pSPIBase->g_spiLen++] = 0xFFU;
			}
			_pSPIBase->baseTransfer(pTransaction_t->cmd_transfer_mode);	// 轮询传输
		}
	void busSetCS(uint8_t _Level);
private:
	void busTransfer();
	void busSetParam();
	void busEnter();
	void busExit();
	uint8_t busBusy();

	void SF_CS_LOW(){
		if(_EN_SF_CS)
			_SF_CS_GPIOx->BSRR = ((uint32_t)_SF_CS_GPIO_Pin << 16U);
	}
	void SF_CS_HIGH(){
		if(_EN_SF_CS)
			_SF_CS_GPIOx->BSRR = _SF_CS_GPIO_Pin;
	}

	FRTOS_SPIBase *_pSPIBase;				// 总线对象
	transfer_mode_t _spiTransMode;			// 传输方式
	GPIO_TypeDef *_SF_CS_GPIOx;	 			// 软件片选IO组
	uint16_t _SF_CS_GPIO_Pin;				// 软件片选引脚标号

	/* 用于同一总线不同设备切换通信模式 */
	uint32_t _BaudRatePrescaler;			//分频
	uint32_t _CLKPhase;						//相位
	uint32_t _CLKPolarity;					//极性

	/* 硬件还是软件CS */
	bool _EN_SF_CS;
	/* 配置命令序列的帧结构 */
};

class FRTOS_SPIDev_ESP32_C3_AT :public FRTOS_SPICmd{
public:
	FRTOS_SPIDev_ESP32_C3_AT(FRTOS_SPIBase *pSPIBase,
			GPIO_TypeDef *SF_CS_GPIOx,
			uint16_t SF_CS_GPIO_Pin,
			uint32_t BaudRatePrescaler,
			uint32_t CLKPhase,
			uint32_t CLKPolarity)
	:FRTOS_SPICmd(pSPIBase, SF_CS_GPIOx, SF_CS_GPIO_Pin, BaudRatePrescaler, CLKPhase, CLKPolarity)
	{}
	void spi_device_polling_transmit(spi_transaction_t * pTransaction_t){
		busSetCS(0); 			/* 片选拉低 */
		if(pTransaction_t->cmd_transfer_mode != TRANSFER_MODE_NONE){
			busTransferCmd(pTransaction_t);
		}
		if(pTransaction_t->tx_size != 0 ){
			busTransmit(pTransaction_t);
		}
		if(pTransaction_t->rx_size != 0 ){
			busReceive(pTransaction_t);
		}
		busSetCS(1); 			/* 片选拉高 */
	}

};

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

	}
	/** @brief: 	DMA模式快速读
	  *	@param: 	addr 以字节寻址的地址位
	  * @param: 	size 读出个数
	  */
	uint16_t SPISRAM_FastRead_DMA(uint32_t addr, uint16_t size, uint8_t* buf)
	{
		spi_transaction_t trans = {
				.instr = INSTR_READ_FAST,						/* 命令 */
				.addr = addr,								/* 地址 */
				.rx_buffer = buf,							/* 数据 */
				.rx_size = size,
				.instr_bytes = 1,							/* 命令字节数 */
				.addr_bytes = 3,							/* 地址字节数 */
				.dummy_bytes = 1,							/* 等待字节数 */
				.cmd_transfer_mode = TRANSFER_MODE_POLL,	/* 指令阶段轮询传输 */
				.data_transfer_mode = TRANSFER_MODE_DMA		/* 数据阶段DMA传输 */
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
				.instr = INSTR_WRITE,							/* 命令 */
				.addr = addr,								/* 地址 */
				.tx_buffer = buf,							/* 数据 */
				.tx_size = size,
				.instr_bytes = 1,							/* 命令字节数 */
				.addr_bytes = 3,							/* 地址字节数 */
				.cmd_transfer_mode = TRANSFER_MODE_POLL,	/* 指令阶段轮询传输 */
				.data_transfer_mode = TRANSFER_MODE_DMA		/* 数据阶段DMA传输 */
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
				.instr = INSTR_READ,							/* 命令 */
				.addr = addr,								/* 地址 */
				.rx_buffer = buf,							/* 数据 */
				.rx_size = size,
				.instr_bytes = 1,							/* 命令字节数 */
				.addr_bytes = 3,							/* 地址字节数 */
				.cmd_transfer_mode = TRANSFER_MODE_POLL,	/* 指令阶段轮询传输 */
				.data_transfer_mode = TRANSFER_MODE_POLL	/* 数据阶段DMA传输 */
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
				.tx_size = size,
				.instr_bytes = 1,							/* 命令字节数 */
				.addr_bytes = 3,							/* 地址字节数 */
				.cmd_transfer_mode = TRANSFER_MODE_POLL,	/* 指令阶段轮询传输 */
				.data_transfer_mode = TRANSFER_MODE_POLL	/* 数据阶段DMA传输 */
		};
		busSetCS(0);			/* 片选拉低 */
		busTransferCmd(&trans);
		busTransmit(&trans); 	/* 片选拉高 */
		busSetCS(1);
		return 0;
	}
private:
	static const uint8_t INSTR_READ = 0x03U;		// 读指令 ‘h03（最高频率 33 MHz）
	static const uint8_t INSTR_READ_FAST = 0x0BU;	// 快速读指令 ‘h0B （最高频率 133 MHz）
	static const uint8_t INSTR_READ_ID = 0x9FU;		// 读 ID 指令 ‘h9F
	static const uint8_t INSTR_WRITE = 0x02U;		// 写指令 ‘h02

};

extern FRTOS_SPIBase SPI2_Base;
}

#endif	/* __cplusplus */
#endif	/* HAL_SPI_MODULE_ENABLED */
#endif /* INC_FRTOS_SPI_H */

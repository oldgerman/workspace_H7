/*
 * frtos_spi.h
 *
 *  Created on: 2022年12月06日
 *      @Author: OldGerman (过气德国佬)
 *		@Modified: 使用C++封装一次安富莱的bsp_spi_bus，可以创建多路SPI总线的实例，
 *					构造时指向多个设备实例，支持同一路总线的设备互斥
 *		@Note：中断和DMA方式仅支持标准SPI的全双工，阻塞式支持仅收或仅发送
 */

#ifndef INC_FRTOS_SPI_H
#define INC_FRTOS_SPI_H
#ifdef __cplusplus
extern "C" {
#endif

/* 私有包含 Private includes -------------------------------------------------*/
#include "spi.h"
#include "cmsis_os.h"
#include "queue.h"		    //提供消息队列
#include "semphr.h"			//提供信号量
#include "task.h"
#include "frtos_spi_conf.h"
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
namespace ns_frtos_spi
{
/* transfer mode */
typedef enum {
	TRANSFER_MODE_NONE = 0,	/* 不传输 */
	TRANSFER_MODE_DMA,    	/* DMA  */
	TRANSFER_MODE_INT,    	/* 中断 */
	TRANSFER_MODE_POLL,   	/* 查询 */
	TRANSMIT_MODE_POLL,    	/* 查询发送 */
	RECEIVE_MODE_POLL,   	/* 查询接收 */
}transfer_mode_t;

/* transfer state */
typedef enum {
	TRANSFER_STATE_WAIT = 0,
	TRANSFER_STATE_COMPLETE,
	TRANSFER_STATE_ERROR
}transfer_state_t;

/**
 * This structure describes one SPI transaction. The descriptor should not be modified until the transaction finishes.
 */
typedef  struct {
	uint32_t instr;
	uint32_t addr;
	uint8_t *tx_buffer;     	 	// Pointer to transmit buffer, or NULL for no MOSI phase
	uint8_t *rx_buffer;            	// Pointer to receive buffer, or NULL for no MISO phase. Written by 4 bytes-unit if DMA is used.（这个和H7 DMA SPI需要四字节对齐一样）

    uint8_t instr_bytes;           		// 指令阶段 发送的数据量，单位 bytes.
    uint8_t addr_bytes;           		// 地址阶段 发送的数据量，单位 bytes.
    uint8_t dummy_bytes;             	// 空闲阶段 发送的数据量，单位 bytes.
    uint16_t data_bytes;                // 数据阶段 收发的数据量，单位 bytes.

    transfer_mode_t cmd_transfer_mode;	// 命令序列 发送的模式
    transfer_mode_t data_transfer_mode;	// 数据阶段 收发的模式

    uint32_t 	timeout;				// 超时时间
    uint8_t		*user;                  // 用户定义的变量，可用于存储ID。
//  uint16_t data_transfer_osDelay_time;//根据非阻塞传输的字节数和波特率估计baseTransferExt()最少osDelay()阻塞时间，暂时不用
}spi_transaction_t ;        			// 若使用DMA，收发缓冲区的地址必须要32字节对齐，且要关闭cache或读操作时清除SCB_InvalidateDCache_by_Addr

/* SPI 传输基础类 */
class FRTOS_SPIBase{
public:
	FRTOS_SPIBase(SPI_HandleTypeDef &hspi, uint8_t*pTxData, uint8_t*pRxData, uint16_t sizeBuf);
	~FRTOS_SPIBase(){}

	void baseSetParam(uint32_t _BaudRatePrescaler, uint32_t _CLKPhase, uint32_t _CLKPolarity);
	void baseTransfer(transfer_mode_t spiTransMode);
	void baseTransferExt(transfer_mode_t spiTransMode, uint8_t* pTxData, uint8_t* pRxData, uint16_t size);
	void baseEnter();
	void baseExit();
	bool baseBusy(void);

	void TxRxCpltCallback(SPI_HandleTypeDef *hspi);
	void ErrorCallback(SPI_HandleTypeDef *hspi);

	SPI_HandleTypeDef  &_hspi;
	uint8_t *_pTxData;
	uint8_t *_pRxData;
	uint32_t _bufferCursor;	// 缓冲区游标，DMA、BDMA都仅支持1~65535个，但为了检测超出65535，使用uin32_t类型
	static const uint8_t sizeCmdOnly = 4 * 3;	//FRTOS_SPIBase对象的buffer仅用作处理FRTOS_SPICmd对象的命令序列时的大小

private:
	const uint16_t _size;	// 构造时将提前定义的全局buffer的地址和大小作为参数传入
	/* 备份SPI几个关键传输参数，波特率，相位，极性. 如果不同外设切换，需要重新Init SPI参数 */
	uint32_t 	s_BaudRatePrescaler;
	uint32_t 	s_CLKPhase;
	uint32_t 	s_CLKPolarity;
	uint8_t 	g_spi_busy;
	volatile uint32_t wTransferState;
#if RTOS_EN
	SemaphoreHandle_t 	spiMutex;		/* 互斥信号量：标记SPI总线占用 */ //互斥信号量仅支持用在 FreeRTOS 的任务中，中断函数中不可使用
	StaticSemaphore_t 	spiMutexBuffer;
#else
	bool				spiMutex;
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

	void busTransferCmd(spi_transaction_t * pTransaction_t);
	void busTransferExtData(spi_transaction_t * pTransaction_t);
	void busTransferExtCmdAndData(spi_transaction_t * pTransaction_t);
	void busSetCS(uint8_t _Level);

private:
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
};

} 	/* NS_FRTOS_SPI */
using namespace ns_frtos_spi;

}		/* extern "C" */
#endif	/* __cplusplus */
#endif /* INC_FRTOS_SPI_H */

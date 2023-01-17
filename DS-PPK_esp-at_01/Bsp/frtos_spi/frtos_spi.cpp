/*
 * frtos_spi.cpp
 *
 *  Created on: 2022年12月06日
 *      @Author: OldGerman (过气德国佬)
 *		@Modified: 使用C++封装一次安富莱的bsp_spi_bus，
 *					去除V7开发板的特化代码，适配CubeMX可以创建多路SPI总线的实例，
 *					构造时指向多个设备实例，支持同一路总线的设备互斥
 */

#include "common_inc.h"
#include "frtos_spi.h"
#include "FreeRTOS.h"
#include "semphr.h"
//ALIGN_32BYTES(uint8_t _pRxData[_size]);	/* 必须32字节对齐 */

/**
  *	@brief: 	构造FRTOS_SPICmd对象
  *	@param: 	pSPIBase			FRTOS_SPIBase对象指针
  *	@param: 	spiTransMode		传输方式
  *	@param: 	SF_CS_GPIOx			软件片选IO组
  *	@param: 	SF_CS_GPIO_Pin		软件片选引脚标号
  *	@param:		BaudRatePrescaler	分频	取值只能是：@defgroup SPI_BaudRate_Prescaler
  *	@param:		CLKPhase			相位	取值只能是：@defgroup SPI_Clock_Phase
  *	@param:		CLKPolarity			极性	取值只能是：@defgroup SPI_Clock_Polarity
  *	@retval: 	None
  */
FRTOS_SPICmd::FRTOS_SPICmd(FRTOS_SPIBase *pSPIBase,
		GPIO_TypeDef *SF_CS_GPIOx,
		uint16_t SF_CS_GPIO_Pin,
		uint32_t BaudRatePrescaler,
		uint32_t CLKPhase,
		uint32_t CLKPolarity)
: _pSPIBase(pSPIBase),
  _SF_CS_GPIOx(SF_CS_GPIOx),
  _SF_CS_GPIO_Pin(SF_CS_GPIO_Pin),
  _BaudRatePrescaler(BaudRatePrescaler),
  _CLKPhase(CLKPhase),
  _CLKPolarity(CLKPolarity)
{
	_EN_SF_CS = true;
}

/**
  *	@brief: 	构造FRTOS_SPICmd对象
  *	@param: 	pSPIBase			FRTOS_SPIBase对象指针
  *	@param: 	spiTransMode		传输方式
  *	@param:		BaudRatePrescaler	分频	取值只能是：@defgroup SPI_BaudRate_Prescaler
  *	@param:		CLKPhase			相位	取值只能是：@defgroup SPI_Clock_Phase
  *	@param:		CLKPolarity			极性	取值只能是：@defgroup SPI_Clock_Polarity
  *	@retval: 	None
  */
FRTOS_SPICmd::FRTOS_SPICmd(FRTOS_SPIBase *pSPIBase,
		uint32_t BaudRatePrescaler,
		uint32_t CLKPhase,
		uint32_t CLKPolarity)
: _pSPIBase(pSPIBase),
  _BaudRatePrescaler(BaudRatePrescaler),
  _CLKPhase(CLKPhase),
  _CLKPolarity(CLKPolarity)
{
	_EN_SF_CS = false;
}

/**
  *	@brief: 	片选控制函数
  *	@param: 	None
  *	@retval: 	None
  */
void FRTOS_SPICmd::busSetCS(uint8_t _Level)
{
	if (_Level == 0)
	{
		_pSPIBase->baseEnter();			// 占用SPI总线
		_pSPIBase->baseSetParam(_BaudRatePrescaler, _CLKPhase, _CLKPolarity); 		// 一个SPI总线挂载多个设备时，每次SPI总线被其中一个设备占用，就根据其SPI的参数要求更改SPI配置
		SF_CS_LOW();		// CS拉低
	}
	else 					// _Level == 1
	{
		SF_CS_HIGH();		// CS拉高
		_pSPIBase->baseExit();			// 释放SPI总线
	}
}

/**
  *	@brief: 	发送命令序列的cmd部分，使用成员_pSPIBase的缓冲区
  *	@param: 	pTransaction_t	spi_transaction_t结构
  *	@retval: 	None
  */
void FRTOS_SPICmd::busTransferCmd(spi_transaction_t * pTransaction_t){
	/* 复位缓冲区游标 */
	_pSPIBase->_bufferCursor = 0;
	/* 命令阶段*/
	for(int8_t i = pTransaction_t->instr_bytes - 1; i >= 0; i--){
		(_pSPIBase->_pTxData)[_pSPIBase->_bufferCursor++] = (pTransaction_t->instr & (0xFF << i * 8)) >> i * 8;
	}
	/* 地址阶段 */
	for(int8_t i = pTransaction_t->addr_bytes - 1; i >= 0; i--){
		(_pSPIBase->_pTxData)[_pSPIBase->_bufferCursor++] = (pTransaction_t->addr & (0xFF << i * 8)) >> i * 8;
	}
	/* 等待阶段 */
	for(int8_t i = pTransaction_t->dummy_bytes - 1; i >= 0; i--){
		(_pSPIBase->_pTxData)[_pSPIBase->_bufferCursor++] = 0xFFU;
	}
	_pSPIBase->baseTransfer(pTransaction_t->cmd_transfer_mode);
}

/**
  *	@brief: 	发送命令序列的data部分，使用外部缓冲区
  *	@param: 	pTransaction_t	spi_transaction_t结构
  *	@retval: 	None
  */
void  FRTOS_SPICmd::busTransferExtData(spi_transaction_t * pTransaction_t){
	_pSPIBase->baseTransferExt(pTransaction_t->data_transfer_mode,
			pTransaction_t->tx_buffer,
			pTransaction_t->rx_buffer,
			pTransaction_t->data_bytes);
}

/**
  *	@brief: 	发送命令序列的cmd和data部分，cmd部分使用成员_pSPIBase的缓冲区，data部分使用外部缓冲区
  *	@param: 	pTransaction_t	spi_transaction_t结构
  *	@retval: 	None
  */
void FRTOS_SPICmd::busTransferExtCmdAndData(spi_transaction_t * pTransaction_t){
	busSetCS(0); /* 片选拉低 */
	if(pTransaction_t->cmd_transfer_mode != TRANSFER_MODE_NONE){
		busTransferCmd(pTransaction_t);
	}
	if(pTransaction_t->data_bytes != 0 ){
		busTransferExtData(pTransaction_t);
	}
	busSetCS(1); /* 片选拉高 */
}

/**
  *	@brief: 	构造FRTOS_SPIBase对象
  *	@param: 	hspi				SPI_HandleTypeDef变量的引用
  *	@param: 	pTxData				发送缓冲区指针
  *	@param: 	pRxData				接收缓冲区指针
  *	@param: 	sizeBuf				收发缓冲区大小
  *	@retval: 	None
 */
FRTOS_SPIBase::FRTOS_SPIBase(SPI_HandleTypeDef &hspi, uint8_t *pTxData, uint8_t *pRxData, uint16_t sizeBuf)
:_hspi(hspi), _pTxData(pTxData), _pRxData(pRxData), _size(sizeBuf){
#if RTOS_EN
	spiMutex = xSemaphoreCreateMutexStatic(&spiMutexBuffer);
	xSemaphoreGive(spiMutex);
#else
	spiMutex = 0;
#endif
	wTransferState = TRANSFER_STATE_WAIT;
}

/**
  *	@brief: 	TxRx传输完成回调函数，需要被HAL_SPI_TxRxCpltCallback()调用
  * @param  	hspi: SPI handle
  *	@retval: 	None
 */
void FRTOS_SPIBase::TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
	if(hspi == &_hspi) {
	    wTransferState = TRANSFER_STATE_COMPLETE;
	}
}

/**
  *	@brief: 	传输错误回调函数，需要被HAL_SPI_ErrorCallback()调用
  * @param  	hspi: SPI handle
  *	@retval: 	None
 */
void FRTOS_SPIBase::ErrorCallback(SPI_HandleTypeDef *hspi) {
	if(hspi == &_hspi) {
	    wTransferState = TRANSFER_STATE_ERROR;
	}
}

/**
  *	@brief: 	配置SPI总线参数，波特率
  *	@param: 	None
  *	@retval: 	None
  */
void FRTOS_SPIBase::baseSetParam(uint32_t BaudRatePrescaler, uint32_t CLKPhase, uint32_t CLKPolarity)
{
	/* 提高执行效率，只有在SPI硬件参数发生变化时，才执行HAL_Init */
	if (s_BaudRatePrescaler == BaudRatePrescaler && s_CLKPhase == CLKPhase && s_CLKPolarity == CLKPolarity) {
		return;
	}

	s_BaudRatePrescaler = BaudRatePrescaler;
	s_CLKPhase = CLKPhase;
	s_CLKPolarity = CLKPolarity;
	
	/* Configure the SPI peripheral */
	/* Set the SPI parameters */
	_hspi.Init.BaudRatePrescaler = s_BaudRatePrescaler;
	_hspi.Init.CLKPhase          = s_CLKPhase;
	_hspi.Init.CLKPolarity       = s_CLKPolarity;

	if (HAL_SPI_Init(&_hspi) != HAL_OK) {
		Error_Handler();
	}	
}

/**
  *	@brief: 	启动数据收发，使用对象内部的参数
  *	@param:  	spiTransMode 数据传输模式
  *	@retval: 	None
  */
void FRTOS_SPIBase::baseTransfer(transfer_mode_t spiTransMode)
{
	if (_bufferCursor > _size) {
		return;
	}
	baseTransferExt(spiTransMode, _pTxData, _pRxData, _bufferCursor);
}

/**
  *	@brief: 	启动数据收发，使用对象内部的参数
  *	@param:  	spiTransMode 数据传输模式
  *	@retval: 	None
  */
void FRTOS_SPIBase::baseTransferExt(transfer_mode_t spiTransMode, uint8_t* pTxData, uint8_t* pRxData, uint16_t size)
{
	switch(spiTransMode){
	case TRANSFER_MODE_DMA:
		wTransferState = TRANSFER_STATE_WAIT;

		while (_hspi.State != HAL_SPI_STATE_READY);

		if(HAL_SPI_TransmitReceive_DMA(&_hspi, (uint8_t*)pTxData, (uint8_t*)pRxData, size) != HAL_OK) {
			Error_Handler();
		}

		while (wTransferState == TRANSFER_STATE_WAIT) {
#if RTOS_EN
			osDelay(1);
#endif
			// 若非RTOS，等待期间只能处理中断
		}

		/**
		 * Invalidate cache prior to access by CPU
		 * DMA接收完一帧数据之后，需要进行数据解析之前，进行CacheNone效化处理
		 * ◆ 第 1 个参数 addr ： 操作的地址一定要是 32 字节对齐的，即这个地址对 32 求余数等于 0。
		 * ◆ 第 2 个参数 dsize ：一定要是 32 字节的整数倍。
		 */
		SCB_InvalidateDCache_by_Addr ((uint32_t *)pRxData, size);
		break;
	case TRANSFER_MODE_INT:
		wTransferState = TRANSFER_STATE_WAIT;

		while (_hspi.State != HAL_SPI_STATE_READY);

		if(HAL_SPI_TransmitReceive_IT(&_hspi, (uint8_t*)pTxData, (uint8_t*)pRxData, size) != HAL_OK){
			Error_Handler();
		}

		while (wTransferState == TRANSFER_STATE_WAIT){
#if RTOS_EN
			osDelay(1);
			// 若非RTOS，等待期间只能处理中断
#endif
		}
		break;
	case TRANSFER_MODE_POLL:
		if(HAL_SPI_TransmitReceive(&_hspi, (uint8_t*)pTxData, (uint8_t*)pRxData, size, HAL_MAX_DELAY) != HAL_OK){
			Error_Handler();
		}
		break;
	case TRANSMIT_MODE_POLL:
		if(HAL_SPI_Transmit(&_hspi, (uint8_t*)pTxData, size, HAL_MAX_DELAY) != HAL_OK){
			Error_Handler();
		}
		break;
	case RECEIVE_MODE_POLL:
		if(HAL_SPI_Receive(&_hspi, (uint8_t*)pRxData, size, HAL_MAX_DELAY) != HAL_OK){
			Error_Handler();
		}
		break;
	default:
		break;
	}
}

/**
  *	@brief: 	占用SPI总线
  *	@param: 	None
  *	@retval: 	None
  */
void FRTOS_SPIBase::baseEnter(void)
{
#if RTOS_EN
	while (xSemaphoreTake(spiMutex, portMAX_DELAY) != pdPASS);
#else
	spiMutex = 1;
#endif
}

/**
  *	@brief: 	释放占用的SPI总线
  *	@param: 	None
  *	@retval: 	None
  */
void FRTOS_SPIBase::baseExit(void)
{
#if RTOS_EN
	xSemaphoreGive(spiMutex);
#else
	spiMutex = 0;
#endif
}

/**
  *	@brief: 	判断SPI总线忙。方法是检测其他SPI芯片的片选信号是否为1
  *	@param: 	None
  *	@retval: 	0 表示不忙  1表示忙
  */
bool FRTOS_SPIBase::baseBusy(void)
{
#if RTOS_EN
	if (xSemaphoreTake(spiMutex, 0) != pdPASS){
		return true;
	}
	else {
		return false;
	}
#else
	return spiMutex;
#endif
}

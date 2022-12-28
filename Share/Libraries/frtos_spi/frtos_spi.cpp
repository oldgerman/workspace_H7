/*
 * frtos_spi.cpp
 *
 *  Created on: 2022年12月06日
 *      @Author: OldGerman (过气德国佬)
 *		@Modified: 使用C++封装一次安富莱的bsp_spi_bus，
 *					去除V7开发板的特化代码，适配CubeMX可以创建多路SPI总线的实例，
 *					构造时指向多个设备实例，支持同一路总线的设备互斥
 */

#include "bsp.h"
#include "frtos_spi.h"

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
  *	@retval: 	无
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
  *	@retval: 	无
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
  *	@brief: 	串行FALSH片选控制函数
  *	@param: 	无
  *	@retval: 	无
  */
void FRTOS_SPICmd::busSetCS(uint8_t _Level)
{
	if (_Level == 0)
	{
		busEnter();			// 占用SPI总线
		busSetParam(); 		// 一个SPI总线挂载多个设备时，每次SPI总线被其中一个设备占用，就根据其SPI的参数要求更改SPI配置
		SF_CS_LOW();		// CS拉低
	}
	else 					// _Level == 1
	{
		SF_CS_HIGH();		// CS拉高
		busExit();			// 释放SPI总线
	}
}

void FRTOS_SPICmd::busSetParam(){
	_pSPIBase->baseSetParam(_BaudRatePrescaler, _CLKPhase, _CLKPolarity);
}
void FRTOS_SPICmd::busTransfer(void){
	_pSPIBase->baseTransfer(_spiTransMode);
}
void FRTOS_SPICmd::busEnter(void){
	_pSPIBase->baseEnter();
}
void FRTOS_SPICmd::busExit(void){
	_pSPIBase->baseExit();
}
uint8_t FRTOS_SPICmd::busBusy(void){
	return _pSPIBase->baseBusy();
}

/**
  *	@brief: 	配置SPI总线。 只包括 SCK、 MOSI、 MISO口线的配置。不包括片选CS，也不包括外设芯片特有的INT、BUSY等
  *	@param: 	无
  *	@retval: 	无
  */
void FRTOS_SPIBase::baseInitBus(void)
{	
	g_spi_busy = 0;
	wTransferState = TRANSFER_STATE_WAIT;
	baseSetParam(SPI_BAUDRATEPRESCALER_8, SPI_PHASE_1EDGE, SPI_POLARITY_LOW);
}

/**
  *	函 数 名: bsp_InitSPIParam
  *	@brief: 配置SPI总线参数，波特率、
  *	@param: 无
  *	@retval: 无
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
  *	@brief: 	启动数据传输，使用对象内部的参数
  *	@param:  	spiTransMode 数据传输模式
  *	@retval: 	无
  */
void FRTOS_SPIBase::baseTransfer(transfer_mode_t spiTransMode)
{
//	HAL_StatusTypeDef ret = HAL_OK;
	if (g_spiLen > _size) {
		return;
	}
	baseTransferExt(spiTransMode, _pTxData, _pRxData, g_spiLen);
}

/**
  *	@brief: 	启动数据传输，使用对象内部的参数
  *	@param:  	spiTransMode 数据传输模式
  *	@retval: 	无
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
			//osDelay(10);
			// 若非RTOS，等待期间只能处理中断
		}

		/**
		 * Invalidate cache prior to access by CPU
		 * DMA接收完一帧数据之后，需要进行数据解析之前，进行Cache无效化处理
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
			//osDelay(10);
			// 若非RTOS，等待期间只能处理中断
		}
		break;
	case TRANSFER_MODE_POLL:
		if(HAL_SPI_TransmitReceive(&_hspi, (uint8_t*)pTxData, (uint8_t*)pRxData, size, HAL_MAX_DELAY) != HAL_OK){
			Error_Handler();
		}
		break;
	default:
		break;
	}
}

/**
  *	@brief: 	占用SPI总线
  *	@param: 	无
  *	@retval: 	0 表示不忙  1表示忙
  */
void FRTOS_SPIBase::baseEnter(void)
{
	g_spi_busy = 1;
}

/**
  *	@brief: 	释放占用的SPI总线
  *	@param: 	无
  *	@retval: 	0 表示不忙  1表示忙
  */
void FRTOS_SPIBase::baseExit(void)
{
	g_spi_busy = 0;
}

/**
  *	@brief: 判断SPI总线忙。方法是检测其他SPI芯片的片选信号是否为1
  *	@param: 无
  *	@retval: 0 表示不忙  1表示忙
  */
uint8_t FRTOS_SPIBase::baseBusy(void)
{
	return g_spi_busy;
}

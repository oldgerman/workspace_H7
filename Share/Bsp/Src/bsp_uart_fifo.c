/*
*********************************************************************************************************
*
*	模块名称 : 串口中断+FIFO驱动模块
*	文件名称 : bsp_uart_fifo.h
*	说    明 : 头文件
*
*	Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
*
*	@Modified By OldGerman
*	@Modified 2022/08
*			  	为Stm32CubeIDE环境重定向printf和scanf，目前仅支持PA9 PA10 的 USART1
*
*********************************************************************************************************
*/

#include "bsp_config.h"
#ifdef EN_BSP_UART_FIFO

#include "bsp.h"
#include "usart.h"

/* 定义每个串口结构体变量 */
#if UART1_FIFO_EN == 1
	static UART_T g_tUart1;
	static uint8_t g_TxBuf1[UART1_TX_BUF_SIZE];		/* 发送缓冲区 */
	static uint8_t g_RxBuf1[UART1_RX_BUF_SIZE];		/* 接收缓冲区 */
#endif

static void UartVarInit(void);

//static void InitHardUart(void);
static void UartSend(UART_T *_pUart, uint8_t *_ucaBuf, uint16_t _usLen);
static uint8_t UartGetChar(UART_T *_pUart, uint8_t *_pByte);
//static void UartIRQ(UART_T *_pUart);

//void RS485_InitTXE(void);

/*
*********************************************************************************************************
*	函 数 名: bsp_InitUart
*	功能说明: 初始化串口硬件，并对全局变量赋初值.
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitUart(void)
{
	SET_BIT(USART1->ICR, USART_ICR_TCCF);	/* 清除TC发送完成标志*/
	SET_BIT(USART1->RQR, USART_RQR_RXFRQ);	/* 清除RXNE接收标志 */
	SET_BIT(USART1->CR1, USART_CR1_RXNEIE);	/* 使能PE，RX接受中断 */
	
	UartVarInit();		/* 必须先初始化全局变量,再配置硬件 */
//	InitHardUart();		/* 配置串口的硬件参数(波特率等) */
//	RS485_InitTXE();	/* 配置RS485芯片的发送使能硬件，配置为推挽输出 */
}

/*
*********************************************************************************************************
*	函 数 名: ComToUart
*	功能说明: 将COM端口号转换为UART指针
*	形    参: _ucPort: 端口号(COM1 - COM8)
*	返 回 值: uart指针
*********************************************************************************************************
*/
UART_T *ComToUart(COM_PORT_E _ucPort)
{
	if (_ucPort == COM1)
	{
		#if UART1_FIFO_EN == 1
			return &g_tUart1;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM2)
	{
		#if UART2_FIFO_EN == 1
			return &g_tUart2;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM3)
	{
		#if UART3_FIFO_EN == 1
			return &g_tUart3;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM4)
	{
		#if UART4_FIFO_EN == 1
			return &g_tUart4;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM5)
	{
		#if UART5_FIFO_EN == 1
			return &g_tUart5;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM6)
	{
		#if UART6_FIFO_EN == 1
			return &g_tUart6;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM7)
	{
		#if UART7_FIFO_EN == 1
			return &g_tUart7;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM8)
	{
		#if UART8_FIFO_EN == 1
			return &g_tUart8;
		#else
			return 0;
		#endif
	}	
	else
	{
		Error_Handler();
		return 0;
	}
}

/*
*********************************************************************************************************
*	函 数 名: ComToUart
*	功能说明: 将COM端口号转换为 USART_TypeDef* USARTx
*	形    参: _ucPort: 端口号(COM1 - COM8)
*	返 回 值: USART_TypeDef*,  USART1, USART2, USART3, UART4, UART5，USART6，UART7，UART8。
*********************************************************************************************************
*/
USART_TypeDef *ComToUSARTx(COM_PORT_E _ucPort)
{
	if (_ucPort == COM1)
	{
		#if UART1_FIFO_EN == 1
			return USART1;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM2)
	{
		#if UART2_FIFO_EN == 1
			return USART2;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM3)
	{
		#if UART3_FIFO_EN == 1
			return USART3;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM4)
	{
		#if UART4_FIFO_EN == 1
			return UART4;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM5)
	{
		#if UART5_FIFO_EN == 1
			return UART5;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM6)
	{
		#if UART6_FIFO_EN == 1
			return USART6;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM7)
	{
		#if UART7_FIFO_EN == 1
			return UART7;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM8)
	{
		#if UART8_FIFO_EN == 1
			return UART8;
		#else
			return 0;
		#endif
	}	
	
	else
	{
		/* 不做任何处理 */
		return 0;
	}
}

/*
*********************************************************************************************************
*	函 数 名: comSendBuf
*	功能说明: 向串口发送一组数据。数据放到发送缓冲区后立即返回，由中断服务程序在后台完成发送
*	形    参: _ucPort: 端口号(COM1 - COM8)
*			  _ucaBuf: 待发送的数据缓冲区
*			  _usLen : 数据长度
*	返 回 值: 无
*********************************************************************************************************
*/
void comSendBuf(COM_PORT_E _ucPort, uint8_t *_ucaBuf, uint16_t _usLen)
{
	UART_T *pUart;

	pUart = ComToUart(_ucPort);
	if (pUart == 0)
	{
		return;
	}

	if (pUart->SendBefor != 0)
	{
		pUart->SendBefor();		/* 如果是RS485通信，可以在这个函数中将RS485设置为发送模式 */
	}

	UartSend(pUart, _ucaBuf, _usLen);
}

/*
*********************************************************************************************************
*	函 数 名: comSendChar
*	功能说明: 向串口发送1个字节。数据放到发送缓冲区后立即返回，由中断服务程序在后台完成发送
*	形    参: _ucPort: 端口号(COM1 - COM8)
*			  _ucByte: 待发送的数据
*	返 回 值: 无
*********************************************************************************************************
*/
void comSendChar(COM_PORT_E _ucPort, uint8_t _ucByte)
{
	comSendBuf(_ucPort, &_ucByte, 1);
}

/*
*********************************************************************************************************
*	函 数 名: comGetChar
*	功能说明: 从接收缓冲区读取1字节，非阻塞。无论有无数据均立即返回。
*	形    参: _ucPort: 端口号(COM1 - COM8)
*			  _pByte: 接收到的数据存放在这个地址
*	返 回 值: 0 表示无数据, 1 表示读取到有效字节
*********************************************************************************************************
*/
uint8_t comGetChar(COM_PORT_E _ucPort, uint8_t *_pByte)
{
	UART_T *pUart;

	pUart = ComToUart(_ucPort);
	if (pUart == 0)
	{
		return 0;
	}

	return UartGetChar(pUart, _pByte);
}

/*
*********************************************************************************************************
*	函 数 名: comClearTxFifo
*	功能说明: 清零串口发送缓冲区
*	形    参: _ucPort: 端口号(COM1 - COM8)
*	返 回 值: 无
*********************************************************************************************************
*/
void comClearTxFifo(COM_PORT_E _ucPort)
{
	UART_T *pUart;

	pUart = ComToUart(_ucPort);
	if (pUart == 0)
	{
		return;
	}

	pUart->usTxWrite = 0;
	pUart->usTxRead = 0;
	pUart->usTxCount = 0;
}

/*
*********************************************************************************************************
*	函 数 名: comClearRxFifo
*	功能说明: 清零串口接收缓冲区
*	形    参: _ucPort: 端口号(COM1 - COM8)
*	返 回 值: 无
*********************************************************************************************************
*/
void comClearRxFifo(COM_PORT_E _ucPort)
{
	UART_T *pUart;

	pUart = ComToUart(_ucPort);
	if (pUart == 0)
	{
		return;
	}

	pUart->usRxWrite = 0;
	pUart->usRxRead = 0;
	pUart->usRxCount = 0;
}

/*
*********************************************************************************************************
*	函 数 名: comSetBaud
*	功能说明: 设置串口的波特率. 本函数固定设置为无校验，收发都使能模式
*	形    参: _ucPort: 端口号(COM1 - COM8)
*			  _BaudRate: 波特率，8倍过采样  波特率.0-12.5Mbps
*                                16倍过采样 波特率.0-6.25Mbps
*	返 回 值: 无
*********************************************************************************************************
*/
void comSetBaud(COM_PORT_E _ucPort, uint32_t _BaudRate)
{
	USART_TypeDef* USARTx;
	
	USARTx = ComToUSARTx(_ucPort);
	if (USARTx == 0)
	{
		return;
	}
	
	bsp_SetUartParam(USARTx,  _BaudRate, UART_PARITY_NONE, UART_MODE_TX_RX);
}

/*
*********************************************************************************************************
*	函 数 名: UartVarInit
*	功能说明: 初始化串口相关的变量
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void UartVarInit(void)
{
#if UART1_FIFO_EN == 1
	g_tUart1.uart = USART1;						/* STM32 串口设备 */
	g_tUart1.pTxBuf = g_TxBuf1;					/* 发送缓冲区指针 */
	g_tUart1.pRxBuf = g_RxBuf1;					/* 接收缓冲区指针 */
	g_tUart1.usTxBufSize = UART1_TX_BUF_SIZE;	/* 发送缓冲区大小 */
	g_tUart1.usRxBufSize = UART1_RX_BUF_SIZE;	/* 接收缓冲区大小 */
	g_tUart1.usTxWrite = 0;						/* 发送FIFO写索引 */
	g_tUart1.usTxRead = 0;						/* 发送FIFO读索引 */
	g_tUart1.usRxWrite = 0;						/* 接收FIFO写索引 */
	g_tUart1.usRxRead = 0;						/* 接收FIFO读索引 */
	g_tUart1.usRxCount = 0;						/* 接收到的新数据个数 */
	g_tUart1.usTxCount = 0;						/* 待发送的数据个数 */
	g_tUart1.SendBefor = 0;						/* 发送数据前的回调函数 */
	g_tUart1.SendOver = 0;						/* 发送完毕后的回调函数 */
	g_tUart1.ReciveNew = 0;						/* 接收到新数据后的回调函数 */
	g_tUart1.Sending = 0;						/* 正在发送中标志 */
#endif
}


/*
*********************************************************************************************************
*	函 数 名: UartSend
*	功能说明: 填写数据到UART发送缓冲区,并启动发送中断。中断处理函数发送完毕后，自动关闭发送中断
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void UartSend(UART_T *_pUart, uint8_t *_ucaBuf, uint16_t _usLen)
{
	uint16_t i;

	for (i = 0; i < _usLen; i++)
	{
		/* 如果发送缓冲区已经满了，则等待缓冲区空 */
		while (1)
		{
			__IO uint16_t usCount;

			DISABLE_INT();
			usCount = _pUart->usTxCount;
			ENABLE_INT();

			if (usCount < _pUart->usTxBufSize)
			{
				break;
			}
			else if(usCount == _pUart->usTxBufSize)/* 数据已填满缓冲区 */
			{
				if((_pUart->uart->CR1 & USART_CR1_TXEIE) == 0)
				{
					SET_BIT(_pUart->uart->CR1, USART_CR1_TXEIE);
				}  
			}
		}

		/* 将新数据填入发送缓冲区 */
		_pUart->pTxBuf[_pUart->usTxWrite] = _ucaBuf[i];

		DISABLE_INT();
		if (++_pUart->usTxWrite >= _pUart->usTxBufSize)
		{
			_pUart->usTxWrite = 0;
		}
		_pUart->usTxCount++;
		ENABLE_INT();
	}

	SET_BIT(_pUart->uart->CR1, USART_CR1_TXEIE);	/* 使能发送中断（缓冲区空） */
}

/*
*********************************************************************************************************
*	函 数 名: UartGetChar
*	功能说明: 从串口接收缓冲区读取1字节数据 （用于主程序调用）
*	形    参: _pUart : 串口设备
*			  _pByte : 存放读取数据的指针
*	返 回 值: 0 表示无数据  1表示读取到数据
*********************************************************************************************************
*/
static uint8_t UartGetChar(UART_T *_pUart, uint8_t *_pByte)
{
	uint16_t usCount;

	/* usRxWrite 变量在中断函数中被改写，主程序读取该变量时，必须进行临界区保护 */
	DISABLE_INT();
	usCount = _pUart->usRxCount;
	ENABLE_INT();

	/* 如果读和写索引相同，则返回0 */
	//if (_pUart->usRxRead == usRxWrite)
	if (usCount == 0)	/* 已经没有数据 */
	{
		return 0;
	}
	else
	{
		*_pByte = _pUart->pRxBuf[_pUart->usRxRead];		/* 从串口接收FIFO取1个数据 */

		/* 改写FIFO读索引 */
		DISABLE_INT();
		if (++_pUart->usRxRead >= _pUart->usRxBufSize)
		{
			_pUart->usRxRead = 0;
		}
		_pUart->usRxCount--;
		ENABLE_INT();
		return 1;
	}
}

/*
*********************************************************************************************************
*   函 数 名: UartTxEmpty
*   功能说明: 判断发送缓冲区是否为空。
*   形    参:  _pUart : 串口设备
*   返 回 值: 1为空。0为不空。
*********************************************************************************************************
*/
uint8_t UartTxEmpty(COM_PORT_E _ucPort)
{
   UART_T *pUart;
   uint8_t Sending;
   
   pUart = ComToUart(_ucPort);
   if (pUart == 0)
   {
      return 0;
   }

   Sending = pUart->Sending;

   if (Sending != 0)
   {
      return 0;
   }
   return 1;
}

static UART_T * UART_HandleToUART_T(UART_HandleTypeDef *huart){
	if(huart == &huart1)
		return 	&g_tUart1;
//	else if
	Error_Handler();
	return NULL;
}




/*	Stm32CubeIDE 重定向printf和scanf
 * https://www.bilibili.com/read/cv12208902/
 */
#ifdef __GNUC__	/*STM32CubeIDE*/
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#define GETCHAR_PROTOTYPE int __io_getchar(FILE *f)

PUTCHAR_PROTOTYPE {
#if 0	/* 将需要printf的字符通过串口中断FIFO发送出去，printf函数会立即返回 */
	comSendChar(COM1, ch);

	return ch;
#else	/* 采用阻塞方式发送每个字符,等待数据发送完毕 */
	/* 写一个字节到USART1 */
	USART1->TDR = ch;

	/* 等待发送结束 */
	while((USART1->ISR & USART_ISR_TC) == 0)
	{}

	return ch;
#endif
}

GETCHAR_PROTOTYPE
{
#if 1	/* 从串口接收FIFO中取1个数据, 只有取到数据才返回 */
	uint8_t ucData;

	while(comGetChar(COM1, &ucData) == 0);

	return ucData;
#else
	/* 等待接收到数据 */
	while((USART1->ISR & USART_ISR_RXNE) == 0)
	{}

	return (int)USART1->RDR;
#endif
}

#endif /* __GNUC__ */


void UartIRQ(UART_HandleTypeDef *huart)
{
	UART_T * _pUart = UART_HandleToUART_T(huart);
	uint32_t isrflags   = READ_REG(_pUart->uart->ISR);
	uint32_t cr1its     = READ_REG(_pUart->uart->CR1);
//	uint32_t cr3its     = READ_REG(_pUart->uart->CR3);

	/* 处理接收中断  */
	if ((isrflags & USART_ISR_RXNE_RXFNE) != RESET)
	{
		/* 从串口接收数据寄存器读取数据存放到接收FIFO */
		uint8_t ch;

		ch = READ_REG(_pUart->uart->RDR);
		_pUart->pRxBuf[_pUart->usRxWrite] = ch;
		if (++_pUart->usRxWrite >= _pUart->usRxBufSize)
		{
			_pUart->usRxWrite = 0;
		}
		if (_pUart->usRxCount < _pUart->usRxBufSize)
		{
			_pUart->usRxCount++;
		}

		/* 回调函数,通知应用程序收到新数据,一般是发送1个消息或者设置一个标记 */
		//if (_pUart->usRxWrite == _pUart->usRxRead)
		//if (_pUart->usRxCount == 1)
		{
			if (_pUart->ReciveNew)
			{
				_pUart->ReciveNew(ch); /* 比如，交给MODBUS解码程序处理字节流 */
			}
		}
	}

	/* 处理发送缓冲区空中断 */
	if ( ((isrflags & USART_ISR_TXE_TXFNF) != RESET) && (cr1its & USART_CR1_TXEIE) != RESET)
	{
		//if (_pUart->usTxRead == _pUart->usTxWrite)
		if (_pUart->usTxCount == 0)
		{
			/* 发送缓冲区的数据已取完时， 禁止发送缓冲区空中断 （注意：此时最后1个数据还未真正发送完毕）*/
			//USART_ITConfig(_pUart->uart, USART_IT_TXE, DISABLE);
			CLEAR_BIT(_pUart->uart->CR1, USART_CR1_TXEIE);

			/* 使能数据发送完毕中断 */
			//USART_ITConfig(_pUart->uart, USART_IT_TC, ENABLE);
			SET_BIT(_pUart->uart->CR1, USART_CR1_TCIE);
		}
		else
		{
			_pUart->Sending = 1;

			/* 从发送FIFO取1个字节写入串口发送数据寄存器 */
			//USART_SendData(_pUart->uart, _pUart->pTxBuf[_pUart->usTxRead]);
			_pUart->uart->TDR = _pUart->pTxBuf[_pUart->usTxRead];
			if (++_pUart->usTxRead >= _pUart->usTxBufSize)
			{
				_pUart->usTxRead = 0;
			}
			_pUart->usTxCount--;
		}

	}
	/* 数据bit位全部发送完毕的中断 */
	if (((isrflags & USART_ISR_TC) != RESET) && ((cr1its & USART_CR1_TCIE) != RESET))
	{
		//if (_pUart->usTxRead == _pUart->usTxWrite)
		if (_pUart->usTxCount == 0)
		{
			/* 如果发送FIFO的数据全部发送完毕，禁止数据发送完毕中断 */
			//USART_ITConfig(_pUart->uart, USART_IT_TC, DISABLE);
			CLEAR_BIT(_pUart->uart->CR1, USART_CR1_TCIE);

			/* 回调函数, 一般用来处理RS485通信，将RS485芯片设置为接收模式，避免抢占总线 */
			if (_pUart->SendOver)
			{
				_pUart->SendOver();
			}

			_pUart->Sending = 0;
		}
		else
		{
			/* 正常情况下，不会进入此分支 */

			/* 如果发送FIFO的数据还未完毕，则从发送FIFO取1个数据写入发送数据寄存器 */
			//USART_SendData(_pUart->uart, _pUart->pTxBuf[_pUart->usTxRead]);
			_pUart->uart->TDR = _pUart->pTxBuf[_pUart->usTxRead];
			if (++_pUart->usTxRead >= _pUart->usTxBufSize)
			{
				_pUart->usTxRead = 0;
			}
			_pUart->usTxCount--;
		}
	}

	/* 清除中断标志 */
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_PEF);
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_FEF);
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_NEF);
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_OREF);
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_IDLEF);
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_TCF);
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_LBDF);
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_CTSF);
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_CMF);
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_WUF);
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_TXFECF);

	return;	//防止调用HAL_UART_IRQHandler();
}

#endif /* EN_BSP_UART_FIFO */

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/

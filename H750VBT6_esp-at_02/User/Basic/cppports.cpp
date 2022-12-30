/*
 * cppports.cpp
 *
 *  Created on: Dec 5, 2022
 *      @Author: OldGerman (过气德国佬)
 * 		@note: H750VBT6_usart_rx_idle_line_irq_ringbuff_tx_04 + ESP-AT SPI
 */

#include "cppports.h"
#include "main.h"

#include "lwrb.h"
#include "usart.h"
#include "spi.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>		//va_start、va_end
#include <string.h>

#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "queue.h"		    //提供消息队列
#include "semphr.h"			//提供信号量
#include "task.h"
#include "stream_buffer.h"	//提供流缓冲区API
#include "portmacro.h"		//提供portYIELD_FROM_ISR();

#include  "esp_at.h"

/* Private function prototypes */
#define WRITE_BUFFER_LEN    2048
#define READ_BUFFER_LEN     4096

uint8_t send_buffer[WRITE_BUFFER_LEN] = "";
uint8_t rcv_buffer[READ_BUFFER_LEN] = "";


/* USART related functions */
void    usart_init(void);
void    dma_Init(void);
void    usart_rx_check(void);
void    usart_process_data(const void* data, size_t len);
void    usart_send_string(const char* str);
uint8_t usart_start_tx_dma_transfer(void);
void usart_printf(const char *format, ...);
#ifndef	RAM_REGION_NO_CACHE
#define RAM_REGION_NO_CACHE	__attribute__((section(".RAM_D2_Array")))
#endif
/* Message queue ID */
static QueueHandle_t usart_rx_dma_queue_id = NULL;		/* usart rx dma 消息队列句柄 */

/* Buffer Size */
#define UART_RING_BUF_SIZE		1024			/* 缓冲区大小 */

/* 可以去掉的一些代码段, 0:去掉 1:保留 */
#define UART_TO_BE_DETERMINED	0

/* 是否在usart_start_tx_dma_transfer()函数内禁用中断 */
#define USART_TX_TRANS_DISABLE_IT   0  		/* 仅当多个操作系统线程可以访问usart_start_tx_dma_transfer()函数，且未配置独占访问保护（互斥锁），
											 或者，如果应用程序从多个中断调用此函数时，才建议在检查下一次传输之前禁用中断。*/

/* Calculate length of statically allocated array */
#define ARRAY_LEN(x)            (sizeof(x) / sizeof((x)[0]))

/* 临时缓冲区 */
RAM_REGION_NO_CACHE uint8_t usart_thread_rx_to_tx[UART_RING_BUF_SIZE];
/* USART RX buffer for DMA to transfer every received byte RX */
RAM_REGION_NO_CACHE uint8_t usart_rx_dma_buffer[UART_RING_BUF_SIZE / 2];

/* Ring buffer instance for TX data */
lwrb_t usart_rx_rb;

/* Ring buffer data array for RX DMA */
RAM_REGION_NO_CACHE uint8_t usart_rx_rb_data[UART_RING_BUF_SIZE];

/* Ring buffer instance for TX data */
lwrb_t usart_tx_rb;

/* Ring buffer data array for TX DMA */
RAM_REGION_NO_CACHE uint8_t usart_tx_rb_data[UART_RING_BUF_SIZE];

/* Length of currently active TX DMA transfer */
volatile size_t usart_tx_dma_current_len;

/* usart_printf() 使用的临时缓冲区 */
RAM_REGION_NO_CACHE uint8_t usart_printf_buffer[UART_RING_BUF_SIZE];


/******************************************** 使用到FreeRTOS的流缓冲区库 end ******************************************************/



/* 自定义printf */
void usart_printf(const char *format, ...) {
	va_list args;
	va_start(args, format);
	vsnprintf((char*) usart_printf_buffer, UART_RING_BUF_SIZE, (char*) format, args);
	va_end(args);

	usart_send_string((char*) usart_printf_buffer);
}

/* Check for new data received with DMA */
void usart_rx_check(void) {
	static size_t old_pos;
	size_t pos;

	/* Calculate current position in buffer and check for new data available */
	pos = ARRAY_LEN(usart_rx_dma_buffer) - __HAL_DMA_GET_COUNTER(&hdma_usart1_rx);
	if (pos != old_pos) {                       /* Check change in received data */
		if (pos > old_pos) {                    /* Current position is over previous one */
			/*
			 * Processing is done in "linear" mode.
			 *
			 * Application processing is fast with single data block,
			 * length is simply calculated by subtracting pointers
			 *
			 * [   0   ]
			 * [   1   ] <- old_pos |------------------------------------|
			 * [   2   ]            |                                    |
			 * [   3   ]            | Single block (len = pos - old_pos) |
			 * [   4   ]            |                                    |
			 * [   5   ]            |------------------------------------|
			 * [   6   ] <- pos
			 * [   7   ]
			 * [ N - 1 ]
			 */
			usart_process_data(&usart_rx_dma_buffer[old_pos], pos - old_pos);
		} else {
			/*
			 * Processing is done in "overflow" mode..
			 *
			 * Application must process data twice,
			 * since there are 2 linear memory blocks to handle
			 *
			 * [   0   ]            |---------------------------------|
			 * [   1   ]            | Second block (len = pos)        |
			 * [   2   ]            |---------------------------------|
			 * [   3   ] <- pos
			 * [   4   ] <- old_pos |---------------------------------|
			 * [   5   ]            |                                 |
			 * [   6   ]            | First block (len = N - old_pos) |
			 * [   7   ]            |                                 |
			 * [ N - 1 ]            |---------------------------------|
			 */
			usart_process_data(&usart_rx_dma_buffer[old_pos], ARRAY_LEN(usart_rx_dma_buffer) - old_pos);
			if (pos > 0) {
				usart_process_data(&usart_rx_dma_buffer[0], pos);
			}
		}
		old_pos = pos;                          /* Save current position as old for next transfers */
	}
}

/* Check if DMA is active and if not try to send data */
uint8_t usart_start_tx_dma_transfer(void) {
	uint8_t started = 0;
#if USART_TX_TRANS_DISABLE_IT
	uint32_t primask;
	primask = __get_PRIMASK();
	__disable_irq();
#endif
	if (usart_tx_dma_current_len == 0
			&& (usart_tx_dma_current_len = lwrb_get_linear_block_read_length(&usart_tx_rb)) > 0) {

		/* Prepare DMA data, length, and start transfer */
		HAL_UART_Transmit_DMA(&huart1, (uint8_t *)lwrb_get_linear_block_read_address(&usart_tx_rb), usart_tx_dma_current_len);

		started = 1;
	}
#if USART_TX_TRANS_DISABLE_IT
	__set_PRIMASK(primask);
#endif
	return started;
}

/**
 * @brief           Process received data over UART
 * Data are written to RX ringbuffer for application processing at latter stage
 * @param[in]       data: Data to process
 * @param[in]       len: Length in units of bytes
 */
void usart_process_data(const void* data, size_t len) {
	lwrb_write(&usart_rx_rb, data, len);  /* Write data to receive buffer */
}

/**
 * @brief           Send string over USART
 * @param[in]       str: String to send
 */
void usart_send_string(const char* str) {
	lwrb_write(&usart_tx_rb, str, strlen(str));   /* Write data to transmit buffer */
	usart_start_tx_dma_transfer();
}

/**
 * @brief           USART1 Initialization Function
 */
void usart_init(void) {
	// HAL库默认开启以上中断
	HAL_UARTEx_ReceiveToIdle_DMA(&huart1, usart_rx_dma_buffer, ARRAY_LEN(usart_rx_dma_buffer));
}

// uint8_t dtmp[1024] RAM_REGION_NO_CACHE;	//创建1024bytes临时缓冲区， usart_thread_rx_to_tx[1024] 代替 dtmp[1024]

/**
 * 高优先级后台任务，阻塞式
 */
// 从 uart 读取数据，然后将数据发送到 spi_master_tx_ring_buf。
// 该函数仅用于使用 uart 端口发送命令。
// read data from uart, and then send data to spi_master_tx_ring_buf.
// this is just used to use uart port to send command.

void uart_thread(){
	void* d;

	/* usart_init()调用一次HAL_UARTEx_ReceiveToIdle_DMA() */
	usart_init();

	/* 创建10个 void* 型消息队列，在STM32上，指针为32bit */
	usart_rx_dma_queue_id = xQueueCreate(10, 	/* UART 消息队列消息个数/深度，设为1不影响 */
			sizeof(void *));					/* 每个消息大小，单位字节 */

	/* Initialize ringbuff for TX & RX */
	lwrb_init(&usart_tx_rb, usart_tx_rb_data, sizeof(usart_tx_rb_data));
	lwrb_init(&usart_rx_rb, usart_rx_rb_data, sizeof(usart_rx_rb_data));

//	usart_send_string("USART DMA example: DMA HT & TC + USART IDLE LINE interrupts + FreeRTOS + Message queue\r\n");
//	usart_send_string("Start sending data to STM32\r\n");

	BaseType_t xResult;

	/* Infinite loop */
	while (1) {
		/* Block thread and wait for event to process USART data */
		xResult = xQueueReceive(usart_rx_dma_queue_id, &d, portMAX_DELAY);

		if (xResult == pdPASS) {
			/* Simply call processing function */
			usart_rx_check();
			(void)d;


			/**
			 *  查询RX环形缓冲区最大待读取读数据大小
			 *  Get number of bytes currently available in buffer
			 */
			uint16_t nBytes = lwrb_get_full(&usart_rx_rb);
			if(nBytes){
	    		memset(usart_thread_rx_to_tx, 0x0, UART_RING_BUF_SIZE);


	    		/**
				 * 从RX环形缓冲区读取nBytes(最大待读取读数据大小) ，存到临时缓冲区：usart_thread_rx_to_tx，
				 * 再将 usart_thread_rx_to_tx 的数据写入TX环形缓冲区
				 */
				if (lwrb_read(&usart_rx_rb, &usart_thread_rx_to_tx, nBytes) == nBytes) {

					if(strstr((char*)usart_thread_rx_to_tx, "AT+TESTSEND") != NULL) {
						usart_printf("Start test send data\r\n");
						memset(send_buffer, 0x33, WRITE_BUFFER_LEN);
						uint32_t start, finish;
						start = xTaskGetTickCount();
						for(int i=0;i< 5000;i++) {
							// send data to spi task
							write_data_to_spi_task_tx_ring_buf(send_buffer, WRITE_BUFFER_LEN);
							notify_slave_to_recv();
						}
						finish = xTaskGetTickCount();
						usart_printf("Send done, send count: %d, time: %ld ms\r\n", WRITE_BUFFER_LEN * 5000, (finish - start));
						continue;
					}

//					/* Write data to transmit buffer */
//					lwrb_write(&usart_tx_rb, &usart_thread_rx_to_tx, nBytes);
					/**
					 * 拷贝UART缓冲区数据到1024bytes的usart_thread_rx_to_tx，使用流缓冲API发给SPI task
					 */
					// send data to spi task
					write_data_to_spi_task_tx_ring_buf(usart_thread_rx_to_tx, nBytes);	//使用了 xStreamBufferSend() 进行任务间通信
					notify_slave_to_recv();

//					/* DMA 将接收到的数据发回 */
//					usart_start_tx_dma_transfer();
				}
			}
		}
	}
//    free(dtmp);
//    dtmp = NULL;
    vTaskDelete(NULL);
}

/**
 * 低优先级的时间片调度任务
 */
void led_thread(){
	TickType_t xLastWakeTime = xTaskGetTickCount();
	const TickType_t xFrequency = 250;

	/* 获取当前的系统时间 */
	xLastWakeTime = xTaskGetTickCount();
	for(;;){
		/* 每100ms翻转开发板红色LED */
		HAL_GPIO_TogglePin(LRGB_R_GPIO_Port, LRGB_R_Pin);

		/* 打印时间节拍 */
//		usart_printf("[led_task] sysTick : %ld ms\r\n", xTaskGetTickCount());

		/* vTaskDelayUntil 是绝对延迟，vTaskDelay 是相对延迟。*/
		vTaskDelayUntil(&xLastWakeTime, xFrequency);
	}
}


/* Interrupt handlers begin */
/**
 * @brief Tx Transfer completed callback.
 * @param huart UART handle.
 * @retval None
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
	if(huart->Instance == USART1) {
		lwrb_skip(&usart_tx_rb, usart_tx_dma_current_len);/* Skip sent data, mark as read */
		usart_tx_dma_current_len = 0;           /* Clear length variable */
		usart_start_tx_dma_transfer();          /* Start sending more data */
	}
}

/**
 * @brief  Reception Event Callback (Rx event notification called after use of advanced reception service).
 * 		串口接收事件回调函数
 * @param  huart UART handle
 * @param  Size  Number of data available in application reception buffer (indicates a position in
 *               reception buffer until which, data are available)
 * @retval None
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	UNUSED(Size);
	if(huart->Instance == USART1) {
		void* d = (void *)1;
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		/**
		 * HAL_UART_RECEPTION_TOIDLE 说明是DMA接收完成，或者半传输完成
		 * MaJerle 的实现需要用到DMA半传输中断和传输完成中断
		 * 虽然这个回调函数没有写DMA，但调用顺序是:
		 * HAL_UART_IRQHandler() --> UART_DMARxHalfCplt() --> HAL_UARTEx_RxEventCallback()
		 * HAL_UART_IRQHandler() --> UART_DMAReceiveCplt() --> HAL_UARTEx_RxEventCallback()
		 */
		if ((huart->ReceptionType == HAL_UART_RECEPTION_TOIDLE) ||
				/* HAL_UART_RECEPTION_STANDARD 说明是空闲中断触发的接收事件 */
				(huart->ReceptionType == HAL_UART_RECEPTION_STANDARD))
		{
			/* Write data to queue. Do not use wait function! */
			xQueueSendToBackFromISR(usart_rx_dma_queue_id, &d, &xHigherPriorityTaskWoken);
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}
	}
}

/* Interrupt handlers end */

void setup(){
	esp_at_init();
}

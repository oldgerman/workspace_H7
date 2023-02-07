/*
 * frtos_spi_esp_at.h
 *
 *  Created on: 2022年12月26日
 *      @Author: OldGerman (过气德国佬)
 */

#ifndef INC_FRTOS_SPI_ESP_AT_H
#define INC_FRTOS_SPI_ESP_AT_H

#ifdef __cplusplus
#include "frtos_spi.h"
using namespace ns_frtos_spi;
extern "C" {
#endif

/* Private includes -------------------------------------------------*/
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

/* Exported types ---------------------------------------------------*/

/* Exported constants -----------------------------------------------*/

/* Exported macro ---------------------------------------------------*/
#define ESP_LOGE( tag, format, ... ) do { \
		printf("ESP (%lu) %s: " format "\r\n", xTaskGetTickCount(), tag, ##__VA_ARGS__);  \
} while(0)

/* Exported functions prototypes ------------------------------------*/

/* Private defines --------------------------------------------------*/
#ifdef __cplusplus
namespace ns_frtos_spi_esp_at
{

class FRTOS_SPIDev_ESP_AT {
public:
	/**
	 * 任务及相关组件（互斥量、消息队列、流缓冲区）使用的内存类型
	 */
	typedef enum {
		TASK_DYNAMIC = 0,	//动态
		TASK_STATIC,		//静态
	} task_mem_type_t;

	// Sets up internal state and registers the thread
	static void init(
			FRTOS_SPICmd *frtos_spi_cmd,
			GPIO_TypeDef *RESET_GPIOx,
			uint16_t RESET_GPIO_Pin,
			osPriority_t os_priority,
			FRTOS_SPIDev_ESP_AT::task_mem_type_t mem_type);

	/** @defgroup 需要由用户在外部调用的API，用于与本类的线程交互信息
	  * @{
	  *
	  */
	/* 复位esp从机 */
	static void reset_esp_at_slave();
	/*
	 * 需要在handshake引脚的上升沿中断回调函数中调用
	This isr handler is called when the handshake line goes high.
	There are two ways to trigger the GPIO interrupt:
		1. Master sends data, slave has received successfully
		2. Slave has data want to transmit
	*/
	static void gpio_handshake_isr_handler();

	// write data to spi tx_ring_buf, this is just for test
	static int32_t write_data_to_spi_task_tx_ring_buf(const void* data, size_t size);
	// notify slave to recv data
	static void notify_slave_to_recv(void);
	/**
	  * @}
	  *
	  */

	/**
	 * slave 的可读/可写状态，0x1 代表可读， 0x2 代表可写
	 */
	typedef enum {
		SPI_NULL = 0,
		SPI_READ,         		// slave -> master
		SPI_WRITE,              // maste -> slave
	} spi_mode_t;

	/**
	 * spi_msg：可读/可写状态
	 */
	typedef struct {
		spi_mode_t direct;
	} spi_msg_t;
	/**
	 * spi_msg：slave通知标志
	 */
	typedef struct {
		bool slave_notify_flag; // 当slave接收完成 或 slave通知master接收时，为真
		// when slave recv done or slave notify master to recv, it will be true
	} spi_master_msg_t;
	/**
	 * 4字节通信报文的DATA段 —— data_info
	 * Master 向 slave 发送请求传输指定大小数据的通信报文其中 4 字节的 data_info
	 * Master 向 slave 发送请求传输数据的请求，然后等待 slave 向握手线发出的允许发送数据的信号
	 */
	typedef struct {
		uint32_t     magic    : 8;      // bit[24:31]  Magic 值，长度 24～31 bit，固定为 0xFE
		uint32_t     send_seq : 8;		// bit[16:23]  Master 向 slave 发送的数据包的序列号，该序列号在 master 每次发送时递增，
										//				长度 16～23 bit （相当于帧计数器）
		uint32_t     send_len : 16;		// bit[0:15]   Master 向 slave 发送的数据的字节数，长度 0～15 bit
	} spi_send_opt_t;
	/**
	 * 4字节通信报文的DATA段 —— slave_status
	 * Master 检测到握手线上有 slave 发出的信号后，需要发送一条消息查询 slave 的工作模式是处于接收数据，还是处于发送数据
	 * 发送查询请求后，slave 返回的状态信息将存储在 4 字节的 slave_status
	 */
	typedef struct {
		uint32_t     direct 	: 8;	// bit[24:31]	slave 的可读/可写状态，长度 24～31 bit， 其中，0x1 代表可读， 0x2 代表可写
		uint32_t     seq_num 	: 8;	// bit[16:23]	数据包序列号，长度 16～23 bit，
										//				当序列号达到最大值 0xFF 时，下一个数据包的序列号需要重新设置为 0x0 。
										//				当 slave 处于可写状态时，该字段为 master 需向 slave 发送的下一个数据包的序列号；
										//				当 slave 处于可读状态时，该字段为 slave 向 master 发送的下一个数据包的序列号。
		uint32_t     transmit_len : 16;	// bit[0:15]	slave 需要向 master 发送的数据的字节数，长度 0～15 bit；
										//				仅当 slave 处于可读状态时，该字段数字有效
	} spi_recv_opt_t;

	static const size_t     	TaskStackSize = 8 * 256;
	static osThreadId_t       	TaskHandle;
	static StaticTask_t 		TaskControlBlock;

private:
	/* 初始化Master端硬件 */
	static void init_master_hd();
	/* SPI互斥锁API */
	static void spi_mutex_lock(void);
	static void spi_mutex_unlock(void);
	/* SPI 收发API */
	static void spi_transfer(spi_transaction_t * pTransaction_t);
	static void at_spi_master_send_data(uint8_t* data, uint16_t len);
	static void at_spi_master_recv_data(uint8_t* data, uint16_t len);
	static void at_spi_rddma_done(void);
	static void at_spi_wrdma_done(void);
	static void query_slave_data_trans_info();
	static void spi_master_request_to_write(uint8_t send_seq, uint16_t send_len);
	static int8_t spi_write_data(uint8_t* buf, int32_t len);

	/*
	 * SPI AT Master端通信报文格式
	 * | ------------- | -------------- | --------------- | ----------------------------- |
	 * | CMD（1 字节） | ADDR（1 字节） | DUMMY（1 字节） | DATA（读/写，高达 4092 字节） |
	 * | ------------- | -------------- | --------------- | ----------------------------- |
	 * 以下静态成员常量用于 SPI AT Master端的6种通信报文
	 */
	static const uint8_t 	CMD_HD_WRBUF_REG    = 0x01;		// CMD段：5) Master 向 slave 发送请求传输指定大小数据
	static const uint8_t 	CMD_HD_RDBUF_REG    = 0x02;		// CMD段：6) Master 检测到握手线上有 slave 发出的信号后，需要发送一条消息查询 slave 进入接收数据的工作模式，还是进入到发送数据的工作模式
	static const uint8_t 	CMD_HD_WRDMA_REG    = 0x03;		// CMD段：1) Master 向 slave 发送数据
	static const uint8_t 	CMD_HD_RDDMA_REG    = 0x04;		// CMD段：3) Master 接收 slave 发送的数据
	static const uint8_t 	CMD_HD_WR_END_REG   = 0x07;		// CMD段：2) Master 向 slave 发送数据结束后，需要发送一条通知消息来结束本次传输
	static const uint8_t 	CMD_HD_INT0_REG     = 0x08;		// CMD段：4) Master 接收 slave 发送的数据后，需要发送一条通知消息来结束本次传输
	static const uint8_t 	WRBUF_START_ADDR    = 0x0;		// ARR段：1~5) 通信报文
	static const uint8_t 	RDBUF_START_ADDR    = 0x4;		// ARR段：6) 通信报文
	static const uint16_t 	ESP_SPI_DMA_MAX_LEN	= 4092;		// DATA段：高达4092字节

	static const uint16_t 	STREAM_BUFFER_SIZE  = 1024 * 8;	// 发送环形流缓冲区：8192 bytes

	/* 线程 */
	static void                thread(void *arg);
	static uint64_t            TaskBuffer[TaskStackSize / 8];
	static osPriority_t 		Priority;

	static uint8_t 				initiative_send_flag; 	// it means master has data to send to slave
	static uint32_t 			plan_send_len; 			// master plan to send data len
	static uint8_t 				current_send_seq;		/* TX数据包序列号 */
	static uint8_t 				current_recv_seq;		/* RX数据包序列号 */

	static FRTOS_SPICmd 		*ptr_frtos_spi_cmd;
	static uint8_t  			*pTxData;
	static uint8_t  			*pRxData;
	static uint8_t  			txDataBuffer[ESP_SPI_DMA_MAX_LEN];
	static uint8_t  			rxDataBuffer[ESP_SPI_DMA_MAX_LEN];
	static spi_recv_opt_t 		recv_opt;
	static uint32_t 			tx_buffer_dummy;
	static spi_send_opt_t 		send_opt;
	static uint32_t 			rx_buffer_dummy;

	static GPIO_TypeDef 		*_RESET_GPIOx;
	static uint16_t			 	_RESET_GPIO_Pin;

	static constexpr const char* TAG = "SPI AT Master"; // 打印日志的固定字符串

	static QueueHandle_t 		msg_queue; 											// 队列句柄：用于表示通信开始 读/写
	static const uint8_t 		msg_queue_item_size = sizeof(spi_master_msg_t);		// 队列每个消息大小
	static const uint8_t 		msg_queue_length = 5;								// 队列深度
	static uint8_t 				msg_queue_storage[msg_queue_length * msg_queue_item_size];
	static StaticQueue_t 		msg_quene_struct;	/* The variable used to hold the queue's data structure. */

	static SemaphoreHandle_t 	pxMutex;			// SPI信号量：互斥信号量
	static StaticSemaphore_t 	pxMutexBuffer;

	static StreamBufferHandle_t 	spi_master_tx_stream_buffer;			// 流缓冲区：环形的
	static const uint8_t 		 	spi_master_tx_stream_buffer_trigger_level = 1;
	static StaticStreamBuffer_t 	spi_master_tx_stream_buffer_struct;		/* The variable used to hold the stream buffer structure. */
	static uint8_t 					spi_master_tx_stream_buffer_storage[STREAM_BUFFER_SIZE];

	static volatile bool esp_initialized;
};


} /* ns_frtos_spi_esp_at */

using namespace ns_frtos_spi_esp_at;


}		/* extern "C" */
#endif	/* __cplusplus */
#endif /* INC_FRTOS_SPI_ESP_AT_H */

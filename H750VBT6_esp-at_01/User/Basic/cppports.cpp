/*
 * cppports.cpp
 *
 *  Created on: Dec 5, 2022
 *  @Modified: OldGerman modified MaJerle's github repository example:
 *  	https://github.com/MaJerle/stm32-usart-uart-dma-rx-tx/blob/main/projects/usart_rx_idle_line_irq_ringbuff_tx_H7/Src/main.c
 *  	https://github.com/MaJerle/stm32-usart-uart-dma-rx-tx/blob/main/projects/usart_rx_idle_line_irq_rtos_F4/Src/main.c
 *  @Notice: Fully use the HAL library API without modifying the IRQHandler function in stm32h7xx_it.c
 *  		完全使用HAL库API，无需修改stm32h7xx_it.c中的IRQHandler函数
 *	@brief：USART、DMA、空闲线路中断、不定长数据收发、环形缓冲区、FreeRTOS消息队列阻塞式驱动
 *			将USART1接收到的不定长数据发回，支持一次发多条数据逐一发回，收发的数据可以大于缓冲区大小
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
//#include <stddef.h>

#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "queue.h"		    //提供消息队列
#include "semphr.h"			//提供信号量
#include "task.h"
#include "stream_buffer.h"	//提供流缓冲区API
#include "portmacro.h"		//提供portYIELD_FROM_ISR();

#include "frtos_spi.h"
using namespace namespace_frtos_spi_t;

/* Private function prototypes */

/* USART related functions */
void    usart_init(void);
void    dma_Init(void);
void    usart_rx_check(void);
void    usart_process_data(const void* data, size_t len);
void    usart_send_string(const char* str);
uint8_t usart_start_tx_dma_transfer(void);
void usart_printf(const char *format, ...);

#define  RAM_REGION_NO_CACHE	__attribute__((section(".RAM_D2_Array")))
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
uint8_t usart_thread_rx_to_tx[UART_RING_BUF_SIZE] RAM_REGION_NO_CACHE;
/* USART RX buffer for DMA to transfer every received byte RX */
uint8_t usart_rx_dma_buffer[UART_RING_BUF_SIZE / 2] RAM_REGION_NO_CACHE;

/* Ring buffer instance for TX data */
lwrb_t usart_rx_rb;

/* Ring buffer data array for RX DMA */
uint8_t usart_rx_rb_data[UART_RING_BUF_SIZE] RAM_REGION_NO_CACHE;

/* Ring buffer instance for TX data */
lwrb_t usart_tx_rb;

/* Ring buffer data array for TX DMA */
uint8_t usart_tx_rb_data[UART_RING_BUF_SIZE] RAM_REGION_NO_CACHE;

/* Length of currently active TX DMA transfer */
volatile size_t usart_tx_dma_current_len;

/* usart_printf() 使用的临时缓冲区 */
uint8_t usart_printf_buffer[UART_RING_BUF_SIZE] RAM_REGION_NO_CACHE;


#define IRAM_ATTR			//H750不用这个宏，改用 GNU LINK?

/**
 * @brief Log level
 *
 */
typedef enum {
    ESP_LOG_NONE,       /*!< No log output */
    ESP_LOG_ERROR,      /*!< Critical errors, software module can not recover on its own */
    ESP_LOG_WARN,       /*!< Error conditions from which recovery measures have been taken */
    ESP_LOG_INFO,       /*!< Information messages which describe normal flow of events */
    ESP_LOG_DEBUG,      /*!< Extra information which is not necessary for normal use (values, pointers, sizes, etc). */
    ESP_LOG_VERBOSE     /*!< Bigger chunks of debugging information, or frequent messages which can potentially flood the output. */
} esp_log_level_t;


#define LOG_COLOR_E
#define LOG_COLOR_W
#define LOG_COLOR_I
#define LOG_COLOR_D
#define LOG_COLOR_V
#define LOG_RESET_COLOR

uint32_t esp_log_timestamp(void){
	return xTaskGetTickCount();
}

#define LOG_FORMAT(letter, format)  LOG_COLOR_ ## letter #letter " (%u) %s: " format LOG_RESET_COLOR "\r\n"

#define ESP_LOG_LEVEL(level, tag, format, ...) do {                     \
        if (level==ESP_LOG_ERROR ) { usart_printf(LOG_FORMAT(E, format), esp_log_timestamp(), tag, ##__VA_ARGS__); } \
    } while(0)

#define ESP_LOG_LEVEL_LOCAL(level, tag, format, ...) do {               \
        ESP_LOG_LEVEL(level, tag, format, ##__VA_ARGS__); \
    } while(0)

#define ESP_LOGE( tag, format, ... ) ESP_LOG_LEVEL_LOCAL(ESP_LOG_ERROR, tag, format, ##__VA_ARGS__)

/*
Pins in use. The SPI Master can use the GPIO mux, so feel free to change these if needed.
*/

#define GPIO_HANDSHAKE        CONFIG_SPI_HANDSHAKE_PIN
#define GPIO_MOSI             CONFIG_SPI_MOSI_PIN
#define GPIO_MISO             CONFIG_SPI_MISO_PIN
#define GPIO_SCLK             CONFIG_SPI_SCLK_PIN
#define GPIO_CS               CONFIG_SPI_CS_PIN

#ifdef CONFIG_SPI_QUAD_MODE
#define GPIO_WP               CONFIG_SPI_WP_PIN
#define GPIO_HD               CONFIG_SPI_HD_PIN
#endif

#ifdef CONFIG_IDF_TARGET_ESP32
#define MASTER_HOST           HSPI_HOST
#elif defined CONFIG_IDF_TARGET_ESP32C3
#define MASTER_HOST           SPI2_HOST
#endif

/*
 * SPI AT Master端通信报文格式
 * | ------------- | -------------- | --------------- | ----------------------------- |
 * | CMD（1 字节） | ADDR（1 字节） | DUMMY（1 字节） | DATA（读/写，高达 4092 字节） |
 * | ------------- | -------------- | --------------- | ----------------------------- |
 * 以下宏常量用于 SPI AT Master端的6种通信报文
 */
#define ESP_SPI_DMA_MAX_LEN   4092					// DATA段：高达4092字节
#define CMD_HD_WRBUF_REG      0x01					// CMD段：5) Master 向 slave 发送请求传输指定大小数据
#define CMD_HD_RDBUF_REG      0x02					// CMD段：6) Master 检测到握手线上有 slave 发出的信号后，需要发送一条消息查询 slave 进入接收数据的工作模式，还是进入到发送数据的工作模式
#define CMD_HD_WRDMA_REG      0x03					// CMD段：1) Master 向 slave 发送数据
#define CMD_HD_RDDMA_REG      0x04					// CMD段：3) Master 接收 slave 发送的数据
#define CMD_HD_WR_END_REG     0x07					// CMD段：2) Master 向 slave 发送数据结束后，需要发送一条通知消息来结束本次传输
#define CMD_HD_INT0_REG       0x08					// CMD段：4) Master 接收 slave 发送的数据后，需要发送一条通知消息来结束本次传输
#define WRBUF_START_ADDR      0x0					// ARR段：1~5) 通信报文
#define RDBUF_START_ADDR      0x4					// ARR段：6) 通信报文

#define STREAM_BUFFER_SIZE    1024 * 8				// 流缓冲区：8192 bytes
#define DMA_CHAN              SPI_DMA_CH_AUTO		// init_master_hd() 中的 spi_bus_initialize() 自动选择DMA通道


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
    uint32_t     direct : 8;		// bit[24:31]	slave 的可读/可写状态，长度 24～31 bit， 其中，0x1 代表可读， 0x2 代表可写
    uint32_t     seq_num : 8;		// bit[16:23]	数据包序列号，长度 16～23 bit，
    								//				当序列号达到最大值 0xFF 时，下一个数据包的序列号需要重新设置为 0x0 。
    								//				当 slave 处于可写状态时，该字段为 master 需向 slave 发送的下一个数据包的序列号；
    								//				当 slave 处于可读状态时，该字段为 slave 向 master 发送的下一个数据包的序列号。
    uint32_t     transmit_len : 16;	// bit[0:15]	slave 需要向 master 发送的数据的字节数，长度 0～15 bit；
    								//				仅当 slave 处于可读状态时，该字段数字有效
} spi_recv_opt_t;



static xQueueHandle msg_queue; 								// 消息队列句柄：用于表示通信开始 读/写
															// meaasge queue used for communicating read/write start
//static spi_device_handle_t handle;						// SPI句柄

static StreamBufferHandle_t spi_master_tx_ring_buf = NULL;	// 流缓冲区：环形的，用于SPI Master TX

static const char* TAG = "SPI AT Master";					// 打印日志的固定字符串
static xSemaphoreHandle pxMutex;							// SPI信号量，实现互斥锁
static uint8_t initiative_send_flag = 0; 					// 标记Master有数据要发给Slave
															// it means master has data to send to slave
static uint32_t plan_send_len = 0; // master plan to send data len

static uint8_t current_send_seq = 0;
static uint8_t current_recv_seq = 0;



/**
 *	@brief: SPI互斥锁上锁
 */
static void spi_mutex_lock(void)
{
    while (xSemaphoreTake(pxMutex, portMAX_DELAY) != pdPASS);	//获取信号量,最大等待时间0xffffffff
}

/**
 *	@brief: SPI互斥锁解锁
 */
static void spi_mutex_unlock(void)
{
    xSemaphoreGive(pxMutex);									//释放信号量
}

/**
 * 当握手线变高时调用此 isr 处理程序。
 * 有两种方式可以触发 GPIO 中断：
 * 1、master发送数据，slave已经接收成功
 * 2、slave有数据要传给master
 * ————————————————————————————————
 * This isr handler is called when the handshake line goes high.
 * There are two ways to trigger the GPIO interrupt:
 * 1. Master sends data, slave has received successfully
 * 2. Slave has data want to transmit
*/
static void IRAM_ATTR gpio_handshake_isr_handler()
{
    //释放信号量 Give the semaphore.
    BaseType_t mustYield = pdFALSE;
    spi_master_msg_t spi_msg = {
        .slave_notify_flag = true,	// 指定结构体成员初始化
    };

    /**
     * 通知master进行下一次传输 notify master to do next transaction
     *
     * 《ALIENTEK STM32F103 全系列开发板 FreeRTOS开发教程》 P211
     * 		xQueueSendFromISR(): 从ISR中给队列发送数据...中断级入队函数...发送消息到队列尾（后向入队）
     */
    xQueueSendFromISR(
    		msg_queue, 			// 用于通信读/写开始的消息队列

    		(void*)&spi_msg,	// 指向要发送的消息，发送的时候会将这个消息拷贝到队列中

			&mustYield			// 标记退出此函数以后是否进行任务切换，这个变量的值由这这个函数来设置的，
    							// 用户不用进行设置，用户只需要提供一个变量来保存这个值就行了。
    							// 当此值为 pdTRUE 的时候在退出中断服务函数之前一定要进行一次任务切换    <----原来如此
    );

    /* 如果 mustYield = pdTRUE，那么退出中断后切到当前最高优先级任务执行 */
    portYIELD_FROM_ISR( mustYield );
}

const uint16_t SPI2_sizeBuf = 4 * 1024;
uint8_t SPI2_RxBuf[SPI2_sizeBuf] RAM_REGION_NO_CACHE;
uint8_t SPI2_TxBuf[SPI2_sizeBuf] RAM_REGION_NO_CACHE;
FRTOS_SPIBase SPI2_Base(hspi2, SPI2_RxBuf, SPI2_TxBuf, SPI2_sizeBuf);
FRTOS_SPIDev_ESP32_C3_AT frtos_spi_esp(&SPI2_Base,
		USR_SPI_CS_GPIO_Port,
		USR_SPI_CS_Pin,
//		SPI_BAUDRATEPRESCALER_64,	//2.5M/s
		SPI_BAUDRATEPRESCALER_16,	//10M/s
		SPI_PHASE_1EDGE,
		SPI_POLARITY_LOW);
/**
  * @brief  TxRx Transfer completed callback.
  * @param  hspi: SPI handle
  * @note   This example shows a simple way to report end of DMA TxRx transfer, and
  *         you can add your own implementation.
  * @retval None
  */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
	SPI2_Base.wTransferState = TRANSFER_STATE_COMPLETE;
}

/**
  * @brief  SPI error callbacks.
  * @param  hspi: SPI handle
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
	SPI2_Base.wTransferState = TRANSFER_STATE_ERROR;
}

/**
 * 1) Master 向 slave 发送数据
 */
static void at_spi_master_send_data(uint8_t* data, uint16_t len)
{
	spi_transaction_t trans = {
			.instr = CMD_HD_WRDMA_REG,
			.tx_buffer = data,
			.tx_size = len,
			.instr_bytes = 1,							/* 命令字节数 */
			.addr_bytes = 1,							/* 地址字节数 */
			.dummy_bytes = 1,							/* 等待字节数 */
			.cmd_transfer_mode = TRANSFER_MODE_POLL,	/* 指令阶段轮询传输 */
			.data_transfer_mode = TRANSFER_MODE_POLL,	//TRANSFER_MODE_DMA,	/* 数据阶段DMA传输 */
	};

	frtos_spi_esp.spi_device_polling_transmit(&trans);
}

/**
 * AT SPI Master 发送 接收数据
 */
static void at_spi_master_recv_data(uint8_t* data, uint16_t len)
{
	spi_transaction_t trans = {
	        .instr = CMD_HD_RDDMA_REG,    // master -> slave command, donnot change
			.rx_buffer = data,
			.rx_size = len,
			.instr_bytes = 1,							/* 命令字节数 */
			.addr_bytes = 1,							/* 地址字节数 */
			.dummy_bytes = 1,							/* 等待字节数 */
			.cmd_transfer_mode = TRANSFER_MODE_POLL,	/* 指令阶段轮询传输 */
			.data_transfer_mode = TRANSFER_MODE_POLL//TRANSFER_MODE_DMA,	/* 数据阶段DMA传输 */
	};
	frtos_spi_esp.spi_device_polling_transmit(&trans);
}

// send a single to slave to tell slave that master has read DMA done
static void at_spi_rddma_done(void)
{
	spi_transaction_t end_t = {
			.instr = CMD_HD_INT0_REG,
			.instr_bytes = 1,							/* 命令字节数 */
			.addr_bytes = 1,							/* 地址字节数 */
			.dummy_bytes = 1,							/* 等待字节数 */
			.cmd_transfer_mode = TRANSFER_MODE_POLL,	/* 指令阶段轮询传输 */
	};
	frtos_spi_esp.spi_device_polling_transmit(&end_t);
}

// send a single to slave to tell slave that master has write DMA done
static void at_spi_wrdma_done(void)
{
	spi_transaction_t end_t = {
			.instr = CMD_HD_WR_END_REG,
			.instr_bytes = 1,							/* 命令字节数 */
			.addr_bytes = 1,							/* 地址字节数 */
			.dummy_bytes = 1,							/* 等待字节数 */
			.cmd_transfer_mode = TRANSFER_MODE_POLL,	/* 指令阶段轮询传输 */
	};
	frtos_spi_esp.spi_device_polling_transmit(&end_t);
}

// 当 spi slave 准备好从 spi master 发送/接收数据时，spi slave 将触发 GPIO 中断，
// 然后 spi master 应该查询 slave 是否将执行读或写操作。
// when spi slave ready to send/recv data from the spi master, the spi slave will a trigger GPIO interrupt,
// then spi master should query whether the slave will perform read or write operation.
static spi_recv_opt_t query_slave_data_trans_info()
{
	spi_recv_opt_t recv_opt;
	spi_transaction_t trans = {
			.instr = CMD_HD_RDBUF_REG,
			.addr = RDBUF_START_ADDR,
			.rx_buffer = (uint8_t *)&recv_opt,
			.rx_size = 4,
			.instr_bytes = 1,							/* 命令字节数 */
			.addr_bytes = 1,							/* 地址字节数 */
			.dummy_bytes = 1,							/* 等待字节数 */
			.cmd_transfer_mode = TRANSFER_MODE_POLL,	/* 指令阶段轮询传输 */
			.data_transfer_mode = TRANSFER_MODE_POLL//TRANSFER_MODE_DMA,		/* 数据阶段DMA传输 */
	};
	frtos_spi_esp.spi_device_polling_transmit(&trans);
    return recv_opt;
}

// before spi master write to slave, the master should write WRBUF_REG register to notify slave,
// and then wait for handshark line trigger gpio interrupt to start the data transmission.
static void spi_master_request_to_write(uint8_t send_seq, uint16_t send_len)
{
    spi_send_opt_t send_opt;
    send_opt.magic = 0xFE;
    send_opt.send_seq = send_seq;
    send_opt.send_len = send_len;

    spi_transaction_t trans = {
        .instr = CMD_HD_WRBUF_REG,
        .addr = WRBUF_START_ADDR,
        .tx_buffer = (uint8_t *)&send_opt,
        .tx_size = 4,
		.instr_bytes = 1,							/* 命令字节数 */
		.addr_bytes = 1,							/* 地址字节数 */
		.dummy_bytes = 1,							/* 等待字节数 */
		.cmd_transfer_mode = TRANSFER_MODE_POLL,	/* 指令阶段轮询传输 */
		.data_transfer_mode = TRANSFER_MODE_POLL//TRANSFER_MODE_DMA,		/* 数据阶段DMA传输 */
    };
	frtos_spi_esp.spi_device_polling_transmit(&trans);
    // increment
    current_send_seq  = send_seq;
}

// spi master write data to slave
static int8_t spi_write_data(uint8_t* buf, int32_t len)
{
    if (len > ESP_SPI_DMA_MAX_LEN) {
        ESP_LOGE(TAG, "Send length errot, len:%d", len);
        return -1;
    }
    at_spi_master_send_data(buf, len);
    at_spi_wrdma_done();
    return 0;
}

/******************************************** 使用到FreeRTOS的流缓冲区库 begin ******************************************************/
// write data to spi tx_ring_buf, this is just for test
static int32_t write_data_to_spi_task_tx_ring_buf(const void* data, size_t size)
{
    int32_t length = size;

    //检查需要发送的数据的合法性
    if (data == NULL  || length > STREAM_BUFFER_SIZE) {
        ESP_LOGE(TAG, "Send length errot, len:%d", length);
        return -1;
    }

    length = xStreamBufferSend(
    		spi_master_tx_ring_buf,		// stream buffer 句柄
			data,						//指向需要发送数据的指针
			size,						//发送数据的长度
			portMAX_DELAY
	);
    return length;
}

// notify slave to recv data
static void notify_slave_to_recv(void)
{
    if (initiative_send_flag == 0) {
        spi_mutex_lock();
        uint32_t tmp_send_len = xStreamBufferBytesAvailable(spi_master_tx_ring_buf);
        if (tmp_send_len > 0) {
            plan_send_len = tmp_send_len > ESP_SPI_DMA_MAX_LEN ? ESP_SPI_DMA_MAX_LEN : tmp_send_len;
            spi_master_request_to_write(current_send_seq + 1, plan_send_len); // to tell slave that the master want to write data
            initiative_send_flag = 1;
        }
        spi_mutex_unlock();
    }
}


static void IRAM_ATTR spi_trans_control_task()
{
    int8_t ret;
    spi_master_msg_t trans_msg = {0};
    uint32_t send_len = 0;

    uint8_t* trans_data = (uint8_t*)malloc(ESP_SPI_DMA_MAX_LEN * sizeof(uint8_t));
    if (trans_data == NULL) {
        ESP_LOGE(TAG, "malloc fail");
        return;
    }

    while (1) {
        xQueueReceive(msg_queue, (void*)&trans_msg, (TickType_t)portMAX_DELAY);
        spi_mutex_lock();
        spi_recv_opt_t recv_opt = query_slave_data_trans_info();

        if (recv_opt.direct == SPI_WRITE) {
            if (plan_send_len == 0) {
                ESP_LOGE(TAG, "master want send data but length is 0");
                continue;
            }

            if (recv_opt.seq_num != current_send_seq) {
                ESP_LOGE(TAG, "SPI send seq error, %x, %x", recv_opt.seq_num, current_send_seq);
                if (recv_opt.seq_num == 1) {
                    ESP_LOGE(TAG, "Maybe SLAVE restart, ignore");
                    current_send_seq = recv_opt.seq_num;
                } else {
                    break;
                }
            }

            //initiative_send_flag = 0;
            send_len = xStreamBufferReceive(spi_master_tx_ring_buf, (void*) trans_data, plan_send_len, 0);

            if (send_len != plan_send_len) {
                ESP_LOGE(TAG, "Read len expect %d, but actual read %d", plan_send_len, send_len);
                break;
            }

            ret = spi_write_data(trans_data, plan_send_len);
            if (ret < 0) {
                ESP_LOGE(TAG, "Load data error");
                return;
            }

            // maybe streambuffer filled some data when SPI transimit, just consider it after send done, because send flag has already in SLAVE queue
            uint32_t tmp_send_len = xStreamBufferBytesAvailable(spi_master_tx_ring_buf);
            if (tmp_send_len > 0) {
                plan_send_len = tmp_send_len > ESP_SPI_DMA_MAX_LEN ? ESP_SPI_DMA_MAX_LEN : tmp_send_len;
                spi_master_request_to_write(current_send_seq + 1, plan_send_len);
            } else {
                initiative_send_flag = 0;
            }

        } else if (recv_opt.direct == SPI_READ) {
            if (recv_opt.seq_num != ((current_recv_seq + 1) & 0xFF)) {
                ESP_LOGE(TAG, "SPI recv seq error, %x, %x", recv_opt.seq_num, (current_recv_seq + 1));
                if (recv_opt.seq_num == 1) {
                    ESP_LOGE(TAG, "Maybe SLAVE restart, ignore");
                } else {
                    break;
                }
            }

            if (recv_opt.transmit_len > STREAM_BUFFER_SIZE || recv_opt.transmit_len == 0) {
                ESP_LOGE(TAG, "SPI read len error, %x", recv_opt.transmit_len);
                break;
            }

            current_recv_seq = recv_opt.seq_num;
            memset(trans_data, 0x0, recv_opt.transmit_len);
            at_spi_master_recv_data(trans_data, recv_opt.transmit_len);
            at_spi_rddma_done();
            trans_data[recv_opt.transmit_len] = '\0';
            printf("%s", trans_data);
            fflush(stdout);    //Force to print even if have not '\n'
        } else {
            ESP_LOGE(TAG, "Unknow direct: %d", recv_opt.direct);
            spi_mutex_unlock();
            continue;
        }

        spi_mutex_unlock();
    }

    free(trans_data);
    vTaskDelete(NULL);
}

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

// uint8_t dtmp[1024] RAM_REGION_NO_CACHE;	//创建1024bytes临时缓冲区， usart_thread_rx_to_tx[1024] 代替 dtmp[1024]


/**
  * @brief EXTI line detection callback, used as SPI handshake GPIO
  * @param GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if ( GPIO_Pin == GPIO_HANDSHAKE_Pin)
	{
		gpio_handshake_isr_handler();
	}
}

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
			}
			/**
			 * 从RX环形缓冲区读取nBytes(最大待读取读数据大小) ，存到临时缓冲区：usart_thread_rx_to_tx，
			 * 再将 usart_thread_rx_to_tx 的数据写入TX环形缓冲区
			 */
			if (lwrb_read(&usart_rx_rb, &usart_thread_rx_to_tx, nBytes) == nBytes) {
				/* Write data to transmit buffer */
				lwrb_write(&usart_tx_rb, &usart_thread_rx_to_tx, nBytes);
	    		/**
	    		 * 拷贝UART缓冲区数据到1024bytes的usart_thread_rx_to_tx，使用流缓冲API发给SPI task
	    		 */
	    		// send data to spi task
	    		write_data_to_spi_task_tx_ring_buf(usart_thread_rx_to_tx, nBytes);	//使用了 xStreamBufferSend() 进行任务间通信
	    		notify_slave_to_recv();

				/* DMA 将接收到的数据发回 */
				usart_start_tx_dma_transfer();
			}
		}
	}
//    free(dtmp);
//    dtmp = NULL;
    vTaskDelete(NULL);
}


static void init_master_hd()
{
    // Create the meaasge queue.
    msg_queue = xQueueCreate(5, sizeof(spi_master_msg_t));
    // Create the tx_buf.
    spi_master_tx_ring_buf = xStreamBufferCreate(STREAM_BUFFER_SIZE, 1);
    // Create the semaphore.
    pxMutex = xSemaphoreCreateMutex();

    spi_mutex_lock();

    spi_recv_opt_t recv_opt = query_slave_data_trans_info();
    ESP_LOGE(TAG, "now direct:%u", recv_opt.direct);

    if (recv_opt.direct == SPI_READ) { // if slave in waiting response status, master need to give a read done single.
        if (recv_opt.seq_num != ((current_recv_seq + 1) & 0xFF)) {
            ESP_LOGE(TAG, "SPI recv seq error, %x, %x", recv_opt.seq_num, (current_recv_seq + 1));
            if (recv_opt.seq_num == 1) {
                ESP_LOGE(TAG, "Maybe SLAVE restart, ignore");
            }
        }

        current_recv_seq = recv_opt.seq_num;

        at_spi_rddma_done();
    }
    spi_mutex_unlock();
}


void spi_thread(){
	spi_trans_control_task();
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

void setup(){
	init_master_hd();
}

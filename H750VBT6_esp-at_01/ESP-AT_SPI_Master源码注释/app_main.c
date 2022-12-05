/*
 * cppports.cpp
 *
 *  Created on: May 16, 2022
 *      Author: OldGerman
 */

#include "cppports.h"
#include "bsp.h"
#include "spi.h"
#include "binary.h"

#ifndef DBG_PRINT
#if 0  //< Change 0 to 1 to open debug macro and check program debug information
#define DBG_PRINT usb_printf
#else
	#define DBG_PRINT(...)
	#endif
#endif



static void PrintfInfo(void)
{
	printf("*************************************************************\n\r");

	/* 检测CPU ID */
	{
		uint32_t CPU_Sn0, CPU_Sn1, CPU_Sn2;

		CPU_Sn0 = *(__IO uint32_t*)(0x1FF1E800);
		CPU_Sn1 = *(__IO uint32_t*)(0x1FF1E800 + 4);
		CPU_Sn2 = *(__IO uint32_t*)(0x1FF1E800 + 8);

		printf("CPU : STM32H750VBT6, LQFP100, 主频: %ldMHz\r\n", SystemCoreClock / 1000000);
		printf("UID = %08X %08X %08X\n\r", (unsigned int)CPU_Sn2, (unsigned int)CPU_Sn1, (unsigned int)CPU_Sn0);
	}
}

static void PrintfHelp(void)
{
	printf("*************************************************************\n\r");
	printf("ESP-HOSTED-FG吞吐量测试:\r\n");
	printf("\r\n");
	printf("\r\n");
	printf("\r\n");
	printf("操作提示:\r\n");
	printf("1. \r\n");
	printf("2. r\n");
	printf("3. \r\n");
	printf("4. \r\n");
	printf("\r\n");
}

void btA_CLICKED_func(){

}

void btB_CLICKED_func(){

}

void btA_LONG_PRESSED_func(){

}

void btB_LONG_PRESSED_func(){

}



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

#if CONFIG_LOG_COLORS
#define LOG_COLOR_BLACK   "30"
#define LOG_COLOR_RED     "31"
#define LOG_COLOR_GREEN   "32"
#define LOG_COLOR_BROWN   "33"
#define LOG_COLOR_BLUE    "34"
#define LOG_COLOR_PURPLE  "35"
#define LOG_COLOR_CYAN    "36"
#define LOG_COLOR(COLOR)  "\033[0;" COLOR "m"
#define LOG_BOLD(COLOR)   "\033[1;" COLOR "m"
#define LOG_RESET_COLOR   "\033[0m"
#define LOG_COLOR_E       LOG_COLOR(LOG_COLOR_RED)
#define LOG_COLOR_W       LOG_COLOR(LOG_COLOR_BROWN)
#define LOG_COLOR_I       LOG_COLOR(LOG_COLOR_GREEN)
#define LOG_COLOR_D
#define LOG_COLOR_V
#else //CONFIG_LOG_COLORS
#define LOG_COLOR_E
#define LOG_COLOR_W
#define LOG_COLOR_I
#define LOG_COLOR_D
#define LOG_COLOR_V
#define LOG_RESET_COLOR
#endif //CONFIG_LOG_COLORS

uint32_t esp_log_timestamp(void){
	return HAL_GetTick();
}

void esp_log_write(esp_log_level_t level, const char* tag, const char* format, ...) __attribute__ ((format (printf, 3, 4)));

#define LOG_FORMAT(letter, format)  LOG_COLOR_ ## letter #letter " (%u) %s: " format LOG_RESET_COLOR "\r\n"

#define ESP_LOG_LEVEL(level, tag, format, ...) do {                     \
        if (level==ESP_LOG_ERROR ) { esp_log_write(ESP_LOG_ERROR, tag, LOG_FORMAT(E, format), esp_log_timestamp(), tag, ##__VA_ARGS__); } \
    } while(0)

#define ESP_LOG_LEVEL_LOCAL(level, tag, format, ...) do {               \
        ESP_LOG_LEVEL(level, tag, format, ##__VA_ARGS__); \
    } while(0)

#define ESP_LOGE( tag, format, ... ) ESP_LOG_LEVEL_LOCAL(ESP_LOG_ERROR, tag, format, ##__VA_ARGS__)

/**********************************************ESP-AT SPI HOST**********************************************/

/*
 *  对于ESP，如果一些应用程序的代码需要放在IRAM 中运行，可以使用 IRAM_ATTR 宏定义进行声明
 */
#define IRAM_ATTR			//H750不用这个宏，改用 GNU LINK

//#include <stdio.h>
//#include <stdint.h>
//#include <stddef.h>
//#include <string.h>
//#include <time.h>
//
//
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "stream_buffer.h"	//提供流缓冲区API
#include "portmacro.h"		//提供portYIELD_FROM_ISR();
//
//#include "esp_system.h"
//#include "esp_err.h"
//#include "esp_log.h"
//#include "esp_intr_alloc.h"
//
//#include "driver/gpio.h"
//#include "driver/uart.h"
//#include "driver/spi_master.h"

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

#define DMA_CHAN              SPI_DMA_CH_AUTO
#define ESP_SPI_DMA_MAX_LEN   4092
#define CMD_HD_WRBUF_REG      0x01
#define CMD_HD_RDBUF_REG      0x02
#define CMD_HD_WRDMA_REG      0x03
#define CMD_HD_RDDMA_REG      0x04
#define CMD_HD_WR_END_REG     0x07
#define CMD_HD_INT0_REG       0x08
#define WRBUF_START_ADDR      0x0
#define RDBUF_START_ADDR      0x4
#define STREAM_BUFFER_SIZE    1024 * 8		// 8192 bytes

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

static QueueHandle_t esp_at_uart_queue = NULL;				// 消息队列句柄：ESP UART的

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
static void IRAM_ATTR gpio_handshake_isr_handler(void* arg)
{
    //释放信号量 Give the semaphore.
    BaseType_t mustYield = false;
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

#if 1
	portYIELD_FROM_ISR(mustYield);
#else
    if (mustYield) {
        portYIELD_FROM_ISR();
    }
#endif
}

/**
 * AT SPI Master 发送 发送数据
 */
static void at_spi_master_send_data(uint8_t* data, uint32_t len)
{
	// 指定结构体成员初始化，但HAL库没有这个 spi_transaction_t 结构体，可以改为HAL_SPI_Transmit()等效处理

    spi_transaction_t trans = {
#if defined(CONFIG_SPI_QUAD_MODE)
        .flags = SPI_TRANS_MODE_QIO,
        .cmd = CMD_HD_WRDMA_REG | (0x2 << 4),    // master -> slave command, donnot change
#elif defined(CONFIG_SPI_DUAL_MODE)
        .flags = SPI_TRANS_MODE_DIO,
        .cmd = CMD_HD_WRDMA_REG | (0x1 << 4),
#else
        .cmd = CMD_HD_WRDMA_REG,    //写模式？ // master -> slave command, donnot change
#endif
        .length = len * 8,
        .tx_buffer = (void*)data
    };

    /*
     * esp_err_t spi_device_polling_transmit(spi_device_handle_t handle,	//SPI设备句柄
     * 									spi_transaction_t *trans_desc)		//要执行的传输
     * 发起一次轮询模式下的传输，等待完成后返回结果
     * 此函数和 spi_device_polling_start() + spi_device_polling_end() 共同使用等价
     *
     * 应该是和 HAL_SPI_Transmit(hspi, pData, Size, Timeout) 等价
     */
    spi_device_polling_transmit(handle, &trans);

}

/**
 * AT SPI Master 发送 接收数据
 */
static void at_spi_master_recv_data(uint8_t* data, uint32_t len)
{
    spi_transaction_t trans = {
#if defined(CONFIG_SPI_QUAD_MODE)
        .flags = SPI_TRANS_MODE_QIO,
        .cmd = CMD_HD_RDDMA_REG | (0x2 << 4),    // master -> slave command, donnot change
#elif defined(CONFIG_SPI_DUAL_MODE)
        .flags = SPI_TRANS_MODE_DIO,
        .cmd = CMD_HD_RDDMA_REG | (0x1 << 4),
#else
        .cmd = CMD_HD_RDDMA_REG,    //读模式？ // master -> slave command, donnot change
#endif
        .rxlength = len * 8,
        .rx_buffer = (void*)data
    };
    spi_device_polling_transmit(handle, &trans);
}

// send a single to slave to tell slave that master has read DMA done
static void at_spi_rddma_done(void)
{
    spi_transaction_t end_t = {
        .cmd = CMD_HD_INT0_REG,
    };
    spi_device_polling_transmit(handle, &end_t);
}

// send a single to slave to tell slave that master has write DMA done
static void at_spi_wrdma_done(void)
{
    spi_transaction_t end_t = {
        .cmd = CMD_HD_WR_END_REG,
    };
    spi_device_polling_transmit(handle, &end_t);
}

// when spi slave ready to send/recv data from the spi master, the spi slave will a trigger GPIO interrupt,
// then spi master should query whether the slave will perform read or write operation.
static spi_recv_opt_t query_slave_data_trans_info()
{
    spi_recv_opt_t recv_opt;
    spi_transaction_t trans = {
        .cmd = CMD_HD_RDBUF_REG,
        .addr = RDBUF_START_ADDR,
        .rxlength = 4 * 8,
        .rx_buffer = &recv_opt,
    };
    spi_device_polling_transmit(handle, (spi_transaction_t*)&trans);
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
        .cmd = CMD_HD_WRBUF_REG,
        .addr = WRBUF_START_ADDR,
        .length = 4 * 8,
        .tx_buffer = &send_opt,
    };
    spi_device_polling_transmit(handle, (spi_transaction_t*)&trans);
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
        ESP_LOGE(TAG, "Write data error, len:%d", length);
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


static void IRAM_ATTR spi_trans_control_task(void* arg)
{
    esp_err_t ret;
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
            ESP_LOGD(TAG, "Unknow direct: %d", recv_opt.direct);
            spi_mutex_unlock();
            continue;
        }

        spi_mutex_unlock();
    }

    free(trans_data);
    vTaskDelete(NULL);
}

/******************************************** 使用到FreeRTOS的流缓冲区库 end ******************************************************/
// armfly串口FIFO的comGetChar函数正确使用姿势
// https://www.armbbs.cn/forum.php?mod=viewthread&tid=94579&extra=page%3D1

// 从 uart 读取数据，然后将数据发送到 spi_master_tx_ring_buf。
// 该函数仅用于使用 uart 端口发送命令。
// read data from uart, and then send data to spi_master_tx_ring_buf.
// this is just used to use uart port to send command.
void uart_task(void* pvParameters)
{
	
#if 0
//https://github.com/espressif/esp-idf/blob/master/components/driver/include/driver/uart.h
/**
 * @brief UART event types used in the ring buffer
 */
typedef enum {
    UART_DATA,              /*!< UART data event*/
    UART_BREAK,             /*!< UART break event*/
    UART_BUFFER_FULL,       /*!< UART RX buffer full event*/
    UART_FIFO_OVF,          /*!< UART FIFO overflow event*/
    UART_FRAME_ERR,         /*!< UART RX frame error event*/
    UART_PARITY_ERR,        /*!< UART RX parity event*/
    UART_DATA_BREAK,        /*!< UART TX data and break event*/
    UART_PATTERN_DET,       /*!< UART pattern detected */
#if SOC_UART_SUPPORT_WAKEUP_INT
    UART_WAKEUP,            /*!< UART wakeup event */
#endif
    UART_EVENT_MAX,         /*!< UART event max index*/
} uart_event_type_t;

/**
 * @brief Event structure used in UART event queue
 */
typedef struct {
    uart_event_type_t type; /*!< UART event type */
    size_t size;            /*!< UART data size for UART_DATA event*/
    bool timeout_flag;      /*!< UART data read timeout flag for UART_DATA event (no new data received during configured RX TOUT)*/
                            /*!< If the event is caused by FIFO-full interrupt, then there will be no event with the timeout flag before the next byte coming.*/
} uart_event_t;

#endif
//    uart_event_t event;
    uint8_t* dtmp = (uint8_t*) malloc(1024);	//创建1024bytes

    for (;;) {
        //Waiting for UART event.(使用队列事件阻塞uart任务)
        if (xQueueReceive(esp_at_uart_queue, (void*) &event,
                          (TickType_t) portMAX_DELAY)) {
            switch (event.type) {
                    //Event of UART receving data
                case UART_DATA:
                    if (event.size) {	//如果size不为0
                    	/**
                    	 * 拷贝UART缓冲区数据到1024bytes的dtmp，使用流缓冲API发给SPI task
                    	 */
                        memset(dtmp, 0x0, 1024);
                        // read the data which spi master want to send
                        uart_read_bytes(0, dtmp, event.size, portMAX_DELAY);
                        // send data to spi task
                        write_data_to_spi_task_tx_ring_buf(dtmp, event.size);	//使用了 xStreamBufferSend() 进行任务间通信
                        notify_slave_to_recv();
                    }
                    break;

                case UART_PATTERN_DET:	//UART模式检测
                    break;

                    //Others
                default:
                    break;
            }
        }
    }

    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}


#if 0
// armfly串口FIFO的comGetChar函数正确使用姿势
// https://www.armbbs.cn/forum.php?mod=viewthread&tid=94579&extra=page%3D1

enum ucStatus {
	ucStatus_waitCmd = 0,
	ucStatus_setFirstHalf,
	ucStatus_setSecondHalf,
	ucStatus_readBitsChar
};
void loop(){
    uint8_t read;
    uint8_t ucStatus = ucStatus_waitCmd;  /* 状态机标志 */
    uint16_t ucCount = 0, i;
    uint8_t buf[128];
    bool setFirstHalf = 0;
	while(1) {
		static uint32_t timeOld = HAL_GetTick();
		if(waitTime(&timeOld, 10)){
			bsp_Button_Update();
			if (comGetChar(COM1, &read))
			{
				switch (ucStatus)
				{
				/* 状态0保证接收到A或B */
				case ucStatus_waitCmd:
					if(read == 'A')
					{
						printf("设置波形数据前%d位，请输入形如 0100111011110000 的数据\r\n", bitLVL_NUM * 8);
						ucStatus = ucStatus_setFirstHalf;
					}
					else if(read == 'B')
					{
						printf("设置波形数据后%d位，请输入形如 0100111011110000 的数据\r\n", bitLVL_NUM * 8);
						ucStatus = ucStatus_setSecondHalf;
					}
					break;

				case ucStatus_setFirstHalf:
					setFirstHalf = true;
					ucStatus = ucStatus_readBitsChar;
					break;

				case ucStatus_setSecondHalf:
					ucStatus = ucStatus_readBitsChar;
					setFirstHalf = false;
					break;

				case ucStatus_readBitsChar:
					buf[ucCount] = read;

					/* 接收够16个数据 */
					bool bitCharError = false;
					if(ucCount == bitLVL_NUM * 8)
					{
						/* 打印接收到的数据值 */
						printf("接收到的数据：");
						for(i = 0; i < ucCount; i++)
						{
							*(buf + i) -= '0';	//	字符转整形
							if(buf[i] > 1 || buf[i] < 0)	//检测数据合法性
								bitCharError = true;
							printf("%d ", buf[i]);
						}
						if(bitCharError){
							printf("输入数据无效，请重新输入\r\n");
							ucStatus = ucStatus_waitCmd;
						}
						else{
							uint8_t bits[bitLVL_NUM] = {0};
							for(i = 0; i < bitLVL_NUM * 8; i++){
								uint32_t j = i / 8;
								(*(bits + j)) |= buf[bitLVL_NUM * 8 - 1 - i] << (i - 8 * j);
							}
							for(i = 0; i < bitLVL_NUM; i++) {
								if(setFirstHalf){
									bitLVL_M0[i] = bits[bitLVL_NUM - 1 - i];
								}else{
									bitLVL_M1[i] = bits[bitLVL_NUM - 1 - i];
								}
							}
							printf("输入数据有效，已更改PWM波形数据\r\n");
							ucStatus = ucStatus_waitCmd;
						}
						ucStatus = 0;
						ucCount = 0;
					}
					else
					{
						ucCount++;
					}
					break;
				}
			}
		}
	}
}
#endif
inline void spi_bus_defalut_config(spi_bus_config_t* bus_cfg)
{
    bus_cfg->mosi_io_num = GPIO_MOSI;
    bus_cfg->miso_io_num = GPIO_MISO;
    bus_cfg->sclk_io_num = GPIO_SCLK;
#if defined(CONFIG_SPI_QUAD_MODE)
    bus_cfg->quadwp_io_num = GPIO_WP;
    bus_cfg->quadhd_io_num = GPIO_HD;
#else
    bus_cfg->quadwp_io_num = -1;
    bus_cfg->quadhd_io_num = -1;
#endif
    bus_cfg->max_transfer_sz = 14000;
}

inline void spi_device_default_config(spi_device_interface_config_t* dev_cfg)
{
    dev_cfg->clock_speed_hz = SPI_MASTER_FREQ_20M;
    dev_cfg->mode = 0;
    dev_cfg->spics_io_num = GPIO_CS;
    dev_cfg->cs_ena_pretrans = 8;
    dev_cfg->cs_ena_posttrans = 8;
    dev_cfg->command_bits = 8;
    dev_cfg->address_bits = 8;
    dev_cfg->dummy_bits = 8;
    dev_cfg->queue_size = 16;
    dev_cfg->flags = SPI_DEVICE_HALFDUPLEX;
    dev_cfg->input_delay_ns = 25;
}

/**
 * 初始化SPI
 * 部分代码等效 MX_SPIx_Init();
 * 部分代码：有RTOS的东西
 */
static void init_master_hd(spi_device_handle_t* spi)
{
    // GPIO config for the handshake line.
    gpio_config_t io_conf = {
        .intr_type = GPIO_PIN_INTR_POSEDGE,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = 1,
        .pin_bit_mask = (1 << GPIO_HANDSHAKE)
    };

    //Set up handshake line interrupt.
    gpio_config(&io_conf);
    gpio_install_isr_service(0);
    gpio_set_intr_type(GPIO_HANDSHAKE, GPIO_PIN_INTR_POSEDGE);
    gpio_isr_handler_add(GPIO_HANDSHAKE, gpio_handshake_isr_handler, NULL);

    // Create the meaasge queue.
    msg_queue = xQueueCreate(5, sizeof(spi_master_msg_t));					// 创建信息队列
    // Create the tx_buf.
    spi_master_tx_ring_buf = xStreamBufferCreate(	// 创建传输的流缓冲区，返回一个 xStreamBUffer句柄 给 spi_master_tx_ring_buf
    		STREAM_BUFFER_SIZE, 					// buffer大小 8192 bytes
    		1										// 触发大小(当任务在阻塞态，要发送这么多的数据才能让任务退出阻塞态)
	);
    //可以检查是否创建成功
    if(spi_master_tx_ring_buf == NULL){
    	 ESP_LOGI(TAG, "Fail to create stream buffer!");
    }

    // Create the semaphore.
    pxMutex = xSemaphoreCreateMutex();										// 创建信号量句柄

    //init bus
    spi_bus_config_t bus_cfg = {};
    spi_bus_defalut_config(&bus_cfg);
    ESP_ERROR_CHECK(spi_bus_initialize(MASTER_HOST, &bus_cfg, DMA_CHAN));

    //add device
    spi_device_interface_config_t dev_cfg = {};
    spi_device_default_config(&dev_cfg);
    ESP_ERROR_CHECK(
    		/**
    		 * 通知FreeRTOS驱动有一个SPI设备连接到了总线上
    		 * 这个API会根据spi_device_interface_config_t结构体初始化一个SPI外设并规定具体的时序
    		 */
    		spi_bus_add_device(MASTER_HOST, &dev_cfg, spi)
	);

    /************************************* 非硬件初始化的代码 Begin **********************************************/
    spi_mutex_lock();	// SPI互斥锁上锁

    spi_recv_opt_t recv_opt = query_slave_data_trans_info();
    ESP_LOGI(TAG, "now direct:%u", recv_opt.direct);

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
    spi_mutex_unlock();	// SPI互斥锁解锁
    /************************************* 非硬件初始化的代码 End **********************************************/
}











void setup(){
	bsp_Init();
	PrintfInfo();
	PrintfHelp();
//	bsp_tim6_enable_IT();	//指示系统正在运行

}

void loop(){
    init_master_hd(&handle);

//    // init UART
//    uart_driver_install(
//    		0, 					// UART 端口号
//    		2048, 				// UART RX 环形缓冲区大小
//			8192, 				// UART TX 环形缓冲区大小
//			10, 				// UART 事件队列大小/深度
//			&esp_at_uart_queue,	// UART 事件队列句柄（输出参数）
//			1					// 用于分配中断的标志
//	);

    xTaskCreate(uart_task, "uTask", 2048, NULL, 4, NULL);									//UART 任务

    xTaskCreate(spi_trans_control_task, "spi_trans_control_task", 1024 * 2, NULL, 8, NULL);	//SPI 任务
}


// armfly串口FIFO的comGetChar函数正确使用姿势
// https://www.armbbs.cn/forum.php?mod=viewthread&tid=94579&extra=page%3D1


/* Demo:

*/

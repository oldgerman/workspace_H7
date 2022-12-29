/*
 * frtos_spi_esp_at.cpp
 *
 *  Created on: 2022年12月26日
 *      @Author: OldGerman (过气德国佬)
 */

#include "frtos_spi_esp_at.h"

FRTOS_SPICmd* 			FRTOS_SPIDev_ESP_AT::ptr_frtos_spi_cmd;

osThreadId          	FRTOS_SPIDev_ESP_AT::TaskHandle = NULL;
uint32_t            	FRTOS_SPIDev_ESP_AT::TaskBuffer[FRTOS_SPIDev_ESP_AT::TaskStackSize];
osStaticThreadDef_t 	FRTOS_SPIDev_ESP_AT::TaskControlBlock;
osPriority 				FRTOS_SPIDev_ESP_AT::Priority;

QueueHandle_t 	FRTOS_SPIDev_ESP_AT::msg_queue; 											// 队列句柄：用于表示通信开始 读/写
uint8_t 		FRTOS_SPIDev_ESP_AT::msg_queue_storage[FRTOS_SPIDev_ESP_AT::msg_queue_length * FRTOS_SPIDev_ESP_AT::msg_queue_item_size];
StaticQueue_t 	FRTOS_SPIDev_ESP_AT::msg_quene_struct;							/* The variable used to hold the queue's data structure. */

SemaphoreHandle_t FRTOS_SPIDev_ESP_AT::pxMutex;			// SPI信号量：互斥信号量
StaticSemaphore_t FRTOS_SPIDev_ESP_AT::pxMutexBuffer;

StreamBufferHandle_t 		FRTOS_SPIDev_ESP_AT::spi_master_tx_stream_buffer;			// 流缓冲区：环形的
StaticStreamBuffer_t 		FRTOS_SPIDev_ESP_AT::spi_master_tx_stream_buffer_struct;	/* The variable used to hold the stream buffer structure. */
RAM_REGION_NO_CACHE uint8_t FRTOS_SPIDev_ESP_AT::spi_master_tx_stream_buffer_storage[STREAM_BUFFER_SIZE];



uint8_t FRTOS_SPIDev_ESP_AT::initiative_send_flag = 0; 	// Master有数据要发给Slave的标记
uint32_t FRTOS_SPIDev_ESP_AT::plan_send_len = 0; 		// Master计划发送数据长度
uint8_t FRTOS_SPIDev_ESP_AT::current_send_seq = 0;		// 当前发送数据包的序列号
uint8_t FRTOS_SPIDev_ESP_AT::current_recv_seq = 0;		// 当前接收数据包的序列号

GPIO_TypeDef * 						FRTOS_SPIDev_ESP_AT::_RESET_GPIOx = nullptr;
uint16_t 							FRTOS_SPIDev_ESP_AT::_RESET_GPIO_Pin = 0;

RAM_REGION_NO_CACHE uint8_t*		FRTOS_SPIDev_ESP_AT::pTxData;
RAM_REGION_NO_CACHE uint8_t*		FRTOS_SPIDev_ESP_AT::pRxData;
RAM_REGION_NO_CACHE uint8_t  		FRTOS_SPIDev_ESP_AT::txDataBuffer[ESP_SPI_DMA_MAX_LEN];
RAM_REGION_NO_CACHE uint8_t  		FRTOS_SPIDev_ESP_AT::rxDataBuffer[ESP_SPI_DMA_MAX_LEN];
RAM_REGION_NO_CACHE FRTOS_SPIDev_ESP_AT::spi_send_opt_t 	FRTOS_SPIDev_ESP_AT::send_opt;
RAM_REGION_NO_CACHE FRTOS_SPIDev_ESP_AT::spi_recv_opt_t 	FRTOS_SPIDev_ESP_AT::recv_opt;
RAM_REGION_NO_CACHE uint32_t 		FRTOS_SPIDev_ESP_AT::tx_buffer_dummy = 0;				//用作假装发送
RAM_REGION_NO_CACHE uint32_t 		FRTOS_SPIDev_ESP_AT::rx_buffer_dummy = 0;				//用作假装接收


void FRTOS_SPIDev_ESP_AT::spi_transfer(spi_transaction_t * pTransaction_t){
	ptr_frtos_spi_cmd->busTransferExtCmdAndData(pTransaction_t);
}

/**
 *	@brief: SPI互斥锁上锁
 */
void FRTOS_SPIDev_ESP_AT::spi_mutex_lock(void)
{
	while (xSemaphoreTake(pxMutex, portMAX_DELAY) != pdPASS);	//获取信号量,最大等待时间0xffffffff
}

/**
 *	@brief: SPI互斥锁解锁
 */
void FRTOS_SPIDev_ESP_AT::spi_mutex_unlock(void)
{
	xSemaphoreGive(pxMutex);									//释放信号量
}
/**
 * Master 向 slave 发送数据
 */
void FRTOS_SPIDev_ESP_AT::at_spi_master_send_data(uint8_t* data, uint16_t len)
{
	spi_transaction_t trans = {
			.instr = CMD_HD_WRDMA_REG,
			.tx_buffer = data,
			.rx_buffer = pRxData,
			.instr_bytes = 1,							/* 命令字节数 */
			.addr_bytes = 1,							/* 地址字节数 */
			.dummy_bytes = 1,							/* 等待字节数 */
			.data_bytes = len,							/* 数据字节数*/
			.cmd_transfer_mode = TRANSFER_MODE_POLL,	/* 指令轮询传输 */
			.data_transfer_mode = TRANSFER_MODE_DMA,    /* 数据量可能较大，使用DMA传输 */
	};
	spi_transfer(&trans);
}

/**
 * Master 接收 slave 数据
 */
void FRTOS_SPIDev_ESP_AT::at_spi_master_recv_data(uint8_t* data, uint16_t len)
{
	spi_transaction_t trans = {
			.instr = CMD_HD_RDDMA_REG,  				// master -> slave command, donnot change
			.tx_buffer = pRxData,					// < 注意使用 pRxData 当作 tx缓冲区
			.rx_buffer = data,							// < 注意使用 pTxData 当作 rx缓冲区
			.instr_bytes = 1,							/* 命令字节数 */
			.addr_bytes = 1,							/* 地址字节数 */
			.dummy_bytes = 1,							/* 等待字节数 */
			.data_bytes = len,							/* 数据字节数*/
			.cmd_transfer_mode = TRANSFER_MODE_POLL,	/* 指令轮询传输 */
			.data_transfer_mode = TRANSFER_MODE_DMA,	/* 数据量可能较大，使用DMA传输 */
	};
	spi_transfer(&trans);
}

// send a single to slave to tell slave that master has read DMA done
void FRTOS_SPIDev_ESP_AT::at_spi_rddma_done(void)
{
	spi_transaction_t end_t = {
			.instr = CMD_HD_INT0_REG,
			.instr_bytes = 1,							/* 命令字节数 */
			.addr_bytes = 1,							/* 地址字节数 */
			.dummy_bytes = 1,							/* 等待字节数 */
			.cmd_transfer_mode = TRANSFER_MODE_POLL,	/* 指令轮询传输 */
	};
	spi_transfer(&end_t);
}

// send a single to slave to tell slave that master has write DMA done
void FRTOS_SPIDev_ESP_AT::at_spi_wrdma_done(void)
{
	spi_transaction_t end_t = {
			.instr = CMD_HD_WR_END_REG,
			.instr_bytes = 1,							/* 命令字节数 */
			.addr_bytes = 1,							/* 地址字节数 */
			.dummy_bytes = 1,							/* 等待字节数 */
			.cmd_transfer_mode = TRANSFER_MODE_POLL,	/* 指令轮询传输 */
	};
	spi_transfer(&end_t);
}

// 当 spi slave 准备好从 spi master 发送/接收数据时，spi slave 将触发 GPIO 中断，
// 然后 spi master 应该查询 slave 是否将执行读或写操作。
// when spi slave ready to send/recv data from the spi master, the spi slave will a trigger GPIO interrupt,
// then spi master should query whether the slave will perform read or write operation.
void FRTOS_SPIDev_ESP_AT::query_slave_data_trans_info()
{
	memset(&recv_opt, 0x0, 4);
	spi_transaction_t trans = {
			.instr = CMD_HD_RDBUF_REG,
			.addr = RDBUF_START_ADDR,
			.tx_buffer = (uint8_t *)&tx_buffer_dummy,
			.rx_buffer = (uint8_t *)&recv_opt,
			.instr_bytes = 1,							/* 命令字节数 */
			.addr_bytes = 1,							/* 地址字节数 */
			.dummy_bytes = 1,							/* 等待字节数 */
			.data_bytes = 4,							/* 数据字节数*/
			.cmd_transfer_mode = TRANSFER_MODE_POLL,	/* 指令轮询传输 */
			.data_transfer_mode = TRANSFER_MODE_POLL,	/* 数据量太短，使用轮询传输 */
	};
	spi_transfer(&trans);
}

// before spi master write to slave, the master should write WRBUF_REG register to notify slave,
// and then wait for handshark line trigger gpio interrupt to start the data transmission.
void FRTOS_SPIDev_ESP_AT::spi_master_request_to_write(uint8_t send_seq, uint16_t send_len)
{
	send_opt.magic = 0xFE;
	send_opt.send_seq = send_seq;
	send_opt.send_len = send_len;

	spi_transaction_t trans = {
			.instr = CMD_HD_WRBUF_REG,
			.addr = WRBUF_START_ADDR,
			.tx_buffer = (uint8_t *)&send_opt,
			.rx_buffer = (uint8_t *)&rx_buffer_dummy,
			.instr_bytes = 1,							/* 命令字节数 */
			.addr_bytes = 1,							/* 地址字节数 */
			.dummy_bytes = 1,							/* 等待字节数 */
			.data_bytes = 4,							/* 数据字节数*/
			.cmd_transfer_mode = TRANSFER_MODE_POLL,	/* 指令轮询传输 */
			.data_transfer_mode = TRANSFER_MODE_POLL,	/* 数据量太短，使用轮询传输 */
	};
	spi_transfer(&trans);
	// increment
	current_send_seq  = send_seq;
}

// spi master write data to slave
int8_t FRTOS_SPIDev_ESP_AT::spi_write_data(uint8_t* buf, int32_t len)
{
	if (len > ESP_SPI_DMA_MAX_LEN) {
		ESP_LOGE(TAG, "Send length errot, len:%d", len);
		return -1;
	}
	at_spi_master_send_data(buf, len);
	at_spi_wrdma_done();
	return 0;
}

/* 复位esp从机 */
void FRTOS_SPIDev_ESP_AT::reset_esp_at_slave(){
	if(_RESET_GPIOx != nullptr && _RESET_GPIO_Pin != 0)
	{
		HAL_GPIO_WritePin(_RESET_GPIOx, _RESET_GPIO_Pin, GPIO_PIN_RESET);
		osDelay(200);	//保持100ms低电平
		HAL_GPIO_WritePin(_RESET_GPIOx, _RESET_GPIO_Pin, GPIO_PIN_SET);
		osDelay(5000);	//等待10秒以确保esp_at从机初始化完毕
	}
}

void IRAM_ATTR FRTOS_SPIDev_ESP_AT::thread(const void *arg)
{
//	(void)arg;

	int8_t ret;
	spi_master_msg_t trans_msg = {0}; // 存放从消息队列中读取的消息，当slave接收完成 或 slave通知master接收时，为真
	uint32_t send_len = 0;

	/* 复位芯片 */
	reset_esp_at_slave();

	/* 初始化从机为接收AT就绪 */
	spi_mutex_lock();
	query_slave_data_trans_info();
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

	if (pTxData == NULL) {
		ESP_LOGE(TAG, "pTxData illegal");
		return;
	}
	if (pRxData == NULL) {
		ESP_LOGE(TAG, "pRxData illegal");
		return;
	}
	/*pRxData 只是用作假装发送或接收，将其数据全设置为0 */
	memset(pRxData, 0x0, ESP_SPI_DMA_MAX_LEN);

	while (1) {
		xQueueReceive(msg_queue, (void*)&trans_msg, (TickType_t)portMAX_DELAY);
		spi_mutex_lock();
		query_slave_data_trans_info();

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
			send_len = xStreamBufferReceive(spi_master_tx_stream_buffer, (void*) pTxData, plan_send_len, 0);

			if (send_len != plan_send_len) {
				ESP_LOGE(TAG, "Read len expect %d, but actual read %d", plan_send_len, send_len);
				break;
			}

			ret = spi_write_data(pTxData, plan_send_len);
			if (ret < 0) {
				ESP_LOGE(TAG, "Load data error");
				return;
			}

			// maybe streambuffer filled some data when SPI transimit, just consider it after send done, because send flag has already in SLAVE queue
			uint32_t tmp_send_len = xStreamBufferBytesAvailable(spi_master_tx_stream_buffer);
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
			memset(pTxData, 0x0, recv_opt.transmit_len);
			at_spi_master_recv_data(pTxData, recv_opt.transmit_len);
			at_spi_rddma_done();
			pTxData[recv_opt.transmit_len] = '\0';
			ESP_LOGE(TAG, "%s", pTxData);
			fflush(stdout);    //Force to print even if have not '\n'
		} else {
			ESP_LOGE(TAG, "Unknow direct: %d", recv_opt.direct);
			spi_mutex_unlock();
			continue;
		}

		spi_mutex_unlock();
	}
	vTaskDelete(NULL);
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
void IRAM_ATTR FRTOS_SPIDev_ESP_AT::gpio_handshake_isr_handler()
{
	//释放信号量 Give the semaphore.
	BaseType_t mustYield = pdFALSE;
	spi_master_msg_t spi_msg = {
			.slave_notify_flag = true,
	};

	/**
	 * 通知master进行下一次传输 notify master to do next transaction
	 * xQueueSendFromISR(): 从ISR中给队列发送数据...中断级入队函数...发送消息到队列尾（后向入队）
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

// write data to spi tx_ring_buf, this is just for test
int32_t FRTOS_SPIDev_ESP_AT::write_data_to_spi_task_tx_ring_buf(const void* data, size_t size)
{
	int32_t length = size;

	//检查需要发送的数据的合法性
	if (data == NULL  || length > STREAM_BUFFER_SIZE) {
		ESP_LOGE(TAG, "Send length errot, len:%d", length);
		return -1;
	}

	length = xStreamBufferSend(
			spi_master_tx_stream_buffer,		// stream buffer 句柄
			data,						//指向需要发送数据的指针
			size,						//发送数据的长度
			portMAX_DELAY
	);
	return length;
}

// notify slave to recv data
void FRTOS_SPIDev_ESP_AT::notify_slave_to_recv(void)
{
	if (initiative_send_flag == 0) {
		spi_mutex_lock();
		uint32_t tmp_send_len = xStreamBufferBytesAvailable(spi_master_tx_stream_buffer);
		if (tmp_send_len > 0) {
			plan_send_len = tmp_send_len > ESP_SPI_DMA_MAX_LEN ? ESP_SPI_DMA_MAX_LEN : tmp_send_len;
			spi_master_request_to_write(current_send_seq + 1, plan_send_len); // to tell slave that the master want to write data
			initiative_send_flag = 1;
		}
		spi_mutex_unlock();
	}
}

// Sets up internal state and registers the thread
void FRTOS_SPIDev_ESP_AT::init(FRTOS_SPICmd *frtos_spi_cmd,
		GPIO_TypeDef *RESET_GPIOx,
		uint16_t RESET_GPIO_Pin,
		osPriority os_priority) {
	// Initialize static members
	ptr_frtos_spi_cmd = frtos_spi_cmd;
	_RESET_GPIOx = RESET_GPIOx;
	_RESET_GPIO_Pin = RESET_GPIO_Pin;
	Priority = os_priority;
	pTxData = (uint8_t *)txDataBuffer;
	pRxData = (uint8_t *)rxDataBuffer;

	// Create the meaasge queue.(消息队列深度5)
	msg_queue = xQueueCreateStatic(
			msg_queue_length,
			msg_queue_item_size,
			msg_queue_storage,
			&msg_quene_struct);

	// Create the tx_buf. (发送环形缓冲区8192字节)
	spi_master_tx_stream_buffer = xStreamBufferCreateStatic(
			sizeof(spi_master_tx_stream_buffer_storage),
			spi_master_tx_stream_buffer_trigger_level,
			spi_master_tx_stream_buffer_storage,
			&spi_master_tx_stream_buffer_struct);

	// Create the semaphore.
	pxMutex = xSemaphoreCreateMutexStatic(&pxMutexBuffer);

	// Ctreate the thread
	osThreadStaticDef(
				esp_at_spi_task,
				thread,
				Priority,
				0,
				TaskStackSize,
				TaskBuffer,
				&TaskControlBlock);
	TaskHandle = osThreadCreate(osThread(esp_at_spi_task), NULL);
}

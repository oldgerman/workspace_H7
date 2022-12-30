/*
 * esp_at.c
 *
 *  Created on: Dec 29, 2022
 *      Author: PSA
 */



// Copyright 2015-2021 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#include "esp_at.h"

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <time.h>

#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "stream_buffer.h"
static void init_master_hd();

osThreadId spi_at_taskHandle;
uint32_t spi_at_taskBuffer[ 256 ];
osStaticThreadDef_t spi_at_taskControlBlock;

static xQueueHandle msg_queue; // meaasge queue used for communicating read/write start
static StreamBufferHandle_t spi_master_tx_ring_buf = NULL;
static const char* TAG = "SPI AT Master";
static xSemaphoreHandle pxMutex;
static uint8_t initiative_send_flag = 0; // it means master has data to send to slave
static uint32_t plan_send_len = 0; // master plan to send data len

static uint8_t current_send_seq = 0;
static uint8_t current_recv_seq = 0;


/* FRTOS_SPIBase类对象：SPI2_Base */
RAM_REGION_NO_CACHE uint8_t SPI2_RxBuf[FRTOS_SPIBase::sizeCmdOnly];
RAM_REGION_NO_CACHE uint8_t SPI2_TxBuf[FRTOS_SPIBase::sizeCmdOnly];

FRTOS_SPIBase SPI2_Base(hspi2, SPI2_TxBuf, SPI2_RxBuf, FRTOS_SPIBase::sizeCmdOnly);

const uint32_t SPI2_Cmd_PSC  = SPI_BAUDRATEPRESCALER_4;		//40MHz
//const uint32_t SPI2_Cmd_PSC  = SPI_BAUDRATEPRESCALER_16;	//10MHz
//const uint32_t SPI2_Cmd_PSC  = SPI_BAUDRATEPRESCALER_64;	//2.5MHz

/*
 * 当CPOL = 0， CPHA = 0 时 SCK 引脚在空闲状态处于低电平，SCK 引脚的第 1 个边沿捕获传输的第 1 个数据。
 * 乐鑫示例代码spi mode 使用 0模式，即 - 0: (0, 0)，(CPOL, CPHA)
 */
FRTOS_SPICmd SPI2_Cmd(
		&SPI2_Base,
		USR_SPI_CS_GPIO_Port,
		USR_SPI_CS_Pin,
		SPI2_Cmd_PSC,
		SPI_PHASE_1EDGE,	//第一个边沿 CPHA = 0
		SPI_POLARITY_LOW);	//极性为LOW  CPOL = 0

RAM_REGION_NO_CACHE uint8_t trans_data[ESP_SPI_DMA_MAX_LEN] = {0};
RAM_REGION_NO_CACHE uint8_t trans_data_dummy[ESP_SPI_DMA_MAX_LEN] = {0};
RAM_REGION_NO_CACHE spi_send_opt_t 	send_opt = {0};
RAM_REGION_NO_CACHE spi_recv_opt_t 	recv_opt = {0};
RAM_REGION_NO_CACHE spi_send_opt_t 	send_opt_dummy = {0};
RAM_REGION_NO_CACHE spi_recv_opt_t 	recv_opt_dummy = {0};


static void spi_mutex_lock(void)
{
    while (xSemaphoreTake(pxMutex, portMAX_DELAY) != pdPASS);
}

static void spi_mutex_unlock(void)
{
    xSemaphoreGive(pxMutex);
}

/*
This isr handler is called when the handshake line goes high.
There are two ways to trigger the GPIO interrupt:
1. Master sends data, slave has received successfully
2. Slave has data want to transmit
*/
void IRAM_ATTR gpio_handshake_isr_handler()
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

static void at_spi_master_send_data(uint8_t* data, uint16_t len)
{
	spi_transaction_t trans = {
			.instr = CMD_HD_WRDMA_REG,
			.tx_buffer = data,
			.rx_buffer = trans_data_dummy,
			.instr_bytes = 1,							/* 命令字节数 */
			.addr_bytes = 1,							/* 地址字节数 */
			.dummy_bytes = 1,							/* 等待字节数 */
			.data_bytes = len,							/* 数据字节数*/
			.cmd_transfer_mode = TRANSMIT_MODE_POLL,	/* 指令轮询传输 */
			.data_transfer_mode = TRANSMIT_MODE_POLL,    /* 数据量可能较大，使用DMA传输 */
	};
	SPI2_Cmd.busTransferExtCmdAndData(&trans);
}

static void at_spi_master_recv_data(uint8_t* data, uint16_t len)
{
	spi_transaction_t trans = {
			.instr = CMD_HD_RDDMA_REG,  				// master -> slave command, donnot change
			.tx_buffer = trans_data_dummy,					// < 注意使用 pRxData 当作 tx缓冲区
			.rx_buffer = data,							// < 注意使用 pTxData 当作 rx缓冲区
			.instr_bytes = 1,							/* 命令字节数 */
			.addr_bytes = 1,							/* 地址字节数 */
			.dummy_bytes = 1,							/* 等待字节数 */
			.data_bytes = len,							/* 数据字节数*/
			.cmd_transfer_mode = TRANSMIT_MODE_POLL,	/* 指令轮询传输 */
			.data_transfer_mode = RECEIVE_MODE_POLL,	/* 数据量可能较大，使用DMA传输 */
	};
	SPI2_Cmd.busTransferExtCmdAndData(&trans);
}

// send a single to slave to tell slave that master has read DMA done
static void at_spi_rddma_done(void)
{
	spi_transaction_t end_t = {
			.instr = CMD_HD_INT0_REG,
			.instr_bytes = 1,							/* 命令字节数 */
			.addr_bytes = 1,							/* 地址字节数 */
			.dummy_bytes = 1,							/* 等待字节数 */
			.cmd_transfer_mode = TRANSMIT_MODE_POLL,	/* 指令轮询传输 */
	};
	SPI2_Cmd.busTransferExtCmdAndData(&end_t);
}

// send a single to slave to tell slave that master has write DMA done
static void at_spi_wrdma_done(void)
{
	spi_transaction_t end_t = {
			.instr = CMD_HD_WR_END_REG,
			.instr_bytes = 1,							/* 命令字节数 */
			.addr_bytes = 1,							/* 地址字节数 */
			.dummy_bytes = 1,							/* 等待字节数 */
			.cmd_transfer_mode = TRANSMIT_MODE_POLL,	/* 指令轮询传输 */
	};
	SPI2_Cmd.busTransferExtCmdAndData(&end_t);
}

// when spi slave ready to send/recv data from the spi master, the spi slave will a trigger GPIO interrupt,
// then spi master should query whether the slave will perform read or write operation.
static void query_slave_data_trans_info()
{
	memset(&recv_opt, 0x0, sizeof(spi_recv_opt_t));
	spi_transaction_t trans = {
			.instr = CMD_HD_RDBUF_REG,
			.addr = RDBUF_START_ADDR,
			.tx_buffer = (uint8_t *)&recv_opt_dummy,
			.rx_buffer = (uint8_t *)&recv_opt,
			.instr_bytes = 1,							/* 命令字节数 */
			.addr_bytes = 1,							/* 地址字节数 */
			.dummy_bytes = 1,							/* 等待字节数 */
			.data_bytes = 4,							/* 数据字节数*/
			.cmd_transfer_mode = TRANSMIT_MODE_POLL,	/* 指令轮询传输 */
			.data_transfer_mode = RECEIVE_MODE_POLL,	/* 数据量太短，使用轮询传输 */
	};
	SPI2_Cmd.busTransferExtCmdAndData(&trans);
}

// before spi master write to slave, the master should write WRBUF_REG register to notify slave,
// and then wait for handshark line trigger gpio interrupt to start the data transmission.
static void spi_master_request_to_write(uint8_t send_seq, uint16_t send_len)
{
	memset(&send_opt, 0x0, sizeof(spi_send_opt_t));
	send_opt.magic = 0xFE;
	send_opt.send_seq = send_seq;
	send_opt.send_len = send_len;

	spi_transaction_t trans = {
			.instr = CMD_HD_WRBUF_REG,
			.addr = WRBUF_START_ADDR,
			.tx_buffer = (uint8_t *)&send_opt,
			.rx_buffer = (uint8_t *)&send_opt_dummy,
			.instr_bytes = 1,							/* 命令字节数 */
			.addr_bytes = 1,							/* 地址字节数 */
			.dummy_bytes = 1,							/* 等待字节数 */
			.data_bytes = 4,							/* 数据字节数*/
			.cmd_transfer_mode = TRANSMIT_MODE_POLL,	/* 指令轮询传输 */
			.data_transfer_mode = TRANSMIT_MODE_POLL,	/* 数据量太短，使用轮询传输 */
	};
	SPI2_Cmd.busTransferExtCmdAndData(&trans);
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

// write data to spi tx_ring_buf, this is just for test
int32_t write_data_to_spi_task_tx_ring_buf(const void* data, size_t size)
{
    int32_t length = size;

    if (data == NULL  || length > STREAM_BUFFER_SIZE) {
        ESP_LOGE(TAG, "Write data error, len:%d", length);
        return -1;
    }

    length = xStreamBufferSend(spi_master_tx_ring_buf, data, size, portMAX_DELAY);
    return length;
}

// notify slave to recv data
void notify_slave_to_recv(void)
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

static void IRAM_ATTR spi_trans_control_task(const void *arg)
{
	int8_t ret;
    spi_master_msg_t trans_msg = {0};
    uint32_t send_len = 0;

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
            ESP_LOGE(TAG, "%s", trans_data);
//            fflush(stdout);    //Force to print even if have not '\n'
        } else {
            ESP_LOGE(TAG, "Unknow direct: %d", recv_opt.direct);
//            recv_opt.direct = SPI_READ;
//            while(recv_opt.direct == SPI_READ){
//            	query_slave_data_trans_info();
//            	ESP_LOGE(TAG, "now direct:%u", recv_opt.direct);
//            	osDelay(500);
//            }
//        	ESP_LOGE(TAG, "now direct:%u", recv_opt.direct);
            spi_mutex_unlock();
            continue;
        }

        spi_mutex_unlock();
    }
    vTaskDelete(NULL);
}


/* 复位esp从机 */
static void esp_at_reset_slave(){
		HAL_GPIO_WritePin(GPIO_RESET_GPIO_Port, GPIO_RESET_Pin, GPIO_PIN_RESET);
		HAL_Delay(200);	//保持100ms低电平
		HAL_GPIO_WritePin(GPIO_RESET_GPIO_Port, GPIO_RESET_Pin, GPIO_PIN_SET);
		HAL_Delay(5000);	//等待10秒以确保esp_at从机初始化完毕
}

static void init_master_hd()
{
//	esp_at_reset_slave();
    // Create the meaasge queue.
    msg_queue = xQueueCreate(5, sizeof(spi_master_msg_t));
    // Create the tx_buf.
    spi_master_tx_ring_buf = xStreamBufferCreate(STREAM_BUFFER_SIZE, 1);
    // Create the semaphore.
    pxMutex = xSemaphoreCreateMutex();


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
}

void esp_at_init()
{
    init_master_hd();
	osThreadStaticDef(
		  spi_at_task,
		  spi_trans_control_task,
		  osPriorityHigh,
		  0, 1024 * 2,
		  spi_at_taskBuffer,
		  &spi_at_taskControlBlock);
	spi_at_taskHandle = osThreadCreate(osThread(spi_at_task), NULL);
}




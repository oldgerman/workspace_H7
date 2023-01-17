/*
 * esp_at.h
 *
 *  Created on: Dec 29, 2022
 *      Author: PSA
 */

#ifndef BASIC_ESP_AT_H_
#define BASIC_ESP_AT_H_

#include "common_inc.h"

#ifdef __cplusplus
extern "C" {
#endif


#define ESP_LOGE( tag, format, ... ) do { \
		printf("ESP (%lu) %s: " format "\r\n", xTaskGetTickCount(), tag, ##__VA_ARGS__);  \
} while(0)

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
#define STREAM_BUFFER_SIZE    1024 * 8

typedef enum {
    SPI_NULL = 0,
    SPI_READ,         // slave -> master
    SPI_WRITE,             // maste -> slave
} spi_mode_t;

typedef struct {
    bool slave_notify_flag; // when slave recv done or slave notify master to recv, it will be true
} spi_master_msg_t;

typedef struct {
    uint32_t     magic    : 8;    // 0xFE
    uint32_t     send_seq : 8;
    uint32_t     send_len : 16;
} spi_send_opt_t;

typedef struct {
    uint32_t     direct : 8;
    uint32_t     seq_num : 8;
    uint32_t     transmit_len : 16;
} spi_recv_opt_t;

typedef struct {
    spi_mode_t direct;
} spi_msg_t;

void esp_at_init();
int32_t write_data_to_spi_task_tx_ring_buf(const void* data, size_t size);
void notify_slave_to_recv(void);
void gpio_handshake_isr_handler();
void esp_at_reset_slave();
void init_master_hd();

#ifdef __cplusplus
#include "frtos_spi_conf.h"
#include "frtos_spi.h"
//#include "frtos_spi_esp_at.h"

using namespace ns_frtos_spi;
//using namespace ns_frtos_spi_esp_at;
extern FRTOS_SPIBase SPI2_Base;
}
#endif

#endif /* BASIC_ESP_AT_H_ */

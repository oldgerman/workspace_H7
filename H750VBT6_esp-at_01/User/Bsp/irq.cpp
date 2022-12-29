/*
 * irq.cpp
 *
 *  Created on: 2022年12月26日
 *      @Author: OldGerman (过气德国佬)
 */

#include "irq.h"
#include "cppports.h"
using namespace ns_frtos_spi_esp_at;
/**
 * @brief EXTI line detection callback, used as SPI handshake GPIO
 * @notice	该中断需要调用FreeRTOS的API，优先级务必小于等于受FreeRTOS管理中断的最高优先级
 * @param GPIO_Pin: Specifies the pins connected EXTI line
 * @retval None
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if ( GPIO_Pin == GPIO_HANDSHAKE_Pin) {
		FRTOS_SPIDev_ESP_AT::gpio_handshake_isr_handler();
	}
}

/**
 * @brief  TxRx Transfer completed callback.
 * @param  hspi: SPI handle
 * @note   This example shows a simple way to report end of DMA TxRx transfer, and
 *         you can add your own implementation.
 * @retval None
 */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
	SPI2_Base.TxRxCpltCallback(hspi);
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
	SPI2_Base.ErrorCallback(hspi);
}

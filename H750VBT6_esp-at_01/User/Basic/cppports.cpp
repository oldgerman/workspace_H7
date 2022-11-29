/*
 * cppports.cpp
 *
 *  Created on: May 16, 2022
 *      Author: OldGerman
 */

#include "cppports.h"

/*
 * This example shows how application can implement RX and TX DMA for UART.
 * It uses simple packet example approach and 3 separate buffers:
 *
 * - Raw DMA RX buffer where DMA transfers data from UART to memory
 * - Ringbuff for RX data which are transfered from raw buffer and ready for application processin
 * - Ringbuff for TX data to be sent out by DMA TX
 */

/* Includes */
#include "main.h"




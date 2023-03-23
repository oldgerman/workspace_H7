/**
  ******************************************************************************
  * @file        freertos_inc.h
  * @author      OldGerman
  * @created on  Jan 7, 2023
  * @brief
  ******************************************************************************
  * @attention
  *
  * Copyright (C) 2022 OldGerman.
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see https://www.gnu.org/licenses/.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef FREERTOS_INC_H_
#define FREERTOS_INC_H_

#ifdef __cplusplus
extern "C" {
#endif
/* List of semaphores --------------------------------------------------------*/
extern osSemaphoreId sem_usb_irq;
extern osSemaphoreId sem_usb_rx;
extern osSemaphoreId sem_usb_tx;
extern osSemaphoreId sem_adc_dma;

/* List of quenes ------------------------------------------------------------*/
/* List of tasks -------------------------------------------------------------*/
/*--------------------------------- System Tasks ------------------------------*/
extern const uint32_t commTaskStackSize;
extern const uint32_t UsbIrqTaskStackSize;
extern const uint32_t usbServerTaskStackSize;
extern osThreadId_t defaultTaskHandle;
extern osThreadId_t commTaskHandle;
extern osThreadId_t usbIrqTaskHandle;
extern osThreadId_t usbServerTaskHandle;

/*---------------------------------- User Tasks -------------------------------*/
extern const uint32_t ledTaskStackSize;
extern osThreadId_t ledTaskHandle;
extern const uint32_t frameProcessorTaskStackSize;
extern osThreadId_t frameProcessorTaskHandle;
/*------------- xxxK (used) / xxxK (for FreeRTOS) / xxxK (total) --------------*/

#ifdef __cplusplus
}
#endif
#endif	/* FREERTOS_INC_H_ */

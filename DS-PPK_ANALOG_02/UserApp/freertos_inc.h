/**
  ******************************************************************************
  * @file           : freertos_inc.h
  * @brief          :
  ******************************************************************************
  * @Created on		: Jan 7, 2023
  * @Author 		: OldGerman
  * @attention		:
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FREERTOS_INC_H
#define __FREERTOS_INC_H

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

/*------------- xxxK (used) / xxxK (for FreeRTOS) / xxxK (total) --------------*/

#ifdef __cplusplus
}
#endif
#endif	/* __FREERTOS_INC_H */

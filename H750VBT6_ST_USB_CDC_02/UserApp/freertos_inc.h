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

/* List of quenes ------------------------------------------------------------*/

/* List of tasks -------------------------------------------------------------*/

/* List of tasks -------------------------------------------------------------*/
/*--------------------------------- System Tasks ------------------------------*/
extern osThreadId defaultTaskHandle;      // Usage: 2048 Bytes stack
extern osThreadId usbIrqTaskHandle;       // Usage: 512  Bytes stack
extern osThreadId usbServerTaskHandle;    // Usage: 2048 Bytes stack

/*---------------------------------- User Tasks -------------------------------*/

/*------------- xxxK (used) / xxxK (for FreeRTOS) / xxxK (total) --------------*/

#ifdef __cplusplus
}
#endif
#endif	/* __FREERTOS_INC_H */

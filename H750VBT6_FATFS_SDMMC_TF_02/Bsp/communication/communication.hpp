/**
  ******************************************************************************
  * @file           : communication.h
  * @brief          :
  ******************************************************************************
  * @Created on		: Jan 7, 2023
  * @Author 		: OldGerman
  * @attention		:
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __COMMUNICATION_H
#define __COMMUNICATION_H

#ifdef __cplusplus
extern "C" {
#endif
/* USER CODE BEGIN C SCOPE ---------------------------------------------------*/
/* Includes ------------------------------------------------------------------*/
#include <cmsis_os.h>

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
void InitCommunication(void);
void CommitProtocol();	//在本文件定义
void CommunicationTask(void *ctx);
void UsbDeferredInterruptTask(void *ctx);
/* Private defines -----------------------------------------------------------*/

/* USER CODE END C SCOPE -----------------------------------------------------*/
#ifdef __cplusplus
}
/* USER CODE BEGIN C++ SCOPE -------------------------------------------------*/
/* Includes ------------------------------------------------------------------*/
#include <functional>
#include <limits>
#include "ascii_processor.hpp"
#include "interface_usb.hpp"
//#include "interface_uart.hpp"

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/

/* Private defines -----------------------------------------------------------*/
#define COMMIT_PROTOCOL \
using treeType = decltype(MakeObjTree());\
uint8_t treeBuffer[sizeof(treeType)];\
void CommitProtocol()\
{\
    auto treePtr = new(treeBuffer) treeType(MakeObjTree());\
    fibre_publish(*treePtr);\
}\
/* USER CODE END C++ SCOPE ---------------------------------------------------*/
#endif
#endif /* __COMMUNICATION_H */

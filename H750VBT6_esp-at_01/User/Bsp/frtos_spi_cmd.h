/*
 * frtos_spi.h
 *
 *  Created on: 2022年12月06日
 *      @Author: OldGerman (过气德国佬)
 *		@Modified: 使用C++封装一次安富莱的bsp_spi_bus，可以创建多路SPI总线的实例，
 *					构造时指向多个设备实例，支持同一路总线的设备互斥
 */

#ifndef INC_FRTOS_SPI_CMD_H
#define INC_FRTOS_SPI_CMD_H
#include "frtos_spi.h"
#ifdef INC_FRTOS_SPI_H
#ifdef __cplusplus
extern "C" {
#endif

/* 私有包含 Private includes -------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* 导出类型 Exported types ---------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* 导出常量 Exported constants -----------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* 导出的宏 Exported macro ---------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* 导出函数原型 Exported functions prototypes --------------------------------*/
/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* 私有定义 Private defines --------------------------------------------------*/
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */
#ifdef __cplusplus

using namespace frtos_spi_enums;

#endif	/* __cplusplus */
#endif	/* INC_FRTOS_SPI_H */
#endif /* INC_FRTOS_SPI_CMD_H */

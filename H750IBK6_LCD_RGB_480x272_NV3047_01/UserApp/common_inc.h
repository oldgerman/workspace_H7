/**
  ******************************************************************************
  * @file        common_inc_h
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
#ifndef COMMON_INC_H_
#define COMMON_INC_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "bsp.h"
#include "cmsis_os.h"
#include "freertos_inc.h"

/* Exported types ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
#define PROJECT_FW_VERSION 1.0
/* Exported macro ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
void Main(void);

#ifdef __cplusplus
}
#include "communication.hpp"
//#include "dynamic_ram.h"
#endif

#endif /* COMMON_INC_H_ */

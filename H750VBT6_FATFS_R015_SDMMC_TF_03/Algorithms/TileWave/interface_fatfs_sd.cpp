/**
  ******************************************************************************
  * @file        interface_fatfs_sd.cpp
  * @author      OldGerman
  * @created on  Mar 26, 2023
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

/* Includes ------------------------------------------------------------------*/
#include "interface_fatfs_sd.h"
#include "common_inc.h"
#include "interface_usb.hpp"
#include "tile_wave.h"
#include "demo_sd_fatfs.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
const uint32_t fatfsSDTaskStackSize = 512 * 4;

/* Private constants ---------------------------------------------------------*/
const osThreadAttr_t fatfsSDTask_attributes = {
		.name = "fatfsSDTask",
		.stack_size = fatfsSDTaskStackSize,
		.priority = (osPriority_t) osPriorityLow,
};
/* Exported variables --------------------------------------------------------*/
osThreadId_t fatfsSDTaskHandle;
osMessageQueueId_t fatfs_sd_msg_queue;

/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Function implementations --------------------------------------------------*/
/**
 * @brief  FATFS SD 卡任务函数
 * @param  argument Pointer to a void
 * @retval None
 */
static void fatfsSDTask(void* argument)
{
	for(;;)
	{

	}
}
/**
 * @brief  初始化 FATFS SD 卡任务函数
 * @param  None
 * @retval None
 */
void fatfsSDInit()
{
	fatfsSDTaskHandle = osThreadNew(fatfsSDTask, NULL, &fatfsSDTask_attributes);
}

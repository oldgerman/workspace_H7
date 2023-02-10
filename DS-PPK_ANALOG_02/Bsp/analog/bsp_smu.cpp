/**
  ******************************************************************************
  * @file        bsp_smu.cpp
  * @author      OldGerman
  * @created on  Jan 30, 2023
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
#include "bsp_smu.h"
#include "common_inc.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Function implementations --------------------------------------------------*/
/**
 * @brief	控制SMU的EN引脚电平
 * 			控制供电芯片TPS65131、TPS63020、TPS7A8300的开关
 * @param	enable	1: 使能 0: 关闭
 */
void bsp_smuEnablePowerChips(bool enable)
{
	HAL_GPIO_WritePin(SMU_EN_GPIO_Port, SMU_EN_Pin, (GPIO_PinState)enable);
}

/**
 * @brief	控制VOUT-FET
 * 			控制输出MOSFET的开关
 * @param	enable	1: 关闭 0: 使能
 */
void bsp_smuEnableVoutMosfet(bool enable)
{
	HAL_GPIO_WritePin(VOUT_EN_GPIO_Port, VOUT_EN_Pin, (GPIO_PinState)(!enable));
}

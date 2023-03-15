/**
  ******************************************************************************
  * @file        bsp_mux.cpp
  * @author      OldGerman
  * @created on  Jan 31, 2023
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
#include "bsp_mux.h"
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
void bsp_muxFunSet(mux_fun_t sLinesCode)
{
	/**
	 * CBA	CHx
	 * 000	0
	 * 001	1
	 * 010	2
	 * 011	3
	 * 100	4
	 * 101	5
	 * 110	6
	 * 111	7
	 */
	if(sLinesCode < 8) {
		HAL_GPIO_WritePin(MUX_C_GPIO_Port, MUX_C_Pin, (GPIO_PinState)((sLinesCode & 4) >> 2));
		HAL_GPIO_WritePin(MUX_B_GPIO_Port, MUX_B_Pin, (GPIO_PinState)((sLinesCode & 2) >> 1));
		HAL_GPIO_WritePin(MUX_A_GPIO_Port, MUX_A_Pin, (GPIO_PinState)((sLinesCode & 1) >> 0));
	}
}

void bsp_muxFunTest()
{
	static uint8_t cnt_mux = 0;
	static mux_fun_t sLinesCode = MUX_FUN_NC;
	switch (cnt_mux) {
		case 0:
			sLinesCode = MUX_FUN_CAL_RES_500KR;
			break;
		case 1:
			sLinesCode = MUX_FUN_CAL_RES_50KR;
			break;
		case 2:
			sLinesCode = MUX_FUN_CAL_RES_500mR;
			break;
		case 3:
			sLinesCode = MUX_FUN_CAL_RES_5KR;
			break;
		case 4:
			sLinesCode = MUX_FUN_CAL_RES_500R;
			break;
		case 5:
			sLinesCode = MUX_FUN_CAL_RES_50R;
			break;
		case 6:
			sLinesCode = MUX_FUN_DPDT_7222_S;
			break;
		case 7:
			sLinesCode = MUX_FUN_NC;
			break;
		default:
			break;
	}
	bsp_muxFunSet(sLinesCode);
	++cnt_mux;
	cnt_mux = cnt_mux % 8;
}

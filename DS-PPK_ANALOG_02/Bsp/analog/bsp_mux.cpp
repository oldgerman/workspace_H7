/*
 * bsp_mux.cpp
 *
 *  Created on: Jan 31, 2023
 *      Author: OldGerman
 */

#include "bsp_analog.h"

void mux_FunSet(mux_fun_t sLinesCode)
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

void mux_FunTest()
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
	mux_FunSet(sLinesCode);
	++cnt_mux;
	cnt_mux = cnt_mux % 8;
}

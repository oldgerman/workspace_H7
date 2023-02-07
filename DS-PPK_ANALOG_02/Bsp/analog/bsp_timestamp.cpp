/*
 * bsp_timestamp.cpp
 *
 *  Created on: Feb 7, 2023
 *      Author: OldGerman
 */
#include "bsp_analog.h"
#include "tim.h"

ALIGN_32BYTES(__attribute__((section (".RAM_D2_Array"))) timestamp_t timestamp);

void bsp_timestamp_init()
{
	memset(&timestamp, 0x0, sizeof(timestamp_t));

	HAL_TIM_Base_Start(&htim2);
}

/**
 * tim2 的 CNT 寄存器每 10us 自增 1，约 11.93 小时后溢出
 */
uint32_t bsp_timestamp_get()
{
	return htim2.Instance->CNT;
}

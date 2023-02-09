/*
 * bsp_smu.cpp
 *
 *  Created on: Jan 30, 2023
 *      Author: OldGerman
 */

#include "bsp_analog.h"

/**
 * @brief	控制SMU的EN引脚电平
 * 			控制供电芯片TPS65131、TPS63020、TPS7A8300的开关
 * @param	enable	1: 使能 0: 关闭
 */
void bsp_smu_set_en(bool enable)
{
	HAL_GPIO_WritePin(SMU_EN_GPIO_Port, SMU_EN_Pin, (GPIO_PinState)enable);
}

/**
 * @brief	控制VDOUT-FET
 * 			控制输出MOSFET的开关
 * @param	enable	1: 关闭 0: 使能
 */
void bsp_vdout_fet_en(bool enable)
{
	HAL_GPIO_WritePin(VOUT_EN_GPIO_Port, VOUT_EN_Pin, (GPIO_PinState)(!enable));
}

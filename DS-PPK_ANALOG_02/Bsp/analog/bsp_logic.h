/*
 * bsp_logic.h
 *
 *  Created on: Feb 9, 2023
 *      Author: OldGerman
 */

#ifndef ANALOG_BSP_LOGIC_H_
#define ANALOG_BSP_LOGIC_H_

#define LOGIC_FB_R1_KR   330.0f  // 330K
#define LOGIC_FB_R2_KR   40.2f   // 40.2K
#define LOGIC_FB_R3_KR   215.0f  // 215K
#define LOGIC_FB_VFB_mV  500.0f  // 500mV

#define LOGIC_LEVEL_HIGH_MAX_mV 5000.0f // 逻辑电平高上限 5V TTL, CMOS
#define LOGIC_LEVEL_HIGH_MIN_mV 1800.0f // 逻辑电平高下限 1.8V CMOS

void bsp_logicInit();
uint32_t bsp_logicSetVoltageLevel(float mV, bool compensation = true);
void bsp_logicVoltageCal();

#endif /* ANALOG_BSP_LOGIC_H_ */

/*
 * bsp_analog.h
 *
 *  Created on: Jan 30, 2023
 *      Author: PSA
 */

#ifndef ANALOG_BSP_ANALOG_H_
#define ANALOG_BSP_ANALOG_H_

#include "common_inc.h"

void bsp_adc1Init();
void bsp_adc2Init();
void bsp_adc3Init();
void bsp_adc1GetValues();
void bsp_adc2GetValues();
void bsp_adc3GetValues();
void bsp_smu_set_en(bool enable);
void bsp_vdout_fet_en(bool enacle);

extern float vref;

#endif /* ANALOG_BSP_ANALOG_H_ */

/*
 * bsp_sound.cpp
 *
 *  Created on: Feb 9, 2023
 *      Author: OldGerman
 */

#include "bsp_sound.h"
#include "tim.h"

void bsp_soundPlayTune(sound_tune_t iTune)
{
	uint8_t ret = HAL_OK;
	uint32_t ARR;
	uint32_t CCR;
	uint32_t RCR;
	switch (iTune) {
		case TUNE_CLICK:
			ARR = 200;		// Freq = 2KHz
			CCR = ARR / 2;	// Duty Cycle = 50%
			RCR = 200;		// Repet count = 200
							// total duration = 200 * 1000ms / 2K = 100ms
			break;
		case TUNE_BEEP:
			ARR = 100;		// 4KHz
			CCR = ARR / 2;	// Duty Cycle = 50%
			RCR = 200;		// Repet count = 200
							// total duration = 200 * 1000ms / 4K = 50ms
			break;
		case TUNE_POWER_UP:

			break;
		case TUNE_POWER_DOWN:

			break;
		case TUNE_SHUTTER:

			break;
		default:
			return;
	}

	htim17.Instance->ARR = ARR - 1;		//50%占空比
	htim17.Instance->CCR1 = CCR - 1;	//50%占空比
	htim17.Instance->RCR = RCR - 1;		//256次

	ret += (uint8_t)HAL_TIM_Base_Stop_IT(&htim17);
	ret += (uint8_t)HAL_TIM_OnePulse_Stop_IT(&htim17, TIM_CHANNEL_1);

	ret += (uint8_t)HAL_TIM_Base_Start_IT(&htim17);
	ret += (uint8_t)HAL_TIM_OnePulse_Start_IT(&htim17, TIM_CHANNEL_1);

	if(ret != HAL_OK) {
		/* PWM Generation Error */
		Error_Handler();
	}
}

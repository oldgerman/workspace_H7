/**
  ******************************************************************************
  * @file        bsp_sound.cpp
  * @author      OldGerman
  * @created on  Feb 10, 2023
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
#include "bsp_sound.h"
#include "tim.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Function implementations --------------------------------------------------*/
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


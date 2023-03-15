/**
  ******************************************************************************
  * @file        bsp_lptim_pwm.h
  * @author      OldGerman
  * @created on  Aug 30, 2022
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef BSP_LPTIM_PWM_H_
#define BSP_LPTIM_PWM_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "bsp_data structure.h"
#include "bsp_functions.h"
#include "stm32h7xx_hal_conf.h"
/* Exported types ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
#ifdef HAL_LPTIM_MODULE_ENABLED
HAL_StatusTypeDef bsp_LPTIMx_PWM_En(LPTIM_HandleTypeDef *hlptim, bool enable);
pwmSet_InfoTypeDef bsp_LPTIMx_PWM_Set(
		LPTIM_HandleTypeDef *hlptim,
		uint32_t LptimClockFreq,
		uint32_t pwmFrequency,
		float pwmDutyCycle);
#endif

#ifdef __cplusplus
}
#endif

#endif /* BSP_LPTIM_PWM_H_ */

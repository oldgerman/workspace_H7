/**
  ******************************************************************************
  * @file        bsp_data structure.h
  * @author      OldGerman
  * @created on  Aug 31, 2022
  * @brief       bsp公用结构体类型
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
#ifndef BSP_DATA_STRUCTURE_H_
#define BSP_DATA_STRUCTURE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef struct pwmSetInfo{
//	uint32_t clkSourceFreq;			/* 时钟源频率，单位Hz */
	float pwm_Dutycycle_Expect;		/* 期望的pwm占空比 */
	uint32_t pwm_Frequency_Expect;	/* 期望的pwm频率 */
	float pwm_Dutycycle;			/* 计算的pwm占空比 */
	float pwmStep_Dutycycle;		/* 计算的pwm占空比步幅 */
	float pwm_Frequency;			/* 计算的pwm频率 */
	HAL_StatusTypeDef hal_Status;	/* HAL Status */
}pwmSet_InfoTypeDef;

/* Exported define -----------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* BSP_DATA_STRUCTURE_H_ */

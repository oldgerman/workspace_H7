/**
  ******************************************************************************
  * @file        bsp_logic.h
  * @author      OldGerman
  * @created on  Feb 9, 2023
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
#ifndef BSP_LOGIC_H_
#define BSP_LOGIC_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
#define LOGIC_FB_R1_KR   330.0f  // 330K
#define LOGIC_FB_R2_KR   40.2f   // 40.2K
#define LOGIC_FB_R3_KR   215.0f  // 215K
#define LOGIC_FB_VFB_mV  500.0f  // 500mV

#define LOGIC_LEVEL_HIGH_MAX_mV 5000.0f // 逻辑电平高上限 5V TTL, CMOS
#define LOGIC_LEVEL_HIGH_MIN_mV 1800.0f // 逻辑电平高下限 1.8V CMOS

/* Exported macro ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern volatile uint32_t logic_convCallbackCnt;
extern uint16_t logic_data[sample_buffer_size];

/* Exported functions --------------------------------------------------------*/
void bsp_logicInit();
void bsp_logicVoltageCal();
uint32_t bsp_logicSetVoltageLevel(float mV, bool compensation = true);

#ifdef __cplusplus
}
#endif

#endif /* BSP_LOGIC_H_ */

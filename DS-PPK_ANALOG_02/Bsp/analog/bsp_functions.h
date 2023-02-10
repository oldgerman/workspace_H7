/**
  ******************************************************************************
  * @file        bsp_functions.h
  * @author      OldGerman
  * @created on  Sep 6, 2022
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
#ifndef BSP_FUNCTIONS_H_
#define BSP_FUNCTIONS_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
uint32_t twoNthPower(uint8_t Nth);
uint8_t twoNthPowerOfNth(uint32_t num);
double fmap(double x, double in_min, double in_max, double out_min, double out_max);
void bsp_convertLevelToBSRR(uint8_t *ptrBitArray, uint32_t bitArrayLengthInByte, uint32_t* ptrBSRRLArray, uint16_t GPIO_Pin);

#ifdef __cplusplus
}
#endif

#endif /* BSP_FUNCTIONS_H_ */

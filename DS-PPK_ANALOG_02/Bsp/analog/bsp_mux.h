/**
  ******************************************************************************
  * @file        bsp_mux.h
  * @author      OldGerman
  * @created on  Jan 31, 2023
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
#ifndef BSP_MUX_H_
#define BSP_MUX_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef enum {
	/* FUN			  		CHx */
	MUX_FUN_CAL_RES_500mR 	= 6,
	MUX_FUN_CAL_RES_50R 	= 1,
	MUX_FUN_CAL_RES_500R 	= 0,
	MUX_FUN_CAL_RES_5KR 	= 3,
	MUX_FUN_CAL_RES_50KR 	= 4,
	MUX_FUN_CAL_RES_500KR 	= 7,
	MUX_FUN_DPDT_7222_S 	= 2,
	MUX_FUN_NC 				= 5,
}mux_fun_t;

/* Exported define -----------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
void bsp_muxFunSet(mux_fun_t sLinesCode);
void bsp_muxFunTest();

#ifdef __cplusplus
}
#endif

#endif /* BSP_MUX_H_ */

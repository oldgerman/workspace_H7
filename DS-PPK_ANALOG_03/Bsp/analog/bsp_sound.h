/**
  ******************************************************************************
  * @file        bsp_sound.h
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef BSP_SOUND_H_
#define BSP_SOUND_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef enum {
	TUNE_NONE = 0,		//无声
	TUNE_CLICK,			//点击声
	TUNE_BEEP,			//嘟嘟声
	TUNE_POWER_UP,		//电源开机声
	TUNE_POWER_DOWN,	//电源关闭声
	TUNE_SHUTTER		//关机声
}sound_tune_t;

/* Exported define -----------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
void bsp_soundPlayTune(sound_tune_t iTune);

#ifdef __cplusplus
}
#endif

#endif /* BSP_SOUND_H_ */

/**
  ******************************************************************************
  * @file        touchpad.cpp
  *
  * @author      OldGerman
  * @created on  Dec 19, 2023
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

#ifndef INDEV_H
#define INDEV_H

#ifdef __cplusplus
#include "touchPointFSM.h"
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "bsp.h"
#include "hal_stm_lvgl/tft/tft.h"
#include "lvgl.h"
#include"lcd_touch.h"

/* Exported types ------------------------------------------------------------*/
typedef struct {
    struct
    {
        int16_t x;
        int16_t y;
    } point;       // 缩放焦点
    struct
    {
        float x; // x 方向
        float y; // y 方向
        float h; // 斜边方向
    } zoom;      // 缩放比(增量)
}TouchData_t;

/* Exported define -----------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern osMessageQueueId_t xMsgQueueTouchData;
extern TouchPointFSM touchPointFSM;

/* Exported functions --------------------------------------------------------*/
void touchpad_init(void);

#ifdef __cplusplus
}
#endif
#endif

/**
  ******************************************************************************
  * @file        TouchPointFSM.h
  * @author      OldGerman
  * @created on  2023年12月25日
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
#ifndef TOUCHPOINTFSM_H_
#define TOUCHPOINTFSM_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdarg.h>
#include <stdio.h>
#ifdef TOUCHPOINTFSM_EN_ARM_MATH
#include "arm_math.h"
#else
#include "math.h"
#endif

/* Exported types ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#include "kalmanfilter.h"

/**
 * @brief 触摸点有限状态机 (Finite-state machine)
 */
class TouchPointFSM
{
public:
    /**
     * @brief 3种状态
     */
    typedef enum
    {
        STATE_0 = 0,
        STATE_1 = 1,
        STATE_2 = 2,
        STATE_LOCK = 6, // 状态锁定，最多运行一个状态机周期
    } State;
    /**
     * @brief 7种事件       // debug
     */
    typedef enum
    {
        EVENT_NONE = 0,    // 00
        EVENT_0_TO_1 = 1,  // 01
        EVENT_0_TO_2 = 2,  // 02
        EVENT_1_TO_0 = 10, // 10
        EVENT_1_TO_2 = 12, // 12
        EVENT_2_TO_0 = 20, // 20
        EVENT_2_TO_1 = 21  // 21
    } Event;

    typedef struct
    {
        bool pressed; // true: 手还停留在触摸屏上 false: 手离开触摸屏
        uint8_t num;  // 触摸点总数
        struct
        {
            int16_t x;
            int16_t y;
        } points[2]; // 触摸点坐标
    } InputData;

    typedef struct
    {
        bool pressed; // 通知LVGL lv_indev_data_t::state，   0: 传参 LV_INDEV_STATE_RELEASED,  1: 传参 LV_INDEV_STATE_PRESSED
        bool zooming; // 通知LVGL控件的事件回调函数执行缩放，0: 不执行缩放,                    1: 执行缩放
        struct
        {
            int16_t x;
            int16_t y;
        } point; // 缩放焦点
        struct
        {
            float x; // x 方向
            float y; // y 方向
            float h; // 斜边方向
        } zoom;      // 缩放比(增量)
    } OutputData;

    /* @brief 卡尔曼滤波器的开关比特 */
    typedef union
    {
        struct
        {
            uint8_t px : 1;     // bit[0]
            uint8_t py : 1;     // bit[1]
            uint8_t zx : 1;     // bit[2]
            uint8_t zy : 1;     // bit[3]
            uint8_t zh : 1;     // bit[4]
            uint8_t unused : 3; // bit[5:7]
        }bits;
        uint8_t ctrl;
    } SwitchBitsKalmanFilter;

    TouchPointFSM(
        bool debug = false,
        int16_t area_w = 1,
        int16_t area_h = 1,
        SwitchBitsKalmanFilter switchBitsKalmanFilter = {
            .bits = {
                .px = 1, // 打开坐标点x滤波
                .py = 1, // 打开坐标点y滤波
                .zx = 1, // 打开缩放比x滤波
                .zy = 1, // 打开缩放比y滤波
                .zh = 1  // 打开缩放比h滤波
            }});
    ~TouchPointFSM() {}
    OutputData update(InputData &act);

    bool debug_;                                    // 是否打印调试信息
    int16_t area_w_;                                // 显示缩放对象的画布宽度
    int16_t area_h_;                                // 显示缩放对象的画布高度
    SwitchBitsKalmanFilter switchBitsKalmanFilter_; // 卡尔曼滤波器开关比特

private:
    void debugPrint(char *fmt, ...);
    void limitZoom(float *data);
    void limitInt16(int16_t *data, int16_t low, int16_t high);

    InputData last_;                // 上一轮状态机循环结束时的数据
    OutputData out_;
    State state_;
    Event event_;
    uint8_t state_lock_counter_;    // STATE_LOCK 的递减计数器，当计数为0时，切换到 STATE_0
    bool state_2_begin_;             // 一轮2点事件开始标记
};

#endif /* TOUCHPOINTFSM_H_ */

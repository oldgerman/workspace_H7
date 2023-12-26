/**
  ******************************************************************************
  * @file        touchPointFSM.cpp
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

/* Includes ------------------------------------------------------------------*/
#include "touchPointFSM.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Function implementations --------------------------------------------------*/

/**
 * @brief  构造函数
 * @param  debug 是否打印调试信息
 * @retval None
 */
TouchPointFSM::TouchPointFSM(bool debug, int16_t area_w, int16_t area_h,  SwitchBitsKalmanFilter switchBitsKalmanFilter)
{
    debug_ = debug;
    state_ = STATE_0;
    event_ = EVENT_NONE;
    state_lock_counter_ = 0;
    area_w_ = area_w;
    area_h_ = area_h;
    switchBitsKalmanFilter_ = switchBitsKalmanFilter;
    state_2_begin_ = true;
}

/**
 * @brief  输出调试信息
 * @param  fmt  字符串指针
 * @param  ...  可变参数
 * @retval None
 */
void TouchPointFSM::debugPrint(char *fmt, ...)
{
    if(debug_) {
        va_list va;
        va_start(va, fmt);
        vfprintf(stderr, fmt, va);
        va_end(va);
    }
}

/**
 * @brief  限幅，约束单步缩放比
 * @param  data 待限幅的数据
 * @retval None
 */
void TouchPointFSM::limitZoom(float* data)
{
    static const float lf_max = 1.414f;   // 最大单步放大比
    static const float lf_min  = 1/1.414f; // 最大单步缩小比
    *data = ((*data)<(lf_min)?(lf_min):((*data)>(lf_max)?(lf_max):(*data)));
}

/**
 * @brief  约束int16
 * @param  data 待限幅的数据
 * @param  low  下界
 * @param  high 上界
 * @retval None
 */
void TouchPointFSM::limitInt16(int16_t* data, int16_t low, int16_t high)
{
    *data = ((*data)<(low)?(low):((*data)>(high)?(high):(*data)));
}

/**
 * @brief   更新状态机
 * @param   act 当前数据
 * @retval  触摸点和缩放数据
 */
TouchPointFSM::OutputData TouchPointFSM::update(InputData &act)
{
    uint8_t state_loop_counter = 1; // 状态机循环次数，由状态机自动改变
    for (;;)
    {
        if (state_loop_counter > 0)
            state_loop_counter--;
        else
            break;

        switch (state_)
        {
        case STATE_LOCK:
        {
            out_.zooming = 0;
            debugPrint("[TouchPointFSM][STATE_%d][EVENT_%02d]\r\n", state_, event_);
            out_.pressed = 0; // STATE_LOCK 期间强制认为手离开屏幕

            switch (event_)
            {
            case EVENT_1_TO_2:
            {
                state_ = STATE_2;
                event_ = EVENT_NONE;
                state_2_begin_ = true;
            }
            break;
            case EVENT_2_TO_1:
            {
                state_ = STATE_1;
                event_ = EVENT_NONE;
            }
            break;
            };
        }
        break;
        case STATE_0:
        {
            out_.zooming = 0;
            debugPrint("[TouchPointFSM][STATE_%d][EVENT_%02d]\r\n", state_, event_);

            switch (event_)
            {
            case EVENT_NONE:
            case EVENT_1_TO_0:
            case EVENT_2_TO_0:
            {
                out_.pressed = act.pressed;
                if (act.pressed)
                {
                    if (act.num == 1)
                    {
                        state_ = STATE_1;
                        event_ = EVENT_0_TO_1;
                        state_loop_counter++;
                    }
                    else if (act.num == 2)
                    {
                        state_ = STATE_2;
                        event_ = EVENT_0_TO_2;
                        state_loop_counter++;
                    }
                }
                /* 保持不变 */
                else
                {
                    // state_ = STATE_0;
                    // event_ = event_;
                }
            }
            break;
            }
        }
        break;
        case STATE_1:
        {
            out_.zooming = 0;
            debugPrint("[TouchPointFSM][STATE_%d][EVENT_%02d]\r\n", state_, event_);

            switch (event_)
            {
            case EVENT_NONE:
            case EVENT_0_TO_1:
            {
                if (act.pressed)
                {
                    if (act.num == 1)
                    {
                        // state_ = STATE_1;
                        event_ = EVENT_NONE;

                        out_.pressed = act.pressed;
                        /* 正常的单点触摸 */
                        // DEBUG_PRINT("【1】touchPointsNum = %d\r\n", touchPointsNum);
                        last_ = act;
                        /* 将当前单点坐标保存为上次单点的坐标 */
                        out_.point.x = last_.points[0].x;
                        out_.point.y = last_.points[0].y;
                    }
                    else if (act.num == 2)
                    {
                        state_ = STATE_2;
                        event_ = EVENT_1_TO_2;
                        state_loop_counter++;
                    }
                }
                else
                {
                    out_.pressed = act.pressed;
                    state_ = STATE_0;
                    event_ = EVENT_1_TO_0;
                    state_loop_counter++;
                }
            }
            break;
            case EVENT_2_TO_1:
            {
                state_ = STATE_LOCK;
                // event_ = event_; //!< 保持 event_ 不变
            }
            break;
            }
        }
        break;
        case STATE_2:
        {
            out_.zooming = 1;
            debugPrint("[TouchPointFSM][STATE_%d][EVENT_%02d]\r\n", state_, event_);

            /* 卡尔曼滤波器组 */
            static kalmanFilter px_kalman_filter;
            static kalmanFilter py_kalman_filter;
            kalmanFilter::Attr_t z_attr = {
                .p_last = 0.02,
                .out = 1 // 卡尔曼滤波器输出 初始化值为0
            };
            static kalmanFilter zx_kalman_filter(z_attr);
            static kalmanFilter zy_kalman_filter(z_attr);
            static kalmanFilter zh_kalman_filter(z_attr);

            switch (event_)
            {
            case EVENT_0_TO_2:
            {
                event_ = EVENT_NONE;
                state_loop_counter++;

                // last_ = act;
                state_2_begin_ = true;
            }
            break;
            /* 从1到2点切换时，需要STATE_LOCK，向LVGL传递2次假释放，退出STATE_LOCK后，
             * 才能进入两点触摸事件，将两点计算的焦点作为一个点传给LVGL */
            case EVENT_1_TO_2:
            {
                state_ = STATE_LOCK;
                // event_ = event_; //!< 保持 event_ 不变

                // last_ = act;
                state_2_begin_ = true;
            }
            break;
            case EVENT_NONE:
            {
                /* 处理2点触摸 */

                if (act.pressed)
                {
                    if (act.num == 1)
                    {
                        state_ = STATE_1;
                        event_ = EVENT_2_TO_1;
                        state_loop_counter++;
                    }
                    else if (act.num == 2)
                    {
                        // state_ = STATE_2;
                        event_ = EVENT_NONE;

                        if(state_2_begin_) {
                            last_ = act;
                            // 多点状态开始时复位卡尔曼滤波器组
                            if(switchBitsKalmanFilter_.bits.px) px_kalman_filter.reset();
                            if(switchBitsKalmanFilter_.bits.px) py_kalman_filter.reset();
                            if(switchBitsKalmanFilter_.bits.zx) zx_kalman_filter.reset();
                            if(switchBitsKalmanFilter_.bits.zy) zy_kalman_filter.reset();
                            if(switchBitsKalmanFilter_.bits.zh) zh_kalman_filter.reset();
                        }
                        
                        /* 计算当前两点的焦点坐标 */
                        out_.point.x = (act.points[0].x + act.points[1].x) / 2;
                        out_.point.y = (act.points[0].y + act.points[1].y) / 2;
                        // debugPrint("act_focus: (%4d, %4d)\r\n", act_focus.x, act_focus.y);

                        /* 卡尔曼滤波：缩放焦点坐标 */
                        if(state_2_begin_) {
                            if(switchBitsKalmanFilter_.bits.px) px_kalman_filter.preheat(out_.point.x, 20);
                            if(switchBitsKalmanFilter_.bits.py) py_kalman_filter.preheat(out_.point.y, 20);
                            debugPrint("pxy_Kalmanfilter.preheat()\r\n");
                        }
                        if(switchBitsKalmanFilter_.bits.px) out_.point.x = px_kalman_filter.update(out_.point.x);
                        if(switchBitsKalmanFilter_.bits.py) out_.point.y = py_kalman_filter.update(out_.point.y);

                        /**
                         * 计算缩放倍率时，约束值的注意点
                         * 当前两个指头的坐标是 (xa0, ya0), (xa1, ya1) ，上次两个指头的坐标是  (x0a0, y0a0), (x0a1, y0a1)，都是浮点类型
                         *  x_zoom_percent =  fabs (xa0 - xa1) / (x0a0 - x0a1) | //!< 算完要取绝对值
                         */
                        int16_t act_x_diff = act.points[0].x - act.points[1].x;
                        int16_t act_y_diff = act.points[0].y - act.points[1].y;
                        int16_t last_x_diff = last_.points[0].x - last_.points[1].x;
                        int16_t last_y_diff = last_.points[0].y - last_.points[1].y;

                        /* 约束int不等于0，防止浮点计算出 nan */
                        out_.zoom.x = fabs((float)((act_x_diff == 0) ? (1) : (act_x_diff)) / ((last_x_diff == 0) ? (1) : (last_x_diff)));
                        out_.zoom.y = fabs((float)((act_y_diff == 0) ? (1) : (act_y_diff)) / ((last_y_diff == 0) ? (1) : (last_y_diff)));

                        /* 限幅：水平和垂直方向的缩放倍率 */
                        limitZoom(&out_.zoom.x);
                        limitZoom(&out_.zoom.y);

                        /* 卡尔曼滤波：水平和垂直方向的缩放倍率*/
                        if(state_2_begin_) {
                            if(switchBitsKalmanFilter_.bits.zx) zx_kalman_filter.preheat(out_.zoom.x, 20);
                            if(switchBitsKalmanFilter_.bits.zy) zy_kalman_filter.preheat(out_.zoom.y, 20);
                            debugPrint("zxy_Kalmanfilter.preheat()\r\n");
                        }
                        if(switchBitsKalmanFilter_.bits.zx) out_.zoom.x = zx_kalman_filter.update(out_.zoom.x);
                        if(switchBitsKalmanFilter_.bits.zy) out_.zoom.y = zy_kalman_filter.update(out_.zoom.y);

#ifdef TOUCHPOINTFSM_EN_ARM_MATH
                        /**
                         * Armfly V5 DSP P154
                         * 10.3
                         *  平方根sqrt 浮点数的平方根计算只需调用一条浮点指令即可，而定点数的计算要稍显麻烦。
                         * 10.3.1 arm_sqrt_f32 对于CM4带FPU的处理器来说，浮点数的平方根求解很简单，只需调用指令__sqrtf，仅需要14个时钟周期就可以完成。
                         * 函数定义如下（在arm_math.h里面）： static __INLINE arm_status arm_sqrt_f32(float32_t in, float32_t * pOut)
                         */
                        arm_sqrt_f32(out_.zoom.x * out_.zoom.x + out_.zoom.y * out_.zoom.y, &out_.zoom.h);
                        out_.zoom.h /= M_SQRT2;       // 归一化斜边缩放比
#else
                        out_.zoom.h = sqrt(out_.zoom.x * out_.zoom.x + out_.zoom.y * out_.zoom.y);
                        out_.zoom.h /= M_SQRT2;       // 归一化斜边缩放比
#endif

                        /* 卡尔曼滤波 */
                        if(state_2_begin_) {
                            if(switchBitsKalmanFilter_.bits.zh) zh_kalman_filter.preheat(out_.zoom.h, 20);
                            debugPrint("zh_Kalmanfilter.preheat()\r\n");
                        }
                        if(switchBitsKalmanFilter_.bits.zh) out_.zoom.h = zh_kalman_filter.update(out_.zoom.h);

                        /* 清除 state_2 开始状态标记 */
                        state_2_begin_ = false;

                        /* 更新 last_points */
                        last_ = act;
                        /* 更新输出状态 */
                        out_.pressed = act.pressed;

                        /* 归一化输出, 方便分析波形噪声和数字滤波器的效果 */
//                         debugPrint("[TOUCH_INFO] point(x,y) | zoom(x,y,h) : %f, %f, %f, %f, %f\r\n",
//                                (float)out_.point.x / area_w_, (float)out_.point.y / area_h_,
//                                out_.zoom.x, out_.zoom.y, out_.zoom.h);

                        /* 归一化后乘以100输出 */
                         debugPrint("[TOUCH_INFO] point(x,y) | zoom(x,y,h) : %f, %f, %f, %f, %f\r\n",
                                (float)out_.point.x / area_w_ * 100, (float)out_.point.y / area_h_ * 100,
                                out_.zoom.x * 100, out_.zoom.y * 100, out_.zoom.h * 100);
                    }
                }
                else
                {
                    state_ = STATE_0;
                    event_ = EVENT_2_TO_0;
                    state_loop_counter++;
                }
            }
            break;
            }
        }
        break;
        }
    }
    return out_;
}

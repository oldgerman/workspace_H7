/**
  ******************************************************************************
  * @file        kalmanfilter.h
  * @author      OldGerman
  * @created on  2023年12月26日
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
#ifndef FILTER_KALMANFILTER_H_
#define FILTER_KALMANFILTER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported types ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

class kalmanFilter
{
public:
    typedef struct
    {
        float p_last; // 上次估算协方差 初始化值为0.02
        float p_act;  // 当前估算协方差 初始化值为0
        float out;    // 卡尔曼滤波器输出 初始化值为0
        float kg;     // 卡尔曼增益 初始化值为0
        float *q;     // 过程噪声协方差 初始化值为0.003 // q控制误差
        float *r;     // 观测噪声协方差 初始化值为0.03  // r控制响应速度
    } Attr_t;         // Kalman Filter parameter

    Attr_t attr_;        // 当前attr_
    Attr_t backup_attr_; // 备份attr_

    float q_;
    float r_;

    kalmanFilter(Attr_t attr = {
                     .p_last = 0.02,
                     .p_act = 0,
                     .out = 0,
                     .kg = 0,
                     .q = nullptr,
                     .r = nullptr,
                 })
    {
        attr_ = attr;
        if (!attr_.q || !attr_.r)
        {
            /* 默认值 q_ = 0.003 r_ = 0.003 适用 50Hz 捏合手势坐标滤波 */
            q_ = 0.003;
            r_ = 0.003;
            attr_.q = &q_;
            attr_.r = &r_;
        }
        else
        {
            /* 有初始值则保存，可用于reset() */
            q_ = *attr.q;
            r_ = *attr.r;
        }
        /* 备份 attr_ */
        backup_attr_ = attr_;
    }

    ~kalmanFilter(){};

    /**
     * @brief  重载滤波器初始值
     * @param  None
     * @retval None
     */
    void reset()
    {
        attr_ = backup_attr_;
    }

    /**
     * @brief       预热滤波器
     * @param input 滤波前的值
     * @param count 预热次数
     * @return      滤波后的值
     */
    float preheat(float input, uint16_t count)
    {
        if (count > 1)
        {
            count -= 1;
            for (uint16_t i = 0; i < count; i++)
            {
                update(input);
            }
        }
        return update(input);
    };
    /**
     * @brief       更新滤波器
     * @param input 滤波前的值
     * @return      滤波后的值
     */
    float update(float input)
    {
        // 预测协方差方程：k时刻系统估算协方差 = k-1时刻的系统协方差 + 过程噪声协方差
        attr_.p_act = attr_.p_last + *attr_.q;
        // 卡尔曼增益方程：卡尔曼增益 = k时刻系统估算协方差 / （k时刻系统估算协方差 + 观测噪声协方差）
        attr_.kg = attr_.p_act / (attr_.p_act + *attr_.r);
        // 更新最优值方程：k时刻状态变量的最优值 = 状态变量的预测值 + 卡尔曼增益 * （测量值 - 状态变量的预测值）
        attr_.out = attr_.out + attr_.kg * (input - attr_.out); // 因为这一次的预测值就是上一次的输出值
        // 更新协方差方程: 本次的系统协方差付给 attr_.p_last 为下一次运算准备
        attr_.p_last = (1 - attr_.kg) * attr_.p_act;
        return attr_.out;
    }
};

#endif /* FILTER_KALMANFILTER_H_ */

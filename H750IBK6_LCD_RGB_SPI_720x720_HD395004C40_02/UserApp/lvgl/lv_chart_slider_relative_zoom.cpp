/**
  ******************************************************************************
  * @file        lv_example_chart_5_test.cpp
  * @author      OldGerman
  * @created on  2023年12月19日
  * @brief       修改 LVGL 官方示例 lv_example_chart5.c
  *              实现相对图表显式区中心的x轴和y轴缩放
  *              全网找不到这样的实现，我人已经测试麻了
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
#include "common_inc.h"
#include "lv_conf.h"
#include "lv_user_app.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#if LV_USE_CHART && LV_USE_SLIDER && LV_BUILD_EXAMPLES

/* 绘制图表长宽 */
#define MY_CHART_W 500
#define MY_CHART_H 500

#define 避免计算缩放比的累计误差

/* Private macro -------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static lv_obj_t * chart; // 全局 lv_chart 对象指针
static volatile lv_chart_t * pd_chart = (lv_chart_t *)chart; // 方便调试器查看 lv_chart对象成员值, volatile 告诉编译器不要将其优化掉
static lv_chart_series_t * ser; // 指向“图表”上数据系列的指针

static lv_style_t chart_base;          // 图表样式
static lv_style_t chart_div_line_base; // 图表分割线样式

static uint32_t pcnt = sizeof(ecg_sample) / sizeof(ecg_sample[0]);     // 统计数据点个数

static bool x_event_released = true;
static bool y_event_released = true;

/* Private function prototypes -----------------------------------------------*/
/* Function implementations --------------------------------------------------*/

static void slider_x_event_released_cb(lv_event_t * e){
    x_event_released = true;
}
static void slider_y_event_released_cb(lv_event_t * e){
    y_event_released = true;
}

static void slider_x_event_value_changed_cb(lv_event_t * e)
{
#if 1
    /* 中心相对缩放 */
    lv_obj_t* obj = lv_event_get_target(e);
    int32_t v = lv_slider_get_value(obj);

    lv_coord_t scroll_left_0 = lv_obj_get_scroll_left(chart);
    lv_coord_t scroll_right_0 = lv_obj_get_scroll_right(chart);

    const float disp_focus_percent = 0.5f;  // 图表显示区的缩放焦点偏移，范围 0~100%
    //configASSERT_disp_focus_percent();      // TODO: 约束焦点范围，使浮点计算结果不为 nan

#ifdef 避免计算缩放比的累计误差
    /* 只要没有松开 slider，就使用本轮第一次按下slider执行回调时计算的缩放比，避免计算缩放比的累计误差
     * 串口调试信息输出示例：
        scroll_left_percent_0: 0.390991
        scroll_left_percent_0: 0.390991
        scroll_left_percent_0: 0.390991
        scroll_left_percent_0: 0.390991
        scroll_left_percent_0: 0.390991
        scroll_left_percent_0: 0.390991
        scroll_left_percent_0: 0.390991
        scroll_left_percent_0: 0.390991
     */
    static float scroll_left_percent_0;
    if(x_event_released == true) {
        scroll_left_percent_0 = ((float)MY_CHART_W * disp_focus_percent + scroll_left_0) / (scroll_left_0 + scroll_right_0 + MY_CHART_W); // 缩放前的左偏移百分比
        x_event_released = false;
    }
#else
    /* 每次回调都以上一次的缩放重新计算缩放比，缩放比的累计计算误差随本函数回调次数而增加
     * 串口调试信息输出示例：
        scroll_left_percent_0: 0.620370
        scroll_left_percent_0: 0.620147
        scroll_left_percent_0: 0.620053
        scroll_left_percent_0: 0.620046
        scroll_left_percent_0: 0.619942
        scroll_left_percent_0: 0.619654
        scroll_left_percent_0: 0.619610
     */
    float scroll_left_percent_0 = ((float)MY_CHART_W * disp_focus_percent + scroll_left_0) / (scroll_left_0 + scroll_right_0 + MY_CHART_W); // 缩放前的左偏移百分比
#endif
    lv_chart_set_zoom_x(chart, v);

    lv_coord_t scroll_left = lv_obj_get_scroll_left(chart);
    lv_coord_t scroll_right = lv_obj_get_scroll_right(chart);

    lv_coord_t x = (float)(scroll_left + scroll_right + MY_CHART_W) * scroll_left_percent_0 - (float)MY_CHART_W * disp_focus_percent;  // 除2 就是 整体波形的焦点 50% 缩放
    lv_obj_scroll_to_x(chart, x, LV_ANIM_OFF); // 禁用动画过渡，不然显式有过渡，还会浪费性能

    printf("scroll_left_percent_0: %f\r\n", scroll_left_percent_0);

#elif 1
    /* 中心绝对缩放 */
    lv_obj_t* obj = lv_event_get_target(e);
    int32_t v = lv_slider_get_value(obj);

    lv_chart_set_zoom_x(chart, v);
    lv_obj_scroll_to_x(
                chart,
                (lv_obj_get_scroll_left(chart) + lv_obj_get_scroll_right(chart)) / 2, // 除2 就是 整体波形的焦点 50% 缩放
                LV_ANIM_OFF             // 禁用动画过渡，不然显式有过渡，还会浪费性能
            );

#elif 1
    /* 玩坏的缩放 1 */

    lv_obj_t* obj = lv_event_get_target(e);
    int32_t v = lv_slider_get_value(obj);
    /* LVGL 的缩放范围是 256 对应 100%，2560 对应 1000%，宏 LV_IMG_ZOOM_NONE 就是 256，*/
    /* 改变缩放前的缩放倍率 */
//    uint16_t zoom_x0 = lv_chart_get_zoom_x(chart); // 这个时候滑块的值改变了，但 chart 的缩放倍率没变
    /* 改变缩放后的缩放倍率 */
    uint16_t zoom_x = v;  // 等效 uint16_t zoom_x = lv_chart_get_zoom_x(chart);
//
//    float zoom_x_percent_relative = (float)zoom_x / (float)zoom_x0;   // x方向相对缩放百分比
    float zoom_x_percent_absolute = (float)zoom_x / LV_IMG_ZOOM_NONE; // x方向缩放绝对百分比

    // 缩放后，中心焦点的x轴偏移的正负(为了反方向scroll，还要取反一次)
    int16_t x_polarity = 1; // x
//    if(fabs(zoom_x_percent - 1) < 1e-6)
//        x_polarity = 1;

    // 缩放后，中心焦点的x轴偏移
//    int16_t x = ((float)MY_CHART_W * 0.5  // 680像素点的中心缩放点就是 50%
//    * zoom_x_percent
//    - (float)MY_CHART_W * 0.5) * x_polarity;
    int16_t x = ((float)MY_CHART_W * 0.5  // 680像素点的中心缩放点就是 50%
    * zoom_x_percent_absolute
    - (float)MY_CHART_W * 0.5) * x_polarity;

//    lv_coord_t coords[4]; // lv_coord_t 实际上是 int16_t 有符号类型
    lv_chart_set_zoom_x(chart, v);

//    coords[0] = lv_obj_get_scroll_right(chart);
//    coords[1] = lv_obj_get_scroll_left(chart);

    lv_obj_scroll_to_x(chart, x, LV_ANIM_OFF);
//
//    coords[2] = lv_obj_get_scroll_right(chart);
//    coords[3] = lv_obj_get_scroll_left(chart);
//
//    printf("Scroll: x: %d, z: %2.4f, r0: %d, l0: %d, r: %d, l: %d\r\n", x, zoom_x_percent_absolute, coords[0], coords[1], coords[2], coords[3]);


#else
    /* 玩坏的缩放 2 */
    lv_obj_t * obj = lv_event_get_target(e);
    int32_t v = lv_slider_get_value(obj);
    // 这个时候滑块的值改变了，但 chart 的点没有改变
    /* 改变缩放前的缩放倍率 */
    uint16_t zoom_x0 = lv_chart_get_zoom_x(chart);
    /* 改变缩放后的缩放倍率 */
    uint16_t zoom_x = v;            // 等效 uint16_t zoom_x = lv_chart_get_zoom_x(chart);
    // 偏移x起始索引，实现当前显式界面中心缩放
    int16_t x_start_point =
            (float)pcnt * 0.5       // 1000索引点的中心缩放点就是 50%
            -
            (float)MY_CHART_W * 0.5  // 680像素点的中心缩放点就是 50%
            /
            (((float)MY_CHART_W * (float)zoom_x / LV_IMG_ZOOM_NONE) / pcnt) // 多少像素对应一个数索引： 单位 pixel/index
            ;

    lv_chart_set_x_start_point(chart, ser, x_start_point);
    // 设置缩放
    lv_chart_set_zoom_x(chart, v);

//    printf("Zoom: x_1: %d, x_2: %d\r\n", x_1, x_2); // Zoom: x_1: 0, x_2: 0 都是0，服了
//
//    lv_chart_get_pressed_point(chart);
#endif

    /* 打印 y 方向缩放的调试信息 */
     {
         lv_coord_t ret[2];
         ret[0] = lv_obj_get_scroll_top(chart);
         ret[1] = lv_obj_get_scroll_bottom(chart);

         printf("Scroll: top: %d, bottom: %d\r\n", ret[0], ret[1]);
     }
}

static void slider_y_event_cb(lv_event_t * e)
{
#if 1
    /* 中心相对缩放 */
    lv_obj_t* obj = lv_event_get_target(e);
    int32_t v = lv_slider_get_value(obj);

    lv_coord_t scroll_top_0 = lv_obj_get_scroll_top(chart);
    lv_coord_t scroll_bottom_0 = lv_obj_get_scroll_bottom(chart);

    const float disp_focus_percent = 0.5f;  // 图表显示区的缩放焦点偏移，范围 0~100%
    //configASSERT_disp_focus_percent();      // TODO: 约束焦点范围，使浮点计算结果不为 nan

#ifdef 避免计算缩放比的累计误差
    /* 只要没有松开 slider，就使用本轮第一次按下slider执行回调时计算的缩放比，避免计算缩放比的累计误差
     * 串口调试信息输出示例：
        scroll_top_percent_0: 0.390991
        scroll_top_percent_0: 0.390991
        scroll_top_percent_0: 0.390991
        scroll_top_percent_0: 0.390991
        scroll_top_percent_0: 0.390991
        scroll_top_percent_0: 0.390991
        scroll_top_percent_0: 0.390991
        scroll_top_percent_0: 0.390991
     */
    static float scroll_top_percent_0;
    if(y_event_released == true) {
        scroll_top_percent_0 = ((float)MY_CHART_H * disp_focus_percent + scroll_top_0) / (scroll_top_0 + scroll_bottom_0 + MY_CHART_H); // 缩放前的左偏移百分比
        y_event_released = false;
    }
#else
    /* 每次回调都以上一次的缩放重新计算缩放比，缩放比的累计计算误差随本函数回调次数而增加
     * 串口调试信息输出示例：
        scroll_top_percent_0: 0.620370
        scroll_top_percent_0: 0.620147
        scroll_top_percent_0: 0.620053
        scroll_top_percent_0: 0.620046
        scroll_top_percent_0: 0.619942
        scroll_top_percent_0: 0.619654
        scroll_top_percent_0: 0.619610
     */
    float scroll_top_percent_0 = ((float)MY_CHART_H * disp_focus_percent + scroll_top_0) / (scroll_top_0 + scroll_bottom_0 + MY_CHART_H); // 缩放前的左偏移百分比
#endif
    lv_chart_set_zoom_y(chart, v);

    lv_coord_t scroll_top = lv_obj_get_scroll_top(chart);
    lv_coord_t scroll_bottom = lv_obj_get_scroll_bottom(chart);

    lv_coord_t y = (float)(scroll_top + scroll_bottom + MY_CHART_H) * scroll_top_percent_0 - (float)MY_CHART_H * disp_focus_percent;  // 除2 就是 整体波形的焦点 50% 缩放
    lv_obj_scroll_to_y(chart, y, LV_ANIM_OFF); // 禁用动画过渡，不然显式有过渡，还会浪费性能

    printf("scroll_top_percent_0: %f\r\n", scroll_top_percent_0);

#elif 1
    /* 中心绝对缩放 */
    {
        lv_obj_t* obj = lv_event_get_target(e);
        int32_t v = lv_slider_get_value(obj);

        lv_chart_set_zoom_y(chart, v);
        lv_obj_scroll_to_y(chart, (lv_obj_get_scroll_bottom(chart) + lv_obj_get_scroll_top(chart)) / 2, LV_ANIM_OFF);
    }
#endif

    /* 打印 x 方向缩放的调试信息 */
    {
        lv_coord_t ret[2];
        ret[0] = lv_obj_get_scroll_left(chart);
        ret[1] = lv_obj_get_scroll_right(chart);

        printf("Scroll: left: %d, right: %d\r\n", ret[0], ret[1]);
    }
}


/**
 * Display 1000 data points with zooming and scrolling.
 * See how the chart changes drawing mode (draw only vertical lines) when
 * the points get too crowded.
 */
void lv_chart_slider_relative_zoom(void)
{
    chart = lv_chart_create(lv_scr_act());

    /*Create a chart*/
    lv_style_init(&chart_base);
    lv_style_init(&chart_div_line_base);

    // Style for the base properties of a graph
    lv_style_set_radius(&chart_base, 0);
    lv_style_set_border_width(&chart_base, 1);
    lv_style_set_pad_all(&chart_base, 0);

    // Style for the base properties of the graph division lines
    ///
//    lv_style_set_text_font(&chart_div_line_base, &lv_font_montserrat_16);

    // 设置图表的坐标和尺寸
    lv_obj_align(chart, LV_ALIGN_CENTER, 0, 0);                                 // 对齐到屏幕中心坐标+偏移量(-30,-30)
    lv_obj_set_size(chart, MY_CHART_W, MY_CHART_W);

    // 设置图表类型：折线图
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);

    // 添加样式：图表样式、图表分割线样式
    lv_obj_add_style(chart, &chart_base, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(chart, &chart_div_line_base, LV_PART_TICKS | LV_STATE_DEFAULT);


    lv_chart_set_range(chart,
            LV_CHART_AXIS_PRIMARY_Y,
            -1000, 1000);                                                   // y方向最小最大值

    /*Do not display points on the data*/
    lv_obj_set_style_size(chart, 0, LV_PART_INDICATOR);                     // LV_PART_INDICATOR 指的是折线图和散点图（小圆圈或正方形）上的点。

    ser = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);

    lv_chart_set_point_count(chart, pcnt);                                  // 设置图表点数1000
    lv_chart_set_ext_y_array(chart, ser, (lv_coord_t *)ecg_sample);         // 数据点的值绑定图表点的y坐标


    /* 创建 lv_slider 对象 */
    lv_obj_t * slider;                                                      // 声明局部变量 slider 对象指针
    // 底部的滑块
    slider = lv_slider_create(lv_scr_act());                                // 构造 slider 实例对象 给 指针
    lv_slider_set_range(slider, LV_IMG_ZOOM_NONE, LV_IMG_ZOOM_NONE * 10);   // 设置滑块最大最小值为LVGL IMG 无缩放状态的 256 到 2560
    lv_obj_add_event_cb(slider,
            slider_x_event_value_changed_cb,                                // 绑定自定义事件回调
            LV_EVENT_VALUE_CHANGED,                                         // 事件为滑块值改变时触发
            NULL);                                                          // 指向事件中可用的任何自定义数据的指针

    lv_obj_add_event_cb(slider,
            slider_x_event_released_cb,                                     // 绑定自定义事件回调
            LV_EVENT_RELEASED,                                              // 事件为滑块保持按下时触发
            NULL);                                                          // 指向事件中可用的任何自定义数据的指针


    lv_obj_set_size(slider, 500, 20);                                       // 设置滑块的长宽
    lv_obj_align_to(slider, chart, LV_ALIGN_OUT_BOTTOM_MID, 0, 40);         // 对齐滑块到父级 chart 对象并y轴偏移40像素
    // 右边的滑块
    slider = lv_slider_create(lv_scr_act());
    lv_slider_set_range(slider, LV_IMG_ZOOM_NONE, LV_IMG_ZOOM_NONE * 10);
    lv_obj_add_event_cb(slider,
            slider_y_event_cb,                                              // 绑定自定义事件回调
            LV_EVENT_VALUE_CHANGED,                                         // 事件为滑块值改变时触发，比 PRESSING 触发高效
            NULL);                                                          // 指向事件中可用的任何自定义数据的指针
    lv_obj_add_event_cb(slider,
            slider_y_event_released_cb,                                     // 绑定自定义事件回调
            LV_EVENT_RELEASED,                                              // 事件为滑块保持按下时触发
            NULL);                                                          // 指向事件中可用的任何自定义数据的指针
    lv_obj_set_size(slider, 20, 500);
    lv_obj_align_to(slider, chart, LV_ALIGN_OUT_RIGHT_MID, 40, 0);
}

#endif

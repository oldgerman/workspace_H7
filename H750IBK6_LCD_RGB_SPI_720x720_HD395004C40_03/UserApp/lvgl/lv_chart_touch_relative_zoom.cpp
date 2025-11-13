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
#include "touchpad.h"
#include "arm_math.h"
#include "math.h"
#include "global.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define DEBUG_PRINT(...)
//#define DEBUG_PRINT  DEBUG_PRINT

/* Source: https://github.com/ankur219/ECG-Arrhythmia-classification/blob/642230149583adfae1e4bd26c6f0e1fd8af2be0e/sample.csv*/
/* 1000 个点 */
static lv_coord_t I_sample[1000];
static lv_coord_t V_sample[1000];

#if LV_USE_CHART && LV_USE_SLIDER && LV_BUILD_EXAMPLES

#define MY_CHART_W 580
#define MY_CHART_H 580
#define MY_LCD_W LCD_T_HD
#define MY_LCD_H LCD_T_VD

typedef struct {
    /* 图表长宽 */
    uint16_t w;
    uint16_t h;
    lv_align_t align; // 对齐方式

    /* 坐标 */
    lv_coord_t x;
    lv_coord_t y;

    lv_coord_t l;
    lv_coord_t r;
    lv_coord_t t;
    lv_coord_t b;

    /* 初始缩放倍率, 最大缩放倍率, 最小缩放倍率 */
    struct {
        struct{
            float init;
            float max;
            float min;
        }x;
        struct {
            float init;
            float max;
            float min;
        }y;
        struct {
            float init;
            float max;
            float min;
        }h;
    } zoom;
} myChartAttr_t;


myChartAttr_t myChartAttr = {
        .w = MY_CHART_W,
        .h = MY_CHART_H,
        .align = LV_ALIGN_CENTER,
        .x = 0,
        .y = 0,
        .l = (MY_LCD_W - MY_CHART_W)/2,
        .r = (MY_LCD_W + MY_CHART_W)/2,
        .t = (MY_LCD_H - MY_CHART_H)/2,
        .b = (MY_LCD_H + MY_CHART_H)/2,
        .zoom = {
            .x = { .init = 5, .max = 10, .min = 1 },
            .y = { .init = 5, .max = 10, .min = 1 },
            .h = { .init = 5, .max = 10, .min = 1 }
        },
};

static bool chart_event_released = true;

#define 避免计算缩放比的累计误差

/* Private macro -------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static lv_obj_t * chart; // 全局 lv_chart 对象指针
static volatile lv_chart_t * pd_chart = (lv_chart_t *)chart; // 方便调试器查看 lv_chart对象成员值, volatile 告诉编译器不要将其优化掉
static lv_chart_series_t * ser1; // 指向“图表”上数据系列的指针
static lv_chart_series_t * ser2; // 指向“图表”上数据系列的指针

static lv_style_t chart_base;          // 图表样式
static lv_style_t chart_div_line_base; // 图表分割线样式

static uint32_t pcnt = sizeof(I_sample) / sizeof(I_sample[0]);     // 统计数据点个数


static lv_obj_t * focusCir; // 缩放焦点的圆
/* Private function prototypes -----------------------------------------------*/
/* Function implementations --------------------------------------------------*/
static void my_touch_zoom_cb(lv_event_t * e);

static void chart_event_released_cb(lv_event_t * e)
{
    chart_event_released = true;
    DEBUG_PRINT("[chart_event_released_cb]\r\n");
}

static void chart_event_scroll_cb(lv_event_t * e){
    DEBUG_PRINT("[chart_event_scroll_cb]\r\n");
}

static void chart_event_pressing_cb(lv_event_t * e){
    my_touch_zoom_cb(e);
//    DEBUG_PRINT("[chart_event_pressing_cb]\r\n");
}

static void chart_event_gesture_cb(lv_event_t * e)
{
    DEBUG_PRINT("[chart_event_gesture_cb]\r\n");
}

bool frist_in_chart = true;

static void my_touch_zoom_cb(lv_event_t * e) {
    osStatus_t osStatus;
//    uint8_t ucQueueCount = osMessageQueueGetCount(xMsgQueueTouched);
//    DEBUG_PRINT("消息队列剩余消息数：%d\r\n", ucQueueCount);
    static TouchData_t lastTouchData;
    TouchData_t touchData;
    osStatus = osMessageQueueGet(xMsgQueueTouchData, &touchData, 0U, 0);   // wait for message

    if (osStatus == osOK) {
        if(frist_in_chart == true) {
//        if(chart_event_released == true) {
            lastTouchData = touchData;
            //假设当前缩放比率都是1，实际这个需要写计算出历史缩放倍率
            lastTouchData.zoom.x = 5.f;
            lastTouchData.zoom.y = 5.f;
            lastTouchData.zoom.h = 5.f;
            frist_in_chart = false;
        }

        /* 新的一轮需要使用新的focus */ // 这个得在触摸回调里实现
//        if(chart_event_released == true)

        // 焦点赋值
        lastTouchData.point = touchData.point;

        // 缩放倍率是一个增量，需要累乘历史倍率
        lastTouchData.zoom.x *= touchData.zoom.x;
        lastTouchData.zoom.y *= touchData.zoom.y;
        lastTouchData.zoom.h *= touchData.zoom.h;

        /* 中心相对缩放 */
//        lv_obj_t* obj = lv_event_get_target(e);

        /* 约束缩放焦点x坐标在图表区域内 */
        lastTouchData.point.x = constrain(lastTouchData.point.x, myChartAttr.l, myChartAttr.r);
        lastTouchData.point.y = constrain(lastTouchData.point.y, myChartAttr.t, myChartAttr.b);
        /* 约束缩放倍率 */
        lastTouchData.zoom.x = constrain(lastTouchData.zoom.x, 1, 10);
        lastTouchData.zoom.y = constrain(lastTouchData.zoom.y, 1, 10);
        lastTouchData.zoom.h = constrain(lastTouchData.zoom.h, 1, 10);

        /*归一化乘以100输出数据*/
//         printf("[TOUCH_CB] point(x,y) | zoom(x,y,s) : %f, %f, %f, %f, %f\r\n",
//                 (float)lastTouchData.point.x / myChartAttr.w * 100, (float)lastTouchData.point.y / myChartAttr.h * 100,
//                 lastTouchData.zoom.x * 100, lastTouchData.zoom.y * 100, lastTouchData.zoom.h * 100);                 //这三个缩放倍率是历史累乘量

        /* 计算LVGL缩放单位的缩放倍率 */
        int32_t x_zoom = lastTouchData.zoom.h * LV_IMG_ZOOM_NONE;
        int32_t y_zoom = lastTouchData.zoom.h * LV_IMG_ZOOM_NONE;

        lv_coord_t scroll_left_0 = lv_obj_get_scroll_left(chart);
        lv_coord_t scroll_right_0 = lv_obj_get_scroll_right(chart);

#if 1
        // 缩放焦点x坐标跟随双指焦点
        float disp_focus_x_percent = (float)(lastTouchData.point.x - myChartAttr.l) / (myChartAttr.w);  // 图表显示区的缩放焦点偏移，范围 0~100%
        //configASSERT_disp_focus_x_percent();      // TODO: 约束焦点范围，使浮点计算结果不为 nan
#else
        // 锁定缩放焦点为图表水平中心
        const float disp_focus_x_percent = 0.5f;
#endif

#if 1
        static float scroll_left_percent_0;
        if(chart_event_released == true) {
            scroll_left_percent_0 = ((float)myChartAttr.w * disp_focus_x_percent + scroll_left_0) / (scroll_left_0 + scroll_right_0 + myChartAttr.w); // 缩放前的左偏移百分比
        }
#else
        float scroll_left_percent_0 = ((float)myChartAttr.w * disp_focus_x_percent + scroll_left_0) / (scroll_left_0 + scroll_right_0 + myChartAttr.w); // 缩放前的左偏移百分比
#endif

        lv_chart_set_zoom_x(chart, x_zoom);

        lv_coord_t scroll_left = lv_obj_get_scroll_left(chart);
        lv_coord_t scroll_right = lv_obj_get_scroll_right(chart);

        lv_coord_t x = (float)(scroll_left + scroll_right + myChartAttr.w) * scroll_left_percent_0 - (float)myChartAttr.w * disp_focus_x_percent;  // 除2 就是 整体波形的焦点 50% 缩放
        lv_obj_scroll_to_x(chart, x, LV_ANIM_OFF); // 禁用动画过渡，不然显式有过渡，还会浪费性能

        DEBUG_PRINT("scroll_left_percent_0: %f\r\n", scroll_left_percent_0);

        /* 打印 y 方向缩放的调试信息 */
         {
             lv_coord_t ret[2];
             ret[0] = lv_obj_get_scroll_top(chart);
             ret[1] = lv_obj_get_scroll_bottom(chart);

             DEBUG_PRINT("Scroll: top: %d, bottom: %d\r\n", ret[0], ret[1]);
         }


        /* 中心相对缩放 */
        lv_coord_t scroll_top_0 = lv_obj_get_scroll_top(chart);
        lv_coord_t scroll_bottom_0 = lv_obj_get_scroll_bottom(chart);
#if 1
        // 缩放焦点y坐标跟随双指焦点
        float disp_focus_y_percent = (float)(lastTouchData.point.y - myChartAttr.t) / (myChartAttr.b);  // 图表显示区的缩放焦点偏移，范围 0~100%
        //configASSERT_disp_focus_y_percent();      // TODO: 约束焦点范围，使浮点计算结果不为 nan
#else
        // 锁定缩放焦点为图表垂直中心
        const float disp_focus_y_percent = 0.5f;
#endif

#if 1
        static float scroll_top_percent_0;
        if(chart_event_released == true) {
            scroll_top_percent_0 = ((float)myChartAttr.h * disp_focus_y_percent + scroll_top_0) / (scroll_top_0 + scroll_bottom_0 + myChartAttr.h); // 缩放前的左偏移百分比
        }
#else
        float scroll_top_percent_0 = ((float)myChartAttr.h * disp_focus_y_percent + scroll_top_0) / (scroll_top_0 + scroll_bottom_0 + myChartAttr.h); // 缩放前的左偏移百分比
#endif

        lv_chart_set_zoom_y(chart, y_zoom); // v 本来是 slider 的 value 范围 256~2560 步进100 ，对应 ZOOM 倍率 1~10

        lv_coord_t scroll_top = lv_obj_get_scroll_top(chart);
        lv_coord_t scroll_bottom = lv_obj_get_scroll_bottom(chart);

        lv_coord_t y = (float)(scroll_top + scroll_bottom + myChartAttr.h) * scroll_top_percent_0 - (float)myChartAttr.h * disp_focus_y_percent;  // 除2 就是 整体波形的焦点 50% 缩放
        lv_obj_scroll_to_y(chart, y, LV_ANIM_OFF); // 禁用动画过渡，不然显式有过渡，还会浪费性能

        DEBUG_PRINT("scroll_top_percent_0: %f\r\n", scroll_top_percent_0);

        // 待 x 和 y 判断完后才更改标记
        chart_event_released = false;

        /* 打印 x 方向缩放的调试信息 */
        {
            lv_coord_t ret[2];
            ret[0] = lv_obj_get_scroll_left(chart);
            ret[1] = lv_obj_get_scroll_right(chart);

            DEBUG_PRINT("Scroll: left: %d, right: %d\r\n", ret[0], ret[1]);
        }
#ifdef DEBUG_TOUCH_ZOOM_POINTS
        // 更新缩放焦点圆圈位置
        lv_obj_set_pos(focusCir , lastTouchData.point.x, lastTouchData.point.y);
#endif
    }
}


/**
 * @brief 参考 lv_example_chart_3
 *         自己的刻度字符串要想覆盖掉LVGL默认的数字字符串，得在 LV_EVENT_DRAW_PART_BEGIN 时绘制
 *
 * @param e
 */
static void chart_event_draw_part_begin_cb(lv_event_t * e)
{
#if 1
    lv_obj_draw_part_dsc_t * dsc = lv_event_get_draw_part_dsc(e);
    if(!lv_obj_draw_part_check_type(
        dsc,
        &lv_chart_class,
        LV_CHART_DRAW_PART_TICK_LABEL
        )) return;

    /* 重绘Y左方刻度字符串 */
    if(dsc->id == LV_CHART_AXIS_PRIMARY_Y && dsc->text) {
        const char * I_value[] = {"0.00", "250", "500", "750", "1.00", "1.25", "1.50", "1.75", "2.00", "2.25", "2.50"};
        const char * I_unit[] =  {"uA", "mA",  "mA",  "mA", "A",    "A",   "A",    "A", "A",    "A",   "A"};

        lv_snprintf(
            dsc->text,
            dsc->text_length,
            "%s%s",
            I_value[(dsc->value + 1000) / 200], I_unit[(dsc->value + 1000) / 200]); // 注意根据 dsc-value 来定位刻度字符的话，需要放缩到字符串数组下标，否则越界访问
    }

    /* 重绘Y右方刻度字符串 */
    if(dsc->id == LV_CHART_AXIS_SECONDARY_Y && dsc->text) {
        const char * I_value[] = {"0.00", "500", "1.00", "1.50", "2.00", "2.50", "3.00", "3.50", "4.00", "4.50", "5.00"};
        const char * I_unit[] =  {"mV",    "mV",   "mV",   "mV",    "V",    "V",    "V",    "V",    "V",    "V",    "V"};

        lv_snprintf(
            dsc->text,
            dsc->text_length,
            "%s%s",
            I_value[(dsc->value + 1000) / 200], I_unit[(dsc->value + 1000) / 200]); // 注意根据 dsc-value 来定位刻度字符的话，需要放缩到字符串数组下标，否则越界访问
    }

    /* 重绘X下方刻度字符串 */
    if(dsc->id == LV_CHART_AXIS_PRIMARY_X && dsc->text) {

        // 缩放比小于 2 时，label文本之间的距离太小，需要减少刻度数量防止刻度字符过近
        uint16_t _zoom = lv_chart_get_zoom_x(chart);
        if(_zoom < LV_IMG_ZOOM_NONE * 2) {
            // lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_X, 10, 5, 5 + 1, 3, true, 40);  // 不能实时调用此API，会不显示表的数据线
            ((lv_chart_t *)chart)->tick[1].major_cnt = 5 + 1; // LV_CHART_AXIS_PRIMARY_X 是 tick[1]
        }
        else {
            ((lv_chart_t *)chart)->tick[1].major_cnt = 10 + 1; // LV_CHART_AXIS_PRIMARY_X 是 tick[1]
        }


        // yyyy-mm-dd HH:mm:ss.SSSSSS
        // 2020-02-06 01:58:00.000020"
        // 时分秒
        const char * time_HHmmss[] = {"00:00:10", "00:00:20", "00:00:30", "00:00:40", "00:00:50", "00:01:00",
                                       "00:01:10", "00:01:20", "00:01:30", "00:01:40", "00:02:50", "00:05:00"};
        // ms、us
        const char * time_SSSSSS[] = {"000.100","100.100","200.100","300.100","400.100","500.100",
                                       "600.100","700.100","800.100","900.100","000.100","100.100"};

        // 操作刻度的label描述符使文本居中对齐
        dsc->label_dsc->align = LV_TEXT_ALIGN_CENTER;

//        dsc->text_length = 20; // 将默认的临时label文本缓冲区大小从16个字符增加到20个字符【不能在这里修改，会内存泄漏。只能改lv_chart.c的宏LV_CHART_LABEL_MAX_TEXT_LENGTH】

        lv_snprintf(
            dsc->text,
            dsc->text_length,
            "%s\n%s",
            time_HHmmss[dsc->value], time_SSSSSS[dsc->value]); // 注意根据 dsc-value 来定位刻度字符的话，需要放缩到字符串数组下标，否则越界访问

    }
#endif
}

/**
 * Display 1000 data points with zooming and scrolling.
 * See how the chart changes drawing mode (draw only vertical lines) when
 * the points get too crowded.
 */
void lv_chart_touch_relative_zoom(void)
{
    /* 填充测试波形数据 */
    for(uint32_t i=0; i<pcnt; i++) {
//        I_sample[i] = fabs(arm_sin_f32(2*3.1415926f*10*i/1000)*100 * arm_cos_f32(2*3.1415926f*10*i/10/1000)*10) * 2 - 1000;
        I_sample[i] = arm_sin_f32(2*3.1415926f*10*i/1000)*100 * arm_cos_f32(2*3.1415926f*10*i/10/1000)*10;
        V_sample[i] = -1.f * I_sample[i] * 0.1 + 500;
    }


    /* 创建 lv_chart 对象 */
    chart = lv_chart_create(lv_scr_act());

    /*Create a chart*/
    lv_style_init(&chart_base);
    lv_style_init(&chart_div_line_base);

    // Style for the base properties of a graph
    lv_style_set_radius(&chart_base, 0);
    lv_style_set_border_width(&chart_base, 1);
    lv_style_set_pad_all(&chart_base, 0);

    // 设置图表的坐标和尺寸
    lv_obj_align(chart, myChartAttr.align, myChartAttr.x, myChartAttr.y);
    lv_obj_set_size(chart, myChartAttr.w, myChartAttr.h);

    // 设置图表类型：折线图
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
    // 设置数据线线宽
    lv_obj_set_style_line_width(chart, 1, LV_PART_ITEMS);

    // 添加样式：图表样式、图表分割线样式
    lv_obj_add_style(chart, &chart_base, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(chart, &chart_div_line_base, LV_PART_TICKS | LV_STATE_DEFAULT);

    // y方向最小最大值
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, -1000, 1000);
    lv_chart_set_range(chart, LV_CHART_AXIS_SECONDARY_Y, -1000, 1000);

    /*Do not display points on the data*/
    lv_obj_set_style_size(chart, 0, LV_PART_INDICATOR);                     // LV_PART_INDICATOR 指的是折线图和散点图（小圆圈或正方形）上的点。

    lv_chart_set_point_count(chart, pcnt);                                  // 设置图表点数1000

    ser1 = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_LIGHT_BLUE), LV_CHART_AXIS_PRIMARY_Y);    // 电流
    ser2 = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_ORANGE), LV_CHART_AXIS_SECONDARY_Y); // 电压

    lv_chart_set_ext_y_array(chart, ser1, (lv_coord_t *)I_sample);         // 电流 数据点的值绑定图表点的y坐标
    lv_chart_set_ext_y_array(chart, ser2, (lv_coord_t *)V_sample);         // 电压 数据点的值绑定图表点的y坐标

    lv_obj_add_event_cb(chart, chart_event_gesture_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_add_event_cb(chart, chart_event_released_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_add_event_cb(chart, chart_event_scroll_cb, LV_EVENT_SCROLL, NULL);
    lv_obj_add_event_cb(chart, chart_event_pressing_cb, LV_EVENT_PRESSING, NULL);
    lv_obj_add_event_cb(chart, chart_event_draw_part_begin_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);

    // 设置刻度
    lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_X, 10, 5, 10 + 1, 1, true, 40);

    lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_Y, 10, 5, 10 + 1, 1, true, 70);
    lv_chart_set_axis_tick(chart, LV_CHART_AXIS_SECONDARY_Y, 10, 5, 10 + 1, 1, true, 70);



    // 覆盖掉默认的水平3条分割线垂直5条分割线
    lv_chart_set_div_line_count(chart, 10 + 1, 10 + 1);

    lv_chart_set_zoom_x(chart, LV_IMG_ZOOM_NONE * 5);
    lv_chart_set_zoom_y(chart, LV_IMG_ZOOM_NONE * 5);
    lv_obj_scroll_to_x(chart, 1000, LV_ANIM_OFF);
    lv_obj_scroll_to_y(chart, 1000, LV_ANIM_OFF);

#ifdef DEBUG_TOUCH_ZOOM_POINTS
    focusCir = lv_obj_create(lv_scr_act());
    lv_obj_set_scrollbar_mode(focusCir , LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_size(focusCir , 20, 20);
    lv_obj_set_style_arc_color(focusCir, lv_palette_main(LV_PALETTE_BLUE), LV_PART_MAIN); //设置圆弧颜色
    lv_obj_set_pos(focusCir , 360, 360);
//    lv_obj_set_style_bg_color(focusCir , lv_palette_lighten(LV_PALETTE_BLUE), 0);
    lv_obj_set_style_radius(focusCir , LV_RADIUS_CIRCLE, 0);
#endif

    // https://lvgl.100ask.net/8.2/overview/coords.html?highlight=%E8%BE%B9%E7%95%8C#id1
    // 可以使用以下函数获取边界框和内容区域的大小：

//    lv_coord_t w = lv_obj_get_width(chart);
//    lv_coord_t h = lv_obj_get_height(chart);
//    lv_coord_t content_w = lv_obj_get_content_width(chart);
//    lv_coord_t content_h = lv_obj_get_content_height(chart);
    //能不能搞一个DEBUG_PRINT在usb串口连上打开时，输出连上前DEBUG_PRINT的输出？
    DEBUG_PRINT("[Chart Size]: w = %d, h = %d, content_w = %d, content_h = %d\r\n", w, h, content_w, content_h);
}

#endif

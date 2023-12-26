/**
  ******************************************************************************
  * @file        lv_example_chart_5_test.cpp
  * @author      OldGerman
  * @created on  2023年12月19日
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
#include "common_inc.h"
#include "lv_conf.h"
#include "lv_user_app.h"

/* Private typedef -----------------------------------------------------------*/
static lv_obj_t* dropdown;
/* Private define ------------------------------------------------------------*/

// quick call for axis ticks function as most parameters stay the same
#define X_AXIS_TICKS(major, minor)  lv_chart_set_axis_tick(graph, LV_CHART_AXIS_PRIMARY_X, 7, 3, major, minor, true, 20)


/* Private macro -------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Function implementations --------------------------------------------------*/

#define MY_CHART_W 500
#define MY_CHART_H 500

#if LV_USE_CHART && LV_USE_SLIDER && LV_BUILD_EXAMPLES

static lv_obj_t * chart; // 全局 lv_chart 对象指针
static volatile lv_chart_t * pd_chart = (lv_chart_t *)chart; // 方便调试器查看 lv_chart对象成员值, volatile 告诉编译器不要将其优化掉
static lv_chart_series_t * ser; // 指向“图表”上数据系列的指针
static uint32_t pcnt = sizeof(ecg_sample) / sizeof(ecg_sample[0]);     // 统计数据点个数


static lv_style_t chart_base;
static lv_style_t chart_div_line_base;

static void update_data_points(lv_event_t* e)
{
    lv_obj_t* obj = lv_event_get_target(e);
    uint16_t data_point_count = 0;
    char dropdown_str[4] = { '\0' };
    lv_dropdown_get_selected_str(obj, dropdown_str, 4);
    data_point_count = (uint16_t)strtod(dropdown_str, NULL);

    lv_chart_set_point_count(chart, data_point_count);
    //lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_X, 0, data_point_count - 1);

    if (data_point_count <= 25)
    {
        lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_X, 7, 3, data_point_count, 1, true, 65);
    }
    else
    {
        lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_X, 7, 3, 25, data_point_count / 25, true, 65);
    }

    lv_chart_refresh(chart);
}

void lv_example_chart_5_test_x_ticks()
{
    chart = lv_chart_create(lv_scr_act());

    lv_style_init(&chart_base);
    lv_style_init(&chart_div_line_base);

    // Style for the base properties of a graph
    lv_style_set_radius(&chart_base, 0);
    lv_style_set_border_width(&chart_base, 1);
    lv_style_set_pad_all(&chart_base, 0);

    // Style for the base properties of the graph division lines
    lv_style_set_text_font(&chart_div_line_base, &lv_font_montserrat_16);

    lv_obj_set_pos(chart, 80, 10);
    lv_obj_set_size(chart, 800, 425);
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
    lv_obj_add_style(chart, &chart_base, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(chart, &chart_div_line_base, LV_PART_TICKS | LV_STATE_DEFAULT);
    lv_chart_set_div_line_count(chart, 3, 2);
    lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_Y, 7, 3, 3, 2, true, 65);
    lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_X, 7, 3, 10, 1, true, 65);
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, -100, 100);
    ser = lv_chart_add_series(chart, lv_color_hex(0xff0000), LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_set_point_count(chart, 10);
    lv_chart_set_ext_y_array(chart, ser, (lv_coord_t*)ecg_sample);

    dropdown = lv_dropdown_create(lv_scr_act());
    lv_obj_set_pos(dropdown, 805, 462);
    lv_obj_set_size(dropdown, 95, 35);
    lv_dropdown_set_symbol(dropdown, LV_SYMBOL_DOWN);
    lv_dropdown_set_options_static(dropdown, "5\n10\n15\n20\n25\n50\n75\n100");
    lv_obj_add_event_cb(dropdown, update_data_points, LV_EVENT_VALUE_CHANGED, NULL);
    lv_event_send(dropdown, LV_EVENT_VALUE_CHANGED, NULL);
}

#endif

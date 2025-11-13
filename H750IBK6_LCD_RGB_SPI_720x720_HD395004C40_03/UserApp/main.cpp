/**
  ******************************************************************************
  * @file        main.cpp
  * @author      OldGerman
  * @created on  Jan 7, 2023
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
//#include "arm_math.h"

#include "tim.h"
#include "dma2d.h"
#include "hal_stm_lvgl/tft/tft.h"
#include "hal_stm_lvgl/touchpad/touchpad.h"

#include "I2C_Wrapper.h"

#include "lv_user_app.h"

#include "bsp_emmc.h"
#include "memoryPool.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported constants -------------------------- ------------------------------*/
/**
 *  运行 lv_demo_widgets 时任务栈内存最大开销 ledTask : 579/1024  56%
 *  分配 512 * 4 时，内存不够，在执行uxListRemove函数时进hardfault报非对齐访问错误
 *  分配 1024 * 4 时，此现象消失
 */
const uint32_t ledTaskStackSize = 1024 * 4;
const osThreadAttr_t ledTask_attributes = {
    .name = "ledTask",
    .stack_size = ledTaskStackSize,
    .priority = (osPriority_t) osPriorityLow,
};

/* Private constants ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
osThreadId_t ledTaskHandle;

/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Function implementations --------------------------------------------------*/

/* Thread definitions */
void threadLedUpdate(void* argument){
    TickType_t xLastWakeTime;
//    const TickType_t xFrequency = 1000;
    /* 获取当前的系统时间 */
    xLastWakeTime = xTaskGetTickCount();

    for(;;){
        /* 翻转开发板引脚 */
//      HAL_GPIO_TogglePin(VOUT_EN_GPIO_Port, VOUT_EN_Pin);

        /* 打印时间节拍 */
//        printf("[led_task] sysTick %ld ms\r\n", xTaskGetTickCount());

        /* arm math 单精度硬件浮点测试 */
//      float data[3];
//      data[0] = arm_sin_f32(3.1415926/6); // sin(PI/6)
//      data[1] = arm_sin_f32(3.1415926/1); // sin(PI/1)
//      data[2] = arm_sin_f32(3.1415926/3); // sin(PI/3)
//      printf("[sin] 30°= %.6f, 45°= %.6f, 60°= %.6f\r\n", data[0], data[1], data[2]);

//        vTaskDelayUntil(&xLastWakeTime, xFrequency);

      lv_task_handler();  // LVGL进程(会调用注册了的GT911触摸扫描)
      vTaskDelayUntil(&xLastWakeTime, 20);  // GT911触摸屏扫描间隔不能小于10ms，建议设置为20ms
    }
}

void ledUpdateInit()
{
	ledTaskHandle = osThreadNew(threadLedUpdate, nullptr, &ledTask_attributes);
}

void Main()
{
    /* 初始化具有互斥锁的I2C对象 */
    FRToSI2C4.FRToSInit();

	/* 初始化动态内存对象的内存池 */
	DRAM_Init();

	/* 启用统计CPU利用率的定时器中断 */
	HAL_TIM_Base_Start_IT(&htim7);

    /* 初始化一些通信，USB-CDC/VCP/WIFI等 */
    InitCommunication();

    /* 初始化LED时间片任务 */
    ledUpdateInit();


    /* 初始化LVGL：包括初始化屏驱NV3052c、触摸芯片GT911、注册LTDC垂直消隐中断 */
    lv_init();
    tft_init();
    touchpad_init();

    /* LVGL示例 */
//    lv_demo_benchmark();
//    lv_demo_music();
//    lv_demo_stress(); //测试 OK 2/19/34/47/50FPS都出现过，一半以上都是50FPS
//    lv_demo_widgets();

    /* LVGL控件示例：显示1000个点并支持缩放滚动 */
//    lv_example_chart_5_test_x_ticks();
//    lv_chart_slider_relative_zoom();
      lv_chart_touch_relative_zoom();
    /* LVGL控件测试：绘制RGB渐变色轮 */
//    {
//        lv_obj_t * obj;
//        uint16_t lv_demo_stress_time_step   = 50;
//        obj = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, 50);
//        lv_obj_set_size(obj, LV_HOR_RES, LV_VER_RES);
//        lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
//        lv_obj_t * t = lv_tabview_add_tab(obj, "First");
//
//        t = lv_tabview_add_tab(obj, "Second");
//
//
//        lv_obj_t * c = lv_colorwheel_create(t, true);
//
//        lv_obj_set_style_arc_width(c, 40, LV_PART_MAIN);  // 弧形宽度
//
//        lv_obj_set_size(c,  600, 600);
//        //                  c = lv_led_create(t, NULL);
//        //                  lv_obj_set_pos(c, 160, 20);
//        t = lv_tabview_add_tab(obj, LV_SYMBOL_EDIT " Edit");
//        t = lv_tabview_add_tab(obj, LV_SYMBOL_CLOSE);
//
//        lv_tabview_set_act(obj, 1, LV_ANIM_ON);
//        //auto_del(obj, lv_demo_stress_time_step * 5 + 30);
//    }

}

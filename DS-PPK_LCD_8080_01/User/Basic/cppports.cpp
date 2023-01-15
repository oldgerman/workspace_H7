/*
 * cppports.cpp
 *
 *  Created on: May 16, 2022
 *      Author: OldGerman
 */

//#include ”/home/wit/code/xx.h”  //Linux下的绝对路径
//#include “F:/litao/code/xx.h"   //Windows下的绝对路径
//#include ”../lcd/lcd.h”         //相对路径，..表示当前目录的上一层目录
//#include ”./lcd.h”             //相对路径，.表示当前目录
//#include ”lcd.h”               //相对路径，当前文件所在的目录


#include "cppports.h"
#include "bsp.h"

extern "C" {
// 测试皂片
//#include "RGB565_240x160.h"
//#include "240x160.h"
//#include "480x320_MB1261_FAN_OUT.h"
//#include "img1.h"

#include "GUI.h"
#include "dma2d.h"
#include "../GUI/lvgl/lvgl.h"
#include "../GUI/lvgl/demos/lv_demos.h"
#include "hal_stm_lvgl/tft/tft.h"
#include "hal_stm_lvgl/touchpad/touchpad.h"

#include "usbd_cdc_if.h"	//usb_printf()
}

#ifndef DBG_PRINT
#if 1  //< Change 0 to 1 to open debug macro and check program debug information
#define DBG_PRINT usb_printf
#else
#define DBG_PRINT(...)
#endif
#endif

static void timerCallBack(){
}

void btA_CLICKED_func(){
}

void btB_CLICKED_func(){
}

void btA_LONG_PRESSED_func(){
}

void btB_LONG_PRESSED_func(){
}


void setup(){
	bsp_Init();

	LCD_ClrScr(0xfff);  		/* 清屏，背景蓝色 */
	HAL_Delay(500);				/* 延时0.5s 以指示全屏蓝色 */

	lv_init();
	tft_init();
	touchpad_init();

//    lv_demo_benchmark();
//    lv_demo_music();
//    lv_demo_stress();
	lv_demo_widgets();
}

void loop(){
	while (1)
	{
		lv_task_handler();
		static uint32_t oldtime = HAL_GetTick();
		if(waitTime(&oldtime, 250)) {
			HAL_GPIO_TogglePin(MUX_A_GPIO_Port, MUX_A_Pin);
			DBG_PRINT("SysTick: %d , Hello World!\r\n" , HAL_GetTick());
		}
	}
}

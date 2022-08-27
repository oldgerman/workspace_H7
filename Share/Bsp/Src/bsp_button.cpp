/*
 * bsp_button.cpp
 *
 *  Created on: 2022年8月27日
 *      Author: OldGerman
 *
 *  Code modification from this link:
 *  	https://github.com/FASTSHIFT/WatchX/blob/fe0ef4001e6fcd41767319e69e581285bbaa1bf9/WatchX/WatchX/BSP/BSP_Button.cpp
 */

#include "bsp.h"
#ifdef EN_BSP_BUTTON
#include "ButtonEvent.h"
#include "cppports.h"
extern void btA_CLICKED_func();
extern void btB_CLICKED_func();
extern void btA_LONG_PRESSED_func();
extern void btB_LONG_PRESSED_func();
/*实例化按键对象*/
ButtonEvent btA;              	//KEYA键
ButtonEvent btB;            	//KEYB键

/**
  * @brief  按键事件回调处理
  * @param  btn:按键对象地址
  * @param  event:事件类型
  * @retval 无
  */
static void Button_EventHandler(ButtonEvent* btn, int event)
{
	if (event == ButtonEvent::EVENT_SHORT_CLICKED)
//	if(event == ButtonEvent::EVENT_PRESSED || event == ButtonEvent::EVENT_LONG_PRESSED_REPEAT)
	{
        if(btn == &btA)
        {
        	btA_CLICKED_func();
        }
        if(btn == &btB)
        {
        	btB_CLICKED_func();
        }
	}
	if(event == ButtonEvent::EVENT_LONG_PRESSED || event == ButtonEvent::EVENT_LONG_PRESSED_REPEAT)
	{
        if(btn == &btA)
        {
        	btA_LONG_PRESSED_func();
        }
        if(btn == &btB)
        {
        	btB_LONG_PRESSED_func();
        }
	}

//	BSP_Printf("KEY %c: %s\r\n", (btn == &btA)?('A'):('B'), btn->event(event));
}

/**
  * @brief  按键初始化
  * @param  无
  * @retval 无
  */
void bsp_Button_Init()
{
	/* 引脚由CubeMX初始化 */
    /*关联事件*/
    btA.EventAttach(Button_EventHandler);
    btB.EventAttach(Button_EventHandler);
}

/**
  * @brief  按键监控更新
  * @param  无
  * @retval 无
  */
void bsp_Button_Update()
{
    btA.EventMonitor(HAL_GPIO_ReadPin(KEY_A_GPIO_Port, KEY_A_Pin) == GPIO_PIN_SET);
    btB.EventMonitor(HAL_GPIO_ReadPin(KEY_B_GPIO_Port, KEY_B_Pin) == GPIO_PIN_RESET);
}
#endif



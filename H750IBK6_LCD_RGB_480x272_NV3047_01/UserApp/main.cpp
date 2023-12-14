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
#include "spi.h"
//
#include "lcd_rgb.h"
//
//
//#include "frtos_spi.h"
//using namespace ns_frtos_spi;
///* FRTOS_SPIBase类对象：SPI2_Base */
//uint8_t SPI2_RxBuf[FRTOS_SPIBase::sizeCmdOnly];
//uint8_t SPI2_TxBuf[FRTOS_SPIBase::sizeCmdOnly];
//
//FRTOS_SPIBase SPI2_Base(hspi2, SPI2_TxBuf, SPI2_RxBuf, FRTOS_SPIBase::sizeCmdOnly);
//
//const uint32_t SPI2_Cmd_PSC  = SPI_BAUDRATEPRESCALER_16;    //7.5MHz
//
//FRTOS_SPICmd SPI2_Cmd(
//        &SPI2_Base,
//        SPI2_CS_GPIO_Port,
//        SPI2_CS_Pin,
//        SPI2_Cmd_PSC,
//        SPI_PHASE_1EDGE,
//        SPI_POLARITY_LOW);
//
//void WriteCommand(uint8_t data)
//{
//    spi_transaction_t trans = {
//            .instr = data,
//            .instr_bytes = 1,                           /* 命令字节数 */
//            .cmd_transfer_mode = TRANSMIT_MODE_POLL,
//    };
//    SPI2_Cmd.busTransferExtCmdAndData(&trans);
//}
//
//void WriteParameter(uint8_t data)
//{
//    spi_transaction_t trans = {
//            .instr = data,
//            .instr_bytes = 1,                           /* 命令字节数 */
//            .cmd_transfer_mode = TRANSMIT_MODE_POLL,
//    };
//    SPI2_Cmd.busTransferExtCmdAndData(&trans);
//}

void WriteCommand(uint16_t data)
{
    data = data | 0x000; // bit[8]: 0 表示写命令

    if(HAL_SPI_Transmit(&hspi2, (uint8_t *)(&data), 1, HAL_MAX_DELAY) != HAL_OK)
    //                                              ^ 对于 9bit 硬件 SPI 传输长度取 1 ，不要写 2
    {
        Error_Handler();
    }
}

void WriteParameter(uint16_t data)
{
    data = data | 0x100; // bit[8]: 1 表示写数据

    if(HAL_SPI_Transmit(&hspi2, (uint8_t *)(&data), 1, HAL_MAX_DELAY) != HAL_OK)
    //                                              ^ 对于 9bit 硬件 SPI 传输长度取 1 ，不要写 2
    {
        Error_Handler();
    }
}



/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
const uint32_t ledTaskStackSize = 512 * 4;
const osThreadAttr_t ledTask_attributes = {
    .name = "ledTask",
    .stack_size = ledTaskStackSize,
    .priority = (osPriority_t) osPriorityNormal,
};

/* Private constants ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
osThreadId_t ledTaskHandle;

/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Function implementations --------------------------------------------------*/

// Default color definitions
#define TFT_BLACK       0x0000      /*   0,   0,   0 */
#define TFT_NAVY        0x000F      /*   0,   0, 128 */
#define TFT_DARKGREEN   0x03E0      /*   0, 128,   0 */
#define TFT_DARKCYAN    0x03EF      /*   0, 128, 128 */
#define TFT_MAROON      0x7800      /* 128,   0,   0 */
#define TFT_PURPLE      0x780F      /* 128,   0, 128 */
#define TFT_OLIVE       0x7BE0      /* 128, 128,   0 */
#define TFT_LIGHTGREY   0xD69A      /* 211, 211, 211 */
#define TFT_DARKGREY    0x7BEF      /* 128, 128, 128 */
#define TFT_BLUE        0x001F      /*   0,   0, 255 */
#define TFT_GREEN       0x07E0      /*   0, 255,   0 */
#define TFT_CYAN        0x07FF      /*   0, 255, 255 */
#define TFT_RED         0xF800      /* 255,   0,   0 */
#define TFT_MAGENTA     0xF81F      /* 255,   0, 255 */
#define TFT_YELLOW      0xFFE0      /* 255, 255,   0 */
#define TFT_WHITE       0xFFFF      /* 255, 255, 255 */
#define TFT_ORANGE      0xFDA0      /* 255, 180,   0 */
#define TFT_GREENYELLOW 0xB7E0      /* 180, 255,   0 */
#define TFT_PINK        0xFE19      /* 255, 192, 203 */ //Lighter pink, was 0xFC9F
#define TFT_BROWN       0x9A60      /* 150,  75,   0 */
#define TFT_GOLD        0xFEA0      /* 255, 215,   0 */
#define TFT_SILVER      0xC618      /* 192, 192, 192 */
#define TFT_SKYBLUE     0x867D      /* 135, 206, 235 */
#define TFT_VIOLET      0x915C      /* 180,  46, 226 */

/* Thread definitions */
void threadLedUpdate(void* argument){
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = 1000;
	/* 获取当前的系统时间 */
	xLastWakeTime = xTaskGetTickCount();

	for(;;){
		/* 翻转开发板引脚 */
//		HAL_GPIO_TogglePin(VOUT_EN_GPIO_Port, VOUT_EN_Pin);

		/* 打印时间节拍 */
		printf("[led_task] sysTick %ld ms\r\n", xTaskGetTickCount());

		/* arm math 单精度硬件浮点测试 */
//		float data[3];
//		data[0] = arm_sin_f32(3.1415926/6);	// sin(PI/6)
//		data[1] = arm_sin_f32(3.1415926/1);	// sin(PI/1)
//		data[2] = arm_sin_f32(3.1415926/3);	// sin(PI/3)
//		printf("[sin] 30°= %.6f, 45°= %.6f, 60°= %.6f\r\n", data[0], data[1], data[2]);

		static uint8_t i_color = 0;
	    switch(i_color) {
	    case 0:
	        LCD_Clear(TFT_RED);
	        break;
	    case 1:
	        LCD_Clear(TFT_YELLOW);
	        break;
	    case 2:
	        LCD_Clear(TFT_BLUE);
	        break;
	    case 3:
	        LCD_Clear(TFT_GREEN);
	        break;
	    case 4:
	        LCD_Clear(TFT_PURPLE);
	        break;
	    default:
	        break;
	    }
	    i_color++;
	    i_color = i_color % 5;
		vTaskDelayUntil(&xLastWakeTime, xFrequency);

//      Touch_Scan();       // 触摸扫描
//      LED1_Toggle;        // LED指示
//		lv_task_handler();  // LVGL进程
//       vTaskDelayUntil(&xLastWakeTime, 20);  // GT911触摸屏扫描间隔不能小于10ms，建议设置为20ms
	}
}

void ledUpdateInit()
{
	ledTaskHandle = osThreadNew(threadLedUpdate, nullptr, &ledTask_attributes);
}


void Main()
{
	/* 初始化动态内存对象的内存池 */
//	DRAM_Init();

	/* 启用统计CPU利用率的定时器中断 */
	HAL_TIM_Base_Start_IT(&htim7);

    /* 初始化一些通信，USB-CDC/VCP/WIFI等 */
    InitCommunication();

    /* 初始化LED时间片任务 */
    ledUpdateInit();


    LCD_Clear(0xff000000);  //  清屏为黑色

    // 因为 LVGL 需要用__aeabi_assert，因此不能再勾选 microLib，
    // 若需要使用串口打印，需要添加部分代码，详见 usart.c 第102行
    //  USART1_Init();              // USART1初始化    ，

//    lv_init();                  //  LVGL初始化
//    lv_port_disp_init();        //  LVGL显示接口初始化
////    lv_port_indev_init();   // LVGL触摸接口初始化
//
//    lv_demo_music();        // 运行官方例程

}

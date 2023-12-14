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

/* Thread definitions */
void threadLedUpdate(void* argument){
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = 10000;
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

		vTaskDelayUntil(&xLastWakeTime, xFrequency);
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


//    for(int i = 0; i < 100; i++) {
//        HAL_GPIO_TogglePin(SPI2_SCK_GPIO_Port, SPI2_SCK_Pin);
//        HAL_Delay(10);
//    }


    HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_SET);
    HAL_Delay(50);
    HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_RESET);
//    GY09C_out code
    WriteCommand(0xFF);
    WriteParameter(0x77);
    WriteParameter(0x01);
    WriteParameter(0x00);
    WriteParameter(0x00);
    WriteParameter(0x13);

    WriteCommand(0xEF);
    WriteParameter(0x08);

    WriteCommand(0xFF);
    WriteParameter(0x77);
    WriteParameter(0x01);
    WriteParameter(0x00);
    WriteParameter(0x00);
    WriteParameter(0x10);


    WriteCommand(0xC0);
    WriteParameter(0x3B);
    WriteParameter(0x00);

    WriteCommand(0xC1);
    WriteParameter(0x12);
    WriteParameter(0x0A);

    WriteCommand(0xC2);
    WriteParameter(0x07); //DOT INV
    WriteParameter(0x03);

    WriteCommand(0xC3);
    WriteParameter(0x02);  //80：HV mode

    WriteCommand(0xCC);
    WriteParameter(0x10);

    WriteCommand(0xCD);
    WriteParameter(0x08);

    WriteCommand(0xB0);
    WriteParameter(0x0F);
    WriteParameter(0x11);
    WriteParameter(0x17);
    WriteParameter(0x15);
    WriteParameter(0x15);
    WriteParameter(0x09);
    WriteParameter(0x0C);
    WriteParameter(0x08);
    WriteParameter(0x08);
    WriteParameter(0x26);
    WriteParameter(0x04);
    WriteParameter(0x59);
    WriteParameter(0x16);
    WriteParameter(0x66);
    WriteParameter(0x2D);
    WriteParameter(0x1F);

    WriteCommand(0xB1);
    WriteParameter(0x0F);
    WriteParameter(0x11);
    WriteParameter(0x17);
    WriteParameter(0x15);
    WriteParameter(0x15);
    WriteParameter(0x09);
    WriteParameter(0x0C);
    WriteParameter(0x08);
    WriteParameter(0x08);
    WriteParameter(0x26);
    WriteParameter(0x04);
    WriteParameter(0x59);
    WriteParameter(0x16);
    WriteParameter(0x66);
    WriteParameter(0x2D);
    WriteParameter(0x1F);

    WriteCommand(0xFF);
    WriteParameter(0x77);
    WriteParameter(0x01);
    WriteParameter(0x00);
    WriteParameter(0x00);
    WriteParameter(0x11);

    WriteCommand(0xB0);
    WriteParameter(0x6D);

    WriteCommand(0xB1);  //VCOM
    WriteParameter(0x3A); //30/47

    WriteCommand(0xB2); //VGH
    WriteParameter(0x01);

    WriteCommand(0xB3);
    WriteParameter(0x80);

    WriteCommand(0xB5);  //vgl
    WriteParameter(0x49);

    WriteCommand(0xB7);
    WriteParameter(0x85);

    WriteCommand(0xB8); //avdd
    WriteParameter(0x20);

    //WriteCommand(0xB9);
    //WriteParameter(0x10);

    WriteCommand(0xC1);
    WriteParameter(0x78);

    WriteCommand(0xC2);
    WriteParameter(0x78);

    WriteCommand(0xD0);
    WriteParameter(0x88);

    WriteCommand(0xE0);
    WriteParameter(0x00);
    WriteParameter(0x00);
    WriteParameter(0x02);

    WriteCommand(0xE1);
    WriteParameter(0x07);
    WriteParameter(0x00);
    WriteParameter(0x09);
    WriteParameter(0x00);
    WriteParameter(0x06);
    WriteParameter(0x00);
    WriteParameter(0x08);
    WriteParameter(0x00);
    WriteParameter(0x00);
    WriteParameter(0x33);
    WriteParameter(0x33);


    WriteCommand(0xE2);
    WriteParameter(0x11);
    WriteParameter(0x11);
    WriteParameter(0x33);
    WriteParameter(0x33);
    WriteParameter(0xF6);
    WriteParameter(0x00);
    WriteParameter(0xF6);
    WriteParameter(0x00);
    WriteParameter(0xF6);
    WriteParameter(0x00);
    WriteParameter(0xF6);
    WriteParameter(0x00);
    WriteParameter(0x00);


    WriteCommand(0xE3);
    WriteParameter(0x00);
    WriteParameter(0x00);
    WriteParameter(0x11);
    WriteParameter(0x11);


    WriteCommand(0xE4);
    WriteParameter(0x44);
    WriteParameter(0x44);

    WriteCommand(0xE5);
    WriteParameter(0x0F);
    WriteParameter(0xF3);
    WriteParameter(0x3D);
    WriteParameter(0xFF);
    WriteParameter(0x11);
    WriteParameter(0xF5);
    WriteParameter(0x3D);
    WriteParameter(0xFF);
    WriteParameter(0x0B);
    WriteParameter(0xEF);
    WriteParameter(0x3D);
    WriteParameter(0xFF);
    WriteParameter(0x0D);
    WriteParameter(0xF1);
    WriteParameter(0x3D);
    WriteParameter(0xFF);



    WriteCommand(0xE6);
    WriteParameter(0x00);
    WriteParameter(0x00);
    WriteParameter(0x11);
    WriteParameter(0x11);

    WriteCommand(0xE7);
    WriteParameter(0x44);
    WriteParameter(0x44);


    WriteCommand(0xE8);
    WriteParameter(0x0E);
    WriteParameter(0xF2);
    WriteParameter(0x3D);
    WriteParameter(0xFF);
    WriteParameter(0x10);
    WriteParameter(0xF4);
    WriteParameter(0x3D);
    WriteParameter(0xFF);
    WriteParameter(0x0A);
    WriteParameter(0xEE);
    WriteParameter(0x3D);
    WriteParameter(0xFF);
    WriteParameter(0x0C);
    WriteParameter(0xF0);
    WriteParameter(0x3D);
    WriteParameter(0xFF);


    WriteCommand(0xE9);
    WriteParameter(0x36);
    WriteParameter(0x00);

    WriteCommand(0xEB);
    WriteParameter(0x00);
    WriteParameter(0x01);
    WriteParameter(0xE4);
    WriteParameter(0xE4);
    WriteParameter(0x44);
    WriteParameter(0xAA);//AA
    WriteParameter(0x10);//10


    WriteCommand(0xEC);
    WriteParameter(0x3C);
    WriteParameter(0x00);


    WriteCommand(0xED);
    WriteParameter(0xFF);
    WriteParameter(0x45);
    WriteParameter(0x67);
    WriteParameter(0xFA);
    WriteParameter(0x01);
    WriteParameter(0x2B);
    WriteParameter(0xCF);
    WriteParameter(0xFF);
    WriteParameter(0xFF);
    WriteParameter(0xFC);
    WriteParameter(0xB2);
    WriteParameter(0x10);
    WriteParameter(0xAF);
    WriteParameter(0x76);
    WriteParameter(0x54);
    WriteParameter(0xFF);

    WriteCommand(0xEF);
    WriteParameter(0x10);
    WriteParameter(0x0D);
    WriteParameter(0x04);
    WriteParameter(0x08);
    WriteParameter(0x3F);
    WriteParameter(0x1F);

    WriteCommand(0xFF);
    WriteParameter(0x77);
    WriteParameter(0x01);
    WriteParameter(0x00);
    WriteParameter(0x00);
    WriteParameter(0x00);

    WriteCommand(0x35);
    WriteParameter(0x00);

    WriteCommand(0x3A);
    WriteParameter(0x66);

    //WriteCommand(0x21);
    WriteCommand(0x11);

    HAL_Delay(120);      //ms



    WriteCommand(0x29);



    HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_SET);

//    LCD_CopyBuffer(0, 0, 480, 480, (uint32_t *)0xC0000000);
//    HAL_Delay(100);      //ms
}

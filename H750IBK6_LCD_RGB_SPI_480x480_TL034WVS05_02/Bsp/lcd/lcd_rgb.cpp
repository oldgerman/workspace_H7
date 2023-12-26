/**
  ******************************************************************************
  * @file        lcd_rgb.cpp
  * @author      OldGerman
  * @created on  Dec 14, 2023
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
#include "lcd_rgb.h"

#include "spi.h"
#include "ltdc.h"
#include "dma2d.h"

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
  * @brief  硬件 SPI 9bt 模式写 ST7701s 命令
  * @note   CubeMX需要配置SPI位宽为9bit模式
  * @param  data:  需要写的数据
  * @retval None
  */
void WriteCommand(uint16_t data)
{
    HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_RESET);
    data = data | 0x000; // bit[8]: 0 表示写命令

    if(HAL_SPI_Transmit(&hspi2, (uint8_t *)(&data), 1, HAL_MAX_DELAY) != HAL_OK)
    //                                              ^ 对于 9bit 硬件 SPI 传输长度取 1 ，不要写 2
    {
        Error_Handler();
    }
    HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_SET);
}

/**
  * @brief  硬件 SPI 9bt 模式写 ST7701s 参数
  * @note   CubeMX需要配置SPI位宽为9bit模式
  * @param  data:  需要写的数据
  * @retval None
  */
void WriteParameter(uint16_t data)
{
    HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_RESET);
    data = data | 0x100; // bit[8]: 1 表示写数据

    if(HAL_SPI_Transmit(&hspi2, (uint8_t *)(&data), 1, HAL_MAX_DELAY) != HAL_OK)
    //                                              ^ 对于 9bit 硬件 SPI 传输长度取 1 ，不要写 2
    {
        Error_Handler();
    }
    HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_SET);
}


/**
  * @brief  初始化 ST7701s，并清屏为黑色
  * @param  None
  * @retval None
  */
void LCD_Init()
{
    /* 复位 */
    HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
    LCD_Delay(10);
    HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET);
    LCD_Delay(20);
    HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
    LCD_Delay(120);                //ms

    /* 写命令和参数 */
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
    WriteParameter(0xE9);
    WriteParameter(0x03);
    WriteCommand(0xC1);
    WriteParameter(0x10);
    WriteParameter(0x0C);
    WriteCommand(0xC2);
    WriteParameter(0x01);
    WriteParameter(0x0A);
    WriteCommand(0xC3);
    WriteParameter(0x02);

    WriteCommand(0xCC);
    WriteParameter(0x10);
    WriteCommand(0xCD);
    WriteParameter(0x08);
    WriteCommand(0xB0);
    WriteParameter(0x0D);
    WriteParameter(0x14);
    WriteParameter(0x9C);
    WriteParameter(0x0B);
    WriteParameter(0x10);
    WriteParameter(0x06);
    WriteParameter(0x08);
    WriteParameter(0x09);
    WriteParameter(0x08);
    WriteParameter(0x22);
    WriteParameter(0x02);
    WriteParameter(0x4F);
    WriteParameter(0x0E);
    WriteParameter(0x66);
    WriteParameter(0x2D);
    WriteParameter(0x1F);
    WriteCommand(0xB1);
    WriteParameter(0x00);
    WriteParameter(0x17);
    WriteParameter(0x9E);
    WriteParameter(0x0F);
    WriteParameter(0x11);
    WriteParameter(0x06);
    WriteParameter(0x0C);
    WriteParameter(0x08);
    WriteParameter(0x08);
    WriteParameter(0x26);
    WriteParameter(0x04);
    WriteParameter(0x51);
    WriteParameter(0x10);
    WriteParameter(0x6A);
    WriteParameter(0x33);
    WriteParameter(0x1B);

    WriteCommand(0xFF);
    WriteParameter(0x77);
    WriteParameter(0x01);
    WriteParameter(0x00);
    WriteParameter(0x00);
    WriteParameter(0x11);

    WriteCommand(0xB0);
    WriteParameter(0x30);

    WriteCommand(0xB1);
    WriteParameter(0x47); //VOCM   57

    WriteCommand(0xB2);
    WriteParameter(0x84);

    WriteCommand(0xB3);
    WriteParameter(0x80);

    WriteCommand(0xB5);
    WriteParameter(0x4E);

    WriteCommand(0xB7);
    WriteParameter(0x85);

    WriteCommand(0xB8);
    WriteParameter(0x20);

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
    WriteParameter(0x06);
    WriteParameter(0xA0);
    WriteParameter(0x08);
    WriteParameter(0xA0);
    WriteParameter(0x05);
    WriteParameter(0xA0);
    WriteParameter(0x07);
    WriteParameter(0xA0);
    WriteParameter(0x00);
    WriteParameter(0x44);
    WriteParameter(0x44);
    WriteCommand(0xE2);
    WriteParameter(0x30);
    WriteParameter(0x30);
    WriteParameter(0x44);
    WriteParameter(0x44);
    WriteParameter(0x6E);
    WriteParameter(0xA0);
    WriteParameter(0x00);
    WriteParameter(0x00);
    WriteParameter(0x6E);
    WriteParameter(0xA0);
    WriteParameter(0x00);
    WriteParameter(0x00);
    WriteCommand(0xE3);
    WriteParameter(0x00);
    WriteParameter(0x00);
    WriteParameter(0x33);
    WriteParameter(0x33);
    WriteCommand(0xE4);
    WriteParameter(0x44);
    WriteParameter(0x44);
    WriteCommand(0xE5);
    WriteParameter(0x0D);
    WriteParameter(0x69);
    WriteParameter(0x0A);
    WriteParameter(0xA0);
    WriteParameter(0x0F);
    WriteParameter(0x6B);
    WriteParameter(0x0A);
    WriteParameter(0xA0);
    WriteParameter(0x09);
    WriteParameter(0x65);
    WriteParameter(0x0A);
    WriteParameter(0xA0);
    WriteParameter(0x0B);
    WriteParameter(0x67);
    WriteParameter(0x0A);
    WriteParameter(0xA0);
    WriteCommand(0xE6);
    WriteParameter(0x00);
    WriteParameter(0x00);
    WriteParameter(0x33);
    WriteParameter(0x33);
    WriteCommand(0xE7);
    WriteParameter(0x44);
    WriteParameter(0x44);
    WriteCommand(0xE8);
    WriteParameter(0x0C);
    WriteParameter(0x68);
    WriteParameter(0x0A);
    WriteParameter(0xA0);
    WriteParameter(0x0E);
    WriteParameter(0x6A);
    WriteParameter(0x0A);
    WriteParameter(0xA0);
    WriteParameter(0x08);
    WriteParameter(0x64);
    WriteParameter(0x0A);
    WriteParameter(0xA0);
    WriteParameter(0x0A);
    WriteParameter(0x66);
    WriteParameter(0x0A);
    WriteParameter(0xA0);
    WriteCommand(0xE9);
    WriteParameter(0x36);
    WriteParameter(0x00);
    WriteCommand(0xEB);
    WriteParameter(0x00);
    WriteParameter(0x01);
    WriteParameter(0xE4);
    WriteParameter(0xE4);
    WriteParameter(0x44);
    WriteParameter(0x88);
    WriteParameter(0x40);
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
    WriteCommand(0x3A);
    WriteParameter(0x66);

    WriteCommand(0x11);

    LCD_Delay(120);

    WriteCommand(0x29);
    WriteCommand(0x35);
    WriteParameter(0x00);

    /* 清屏为黑色 */
    LCD_Clear(0xff000000);
}

/**
  * @brief  使用DMA2D清屏，将LCD清除为 color 的颜色
  * @note   1. 使用DMA2D实现
  *         2. 阻塞式
  * @param  color:  颜色
  * @retval None
  */
void LCD_Clear(uint32_t color)
{

    DMA2D->CR     &=    ~(DMA2D_CR_START);              //  停止DMA2D
    DMA2D->CR       =   DMA2D_R2M;                          //  寄存器到SDRAM
    DMA2D->OPFCCR   =   ColorMode_0;                        //  设置颜色格式
    DMA2D->OOR      =   0;                                      //  设置行偏移
    DMA2D->OMAR     =   LCD_MemoryAdd ;                 // 地址
    DMA2D->NLR      =   (LCD_Width<<16)|(LCD_Height);   //  设定长度和宽度

    DMA2D->OCOLR    =   color;                          //  黑色


    DMA2D->CR     |=    DMA2D_CR_START;                 //  启动DMA2D

    while (DMA2D->CR & DMA2D_CR_START) ;                //  等待传输完成

}

/**
  * @brief  使用DMA2D在坐标 (x,y) 起始处复制缓冲区到显示区
  * @note   1. 使用DMA2D实现
  *         2. 阻塞式
  *         3. 要绘制的区域不能超过屏幕的显示区域
  *         4. 可在 lv_port_disp.c 文件中，被函数 disp_flush() 调用，用以刷新显示区域
  * @param  x:      水平坐标
  * @param  y：     垂直坐标
  * @param  width:  图片的水平宽度
  * @param  height: 图片的垂直宽度
  * @param  color:  要复制的缓冲区地址
  * @retval None
  */
void LCD_CopyBuffer(uint16_t x, uint16_t y, uint16_t width, uint16_t height,uint32_t *color)
{

    DMA2D->CR     &=    ~(DMA2D_CR_START);              //  停止DMA2D
    DMA2D->CR       =   DMA2D_M2M;                          //  存储器到存储器
    DMA2D->FGPFCCR  =   LTDC_PIXEL_FORMAT_RGB565;       //  设置颜色格式
    DMA2D->FGOR    =  0;                                     //
    DMA2D->OOR      =   LCD_Width - width;              //  设置行偏移
    DMA2D->FGMAR   =  (uint32_t)color;
    DMA2D->OMAR     =   LCD_MemoryAdd + BytesPerPixel_0*(LCD_Width * y + x);    // 地址;
    DMA2D->NLR      =   (width<<16)|(height);           //  设定长度和宽度
    DMA2D->CR     |=    DMA2D_CR_START;                 //  启动DMA2D

    while (DMA2D->CR & DMA2D_CR_START) ;               //  等待传输完成

}

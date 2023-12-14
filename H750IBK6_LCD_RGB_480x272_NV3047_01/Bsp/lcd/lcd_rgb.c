﻿/***
	************************************************************************************************************************************************************************************************
	*	@file  	lcd_rgb.c
	*	@version V1.0
	*  @date    2022-7-11
	*	@author  反客科技
	*	@brief   驱动RGB显示屏进行显示	
   **********************************************************************************************************************************************************************************************
   *  @description
	*
	*	实验平台：反客STM32H750XBH6核心板 （型号：FK750M5-XBH6）+ 反客800*480分辨率的RGB屏幕
	*	淘宝地址：https://shop212360197.taobao.com
	*	QQ交流群：536665479
	*
>>>>> 重要说明：
	*
	*	1. FK750M5-XBH6 核心板 使用的是外部SDRAM作为显存，起始地址0xC0000000
	*	2. 在刚下载完程序时，屏幕有轻微抖动和闪烁属于正常现象，等待片刻或者重新上电即可恢复正常
	* 	3. LTDC时钟在 main.c 文件里的 SystemClock_Config()函数设置，配置为33MHz，即刷新率在60帧左右，过高或者过低都会造成闪烁
	*********************************************************************************************************************************************************************************************FANKE*****
***/

#include "lcd_rgb.h"

#include "ltdc.h"
#include "dma2d.h"


/***************************************************************************************************************
*   函 数 名:  LCD_Clear
*
*   函数功能:   清屏函数，将LCD清除为 color 的颜色，使用DMA2D实现
*
***************************************************************************************************************/

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

/***************************************************************************************************************************************
*   函 数 名: LCD_CopyBuffer
*
*   入口参数: x - 水平坐标，取值范围 0~479
*                y - 垂直坐标，取值范围 0~271
*                width  - 图片的水平宽度，最大取值480
*                height - 图片的垂直宽度，最大取值272
*                *color - 要复制的缓冲区地址
*
*   函数功能: 在坐标 (x,y) 起始处复制缓冲区到显示区
*
*   说    明: 1. 使用DMA2D实现
*                2. 要绘制的区域不能超过屏幕的显示区域
*                3. 在 lv_port_disp.c 文件中，被函数 disp_flush() 调用，用以刷新显示区域
*
*****************************************************************************************************************************************/

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

    while (DMA2D->CR & DMA2D_CR_START) ;            //  等待传输完成

}

/**************************************************************************************************************************************************************************************************************************************************************************FANKE***/
// 实验平台：反客 STM32H750核心板
//

/**
  ******************************************************************************
  * @file        lcd_rgb.h
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef LCD_LCD_RGB_H_
#define LCD_LCD_RGB_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
//#include "stm32h7xx_hal.h"
//#include "usart.h"
#include "main.h"
#include "sdram.h"
#include <stdio.h>

/* Exported types ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/

// 1. 如果只用单层，该参数定义为1即可，使用双层的话，需要修改为 2
// 2. FK750M5-XBH6 核心板 使用的是外部SDRAM作为显存，起始地址0xC0000000
// 3. 显存所需空间 = 分辨率 * 每个像素所占字节数，例如 480*272的屏，使用16位色（RGB565或者AEGB1555），需要显存 480*272*2 = 261120 字节
//
#define     LCD_NUM_LAYERS  1   //  定义显示的层数，750可驱动两层显示

#define ColorMode_0   LTDC_PIXEL_FORMAT_RGB565          //定义 layer0 的颜色格式
//#define   ColorMode_0   LTDC_PIXEL_FORMAT_ARGB1555
//#define   ColorMode_0    LTDC_PIXEL_FORMAT_ARGB4444
//#define   ColorMode_0   LTDC_PIXEL_FORMAT_RGB888
//#define   ColorMode_0   LTDC_PIXEL_FORMAT_ARGB8888


#if  LCD_NUM_LAYERS == 2    // 如果开启了双层，则在此处定义 layer1 的颜色格式

//  #define ColorMode_1   LTDC_PIXEL_FORMAT_RGB565
//  #define ColorMode_1   LTDC_PIXEL_FORMAT_ARGB1555  // ARGB1555仅支持一位透明色，即仅有透明和不透明两种状态
//  #define ColorMode_1   LTDC_PIXEL_FORMAT_ARGB4444  // ARGB4444支持4位透明色，有16种透明状态
// #define  ColorMode_1   LTDC_PIXEL_FORMAT_RGB888
#define ColorMode_1   LTDC_PIXEL_FORMAT_ARGB8888   // ARGB8888支持8位透明色，有256种透明状态

#endif


/*-------------------------------------------------------- LCD相关参数 -------------------------------------------------------*/
// 根据 TL034WVS05-B1477A 屏幕的手册进行设置
#define THBP  20     // thb：  HSD blanking
#define TVBP  10     // tvb：  VSD Blanking
#define THSW  10     // thpw： HSD pulse width
#define TVSW  6      // tvpw： VSD pulse width
#define THFP  10     // thfp： HSD Front porch
#define TVFP  10     // tvfp： VSD Front porch

#define LCD_Width       480             //  LCD的像素长度
#define LCD_Height      480             //  LCD的像素宽度
#define LCD_MemoryAdd   SDRAM_BANK1_ADDR // 显存的起始地址

// layer0 每个像素所占字节

#if ( ColorMode_0 == LTDC_PIXEL_FORMAT_RGB565 || ColorMode_0 == LTDC_PIXEL_FORMAT_ARGB1555 || ColorMode_0 ==LTDC_PIXEL_FORMAT_ARGB4444 )
#define BytesPerPixel_0     2       //16位色模式每个像素占2字节
#elif ColorMode_0 == LTDC_PIXEL_FORMAT_RGB888
#define BytesPerPixel_0     3       //24位色模式每个像素占3字节
#elif ColorMode_0 == LTDC_PIXEL_FORMAT_ARGB8888
#define BytesPerPixel_0     4       //32位色模式每个像素占4字节
#endif

// layer1 每个像素所占字节

#if LCD_NUM_LAYERS == 2

#if ( ColorMode_1 == LTDC_PIXEL_FORMAT_RGB565 || ColorMode_1 == LTDC_PIXEL_FORMAT_ARGB1555 || ColorMode_1 == LTDC_PIXEL_FORMAT_ARGB4444 )
#define BytesPerPixel_1     2   //16位色模式每个像素占2字节
#elif ColorMode_1 == LTDC_PIXEL_FORMAT_RGB888
#define BytesPerPixel_1     3   //24位色模式每个像素占3字节
#else
#define BytesPerPixel_1     4   //32位色模式每个像素占4字节
#endif

#define LCD_MemoryAdd_OFFSET   LCD_Width * LCD_Height * BytesPerPixel_0      //第二层的显存的偏移地址

#endif

/*-------------------------------------------------------- LCD背光引脚 -------------------------------------------------------*/

//#define  LCD_Backlight_PIN                                GPIO_PIN_6
//#define   LCD_Backlight_PORT                          GPIOH
//#define   GPIO_LDC_Backlight_CLK_ENABLE           __HAL_RCC_GPIOH_CLK_ENABLE()
//
//#define   LCD_Backlight_OFF       HAL_GPIO_WritePin(LCD_Backlight_PORT, LCD_Backlight_PIN, GPIO_PIN_RESET);   // 关闭背光
//#define   LCD_Backlight_ON        HAL_GPIO_WritePin(LCD_Backlight_PORT, LCD_Backlight_PIN, GPIO_PIN_SET);     // 开启背光



/* Exported macro ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

void MX_LTDC_Init(void);        // LTDC以及层初始化
void  LCD_CopyBuffer(uint16_t x, uint16_t y, uint16_t width, uint16_t height,uint32_t *color);  // 复制缓冲区到显示区


#ifdef __cplusplus
}
#endif

#endif /* LCD_LCD_RGB_H_ */

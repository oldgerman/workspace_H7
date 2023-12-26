/**
  ******************************************************************************
  * @file        lcd_rgb.h
  * @modify      OldGerman
  * @created on  Dec 14, 2023
  * @brief       
  ******************************************************************************
  * @attention
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

#define ColorMode_0   LTDC_PIXEL_FORMAT_RGB565          //定义 layer0 的颜色格式
//#define   ColorMode_0   LTDC_PIXEL_FORMAT_ARGB1555
//#define   ColorMode_0    LTDC_PIXEL_FORMAT_ARGB4444
//#define   ColorMode_0   LTDC_PIXEL_FORMAT_RGB888
//#define   ColorMode_0   LTDC_PIXEL_FORMAT_ARGB8888

#define HBP  43 // 根据屏幕的手册进行设置
#define VBP  12
#define HSW  1
#define VSW  1
#define HFP  8
#define VFP  8

#define LCD_Width       480             //  LCD的像素长度
#define LCD_Height      272             //  LCD的像素宽度
#define LCD_MemoryAdd   LCD_MEM_ADDR      //  显存的起始地址

// layer0 每个像素所占字节

#if ( ColorMode_0 == LTDC_PIXEL_FORMAT_RGB565 || ColorMode_0 == LTDC_PIXEL_FORMAT_ARGB1555 || ColorMode_0 ==LTDC_PIXEL_FORMAT_ARGB4444 )
    #define BytesPerPixel_0     2       //16位色模式每个像素占2字节
#elif ColorMode_0 == LTDC_PIXEL_FORMAT_RGB888
    #define BytesPerPixel_0     3       //24位色模式每个像素占3字节
#else
    #define BytesPerPixel_0     4       //32位色模式每个像素占4字节
#endif


/* Exported macro ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
void LCD_Clear(uint32_t color);
void LCD_CopyBuffer(uint16_t x, uint16_t y, uint16_t width, uint16_t height,uint32_t *color);

#ifdef __cplusplus
}
#endif

#endif /* LCD_LCD_RGB_H_ */

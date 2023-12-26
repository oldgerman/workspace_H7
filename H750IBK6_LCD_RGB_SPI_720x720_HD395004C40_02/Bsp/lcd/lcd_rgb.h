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
/* 设置延时函数 */
#define LCD_Delay HAL_Delay
//#define LCD_Delay osDelay

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


#define ColorMode_0   LTDC_PIXEL_FORMAT_RGB565          //定义 layer0 的颜色格式
//#define   ColorMode_0   LTDC_PIXEL_FORMAT_ARGB1555
//#define   ColorMode_0    LTDC_PIXEL_FORMAT_ARGB4444
//#define   ColorMode_0   LTDC_PIXEL_FORMAT_RGB888
//#define   ColorMode_0   LTDC_PIXEL_FORMAT_ARGB8888
#define LCD_Width       LCD_T_HD             //  LCD的像素长度
#define LCD_Height      LCD_T_VD             //  LCD的像素宽度
#define LCD_MemoryAdd   LCD_MEM_ADDR      //  显存的起始地址

void  LCD_Clear(uint32_t color);
void  LCD_CopyBuffer(uint16_t x, uint16_t y, uint16_t width, uint16_t height,uint32_t *color);
void  LCD_CopyBuffer_IT(uint16_t x, uint16_t y, uint16_t width, uint16_t height,uint32_t *color);


// layer0 每个像素所占字节

#if ( ColorMode_0 == LTDC_PIXEL_FORMAT_RGB565 || ColorMode_0 == LTDC_PIXEL_FORMAT_ARGB1555 || ColorMode_0 ==LTDC_PIXEL_FORMAT_ARGB4444 )
    #define BytesPerPixel_0     2       //16位色模式每个像素占2字节
#elif ColorMode_0 == LTDC_PIXEL_FORMAT_RGB888
    #define BytesPerPixel_0     3       //24位色模式每个像素占3字节
#else
    #define BytesPerPixel_0     4       //32位色模式每个像素占4字节
#endif


/* LCD背光引脚 */


/* Exported macro ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
void WriteCommand(uint16_t data);
void WriteParameter(uint16_t data);
void LCD_Init();
void LCD_Clear(uint32_t color);
void LCD_CopyBuffer(uint16_t x, uint16_t y, uint16_t width, uint16_t height,uint32_t *color);

#ifdef __cplusplus
}
#endif

#endif /* LCD_LCD_RGB_H_ */

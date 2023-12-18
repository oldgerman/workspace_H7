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

#include "tft.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern __IO uint32_t   transferCompleteDetected;

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
//    HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_RESET);
    data = data | 0x000; // bit[8]: 0 表示写命令

    if(HAL_SPI_Transmit(&hspi2, (uint8_t *)(&data), 1, HAL_MAX_DELAY) != HAL_OK)
    //                                              ^ 对于 9bit 硬件 SPI 传输长度取 1 ，不要写 2
    {
        Error_Handler();
    }
//    HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_SET);
}

/**
  * @brief  硬件 SPI 9bt 模式写 ST7701s 参数
  * @note   CubeMX需要配置SPI位宽为9bit模式
  * @param  data:  需要写的数据
  * @retval None
  */
void WriteParameter(uint16_t data)
{
//    HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_RESET);
    data = data | 0x100; // bit[8]: 1 表示写数据

    if(HAL_SPI_Transmit(&hspi2, (uint8_t *)(&data), 1, HAL_MAX_DELAY) != HAL_OK)
    //                                              ^ 对于 9bit 硬件 SPI 传输长度取 1 ，不要写 2
    {
        Error_Handler();
    }
//    HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_SET);
}

void Wrt_Reg_3052(uint16_t cmd, uint16_t param)
{
    HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_RESET);
        LCD_Delay(1);
    WriteCommand(cmd);
    WriteParameter(param);
        LCD_Delay(1);
    HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_SET);
}

void Write_LCD_REG(uint16_t unused, uint16_t data)
{
    HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_RESET);
//    LCD_Delay(1);
    if(HAL_SPI_Transmit(&hspi2, (uint8_t *)(&data), 1, HAL_MAX_DELAY) != HAL_OK)
    //                                              ^ 对于 9bit 硬件 SPI 传输长度取 1 ，不要写 2
    {
        Error_Handler();
    }
//    LCD_Delay(1);
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
    LCD_Delay(20); // 10
    HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET);
    LCD_Delay(40); // 20
    HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
    LCD_Delay(200); // 120ms, 注意，如果160倍频改为192使主频480MHz，那么这个时间需要延长1.2倍，否则NV3052复位时间不足会导致刷屏水波纹

    /* 写命令和参数 */

    /*
     * LTDC 只有刷新显示区域时才占用SDRAM 总线带宽，在水平消隐和垂直消隐期间都不占用 SDRAM 带宽
     * 但只有垂直消隐的 front 和 back 周期加起来的这段时间可以用
     *
     * LTDC 自刷新 720x720 16bit 的区域消耗 SDRAM 读带宽 59.32MB/s，此阶段冗余带宽约 114-59 = 55MB/s
     * 如果使用 DMA2D在垂直消隐期间刷SDRAM帧缓冲区到SDRAM LTDC缓冲区，则读带宽冗余 114MB/s，写带宽 184MB/s，因为SDRAM读写不能同时进行，得打折
     * [16:53:56.913] 以16位数据宽度写入数据，大小：32 MB，耗时: 173 ms, 写入速度：184.97 MB/s
     * [16:53:57.193] 读取数据完毕，大小：32 MB，耗时: 280 ms, 读取速度：114.29 MB/s
     */
    //#define VSPW 5
    //#define VBPD 15 \ 垂直消隐区才 30个 周期
    //#define VFPD 15 /
    //
    //#define HSPW 2
    //#define HBPD 44
    //#define HFPD 46

    // 720X720 BOE3.95 6G （B3 QV040YNQ-N80）
    ///////////////////////////////////
    Wrt_Reg_3052(0xFF,0x30);
    Wrt_Reg_3052(0xFF,0x52);
    Wrt_Reg_3052(0xFF,0x01);
    Wrt_Reg_3052(0xE3,0x00);
    Wrt_Reg_3052(0x0A,0x01);
    Wrt_Reg_3052(0x23,0xA0);//a2
    Wrt_Reg_3052(0x24,0x0f);
    Wrt_Reg_3052(0x25,0x14);
    Wrt_Reg_3052(0x26,0x2E);
    Wrt_Reg_3052(0x27,0x2E);
    Wrt_Reg_3052(0x29,0x02);
    Wrt_Reg_3052(0x2A,0xCF);
    Wrt_Reg_3052(0x32,0x34);

    //VCOM
    Wrt_Reg_3052(0x38,0x9C);
    Wrt_Reg_3052(0x39,0xA7);
    Wrt_Reg_3052(0x3A,0x4F);

    Wrt_Reg_3052(0x3B,0x94);
    Wrt_Reg_3052(0x40,0x07);
    Wrt_Reg_3052(0x42,0x6D);
    Wrt_Reg_3052(0x43,0x83);
    Wrt_Reg_3052(0x81,0x00);
    Wrt_Reg_3052(0x91,0x57);//2POWER
    Wrt_Reg_3052(0x92,0x57);
    Wrt_Reg_3052(0xA0,0x52);
    Wrt_Reg_3052(0xA1,0x50);
    Wrt_Reg_3052(0xA4,0x9C);
    Wrt_Reg_3052(0xA7,0x02);
    Wrt_Reg_3052(0xA8,0x02);
    Wrt_Reg_3052(0xA9,0x02);
    Wrt_Reg_3052(0xAA,0xA8);
    Wrt_Reg_3052(0xAB,0x28);
    Wrt_Reg_3052(0xAE,0xD2);
    Wrt_Reg_3052(0xAF,0x02);
    Wrt_Reg_3052(0xB0,0xD2);
    Wrt_Reg_3052(0xB2,0x26);
    Wrt_Reg_3052(0xB3,0x26);
    Wrt_Reg_3052(0xFF,0x30);
    Wrt_Reg_3052(0xFF,0x52);
    Wrt_Reg_3052(0xFF,0x02);
    Wrt_Reg_3052(0xB0,0x02);
    Wrt_Reg_3052(0xB1,0x31);
    Wrt_Reg_3052(0xB2,0x24);
    Wrt_Reg_3052(0xB3,0x30);
    Wrt_Reg_3052(0xB4,0x38);
    Wrt_Reg_3052(0xB5,0x3E);
    Wrt_Reg_3052(0xB6,0x26);
    Wrt_Reg_3052(0xB7,0x3E);
    Wrt_Reg_3052(0xB8,0x0a);
    Wrt_Reg_3052(0xB9,0x00);
    Wrt_Reg_3052(0xBA,0x11);
    Wrt_Reg_3052(0xBB,0x11);
    Wrt_Reg_3052(0xBC,0x13);
    Wrt_Reg_3052(0xBD,0x14);
    Wrt_Reg_3052(0xBE,0x18);
    Wrt_Reg_3052(0xBF,0x11);
    Wrt_Reg_3052(0xC0,0x16);
    Wrt_Reg_3052(0xC1,0x00);
    Wrt_Reg_3052(0xD0,0x05);
    Wrt_Reg_3052(0xD1,0x30);
    Wrt_Reg_3052(0xD2,0x25);
    Wrt_Reg_3052(0xD3,0x35);
    Wrt_Reg_3052(0xD4,0x34);
    Wrt_Reg_3052(0xD5,0x3B);
    Wrt_Reg_3052(0xD6,0x26);
    Wrt_Reg_3052(0xD7,0x3D);
    Wrt_Reg_3052(0xD8,0x0a);
    Wrt_Reg_3052(0xD9,0x00);
    Wrt_Reg_3052(0xDA,0x12);
    Wrt_Reg_3052(0xDB,0x10);
    Wrt_Reg_3052(0xDC,0x12);
    Wrt_Reg_3052(0xDD,0x14);
    Wrt_Reg_3052(0xDE,0x18);
    Wrt_Reg_3052(0xDF,0x11);
    Wrt_Reg_3052(0xE0,0x15);
    Wrt_Reg_3052(0xE1,0x00);
    Wrt_Reg_3052(0xFF,0x30);
    Wrt_Reg_3052(0xFF,0x52);
    Wrt_Reg_3052(0xFF,0x03);
    Wrt_Reg_3052(0x00,0x00);
    Wrt_Reg_3052(0x01,0x00);
    Wrt_Reg_3052(0x02,0x00);
    Wrt_Reg_3052(0x03,0x00);
    Wrt_Reg_3052(0x08,0x0D);
    Wrt_Reg_3052(0x09,0x0E);
    Wrt_Reg_3052(0x0A,0x0F);
    Wrt_Reg_3052(0x0B,0x10);
    Wrt_Reg_3052(0x20,0x00);
    Wrt_Reg_3052(0x21,0x00);
    Wrt_Reg_3052(0x22,0x00);
    Wrt_Reg_3052(0x23,0x00);
    Wrt_Reg_3052(0x28,0x22);
    Wrt_Reg_3052(0x2A,0xE9);
    Wrt_Reg_3052(0x2B,0xE9);
    Wrt_Reg_3052(0x30,0x00);
    Wrt_Reg_3052(0x31,0x00);
    Wrt_Reg_3052(0x32,0x00);
    Wrt_Reg_3052(0x33,0x00);
    Wrt_Reg_3052(0x34,0x01);
    Wrt_Reg_3052(0x35,0x00);
    Wrt_Reg_3052(0x36,0x00);
    Wrt_Reg_3052(0x37,0x03);
    Wrt_Reg_3052(0x40,0x0A);
    Wrt_Reg_3052(0x41,0x0B);
    Wrt_Reg_3052(0x42,0x0C);
    Wrt_Reg_3052(0x43,0x0D);
    Wrt_Reg_3052(0x44,0x22);
    Wrt_Reg_3052(0x45,0xE4);
    Wrt_Reg_3052(0x46,0xE5);
    Wrt_Reg_3052(0x47,0x22);
    Wrt_Reg_3052(0x48,0xE6);
    Wrt_Reg_3052(0x49,0xE7);
    Wrt_Reg_3052(0x50,0x0E);
    Wrt_Reg_3052(0x51,0x0F);
    Wrt_Reg_3052(0x52,0x10);
    Wrt_Reg_3052(0x53,0x11);
    Wrt_Reg_3052(0x54,0x22);
    Wrt_Reg_3052(0x55,0xE8);
    Wrt_Reg_3052(0x56,0xE9);
    Wrt_Reg_3052(0x57,0x22);
    Wrt_Reg_3052(0x58,0xEA);
    Wrt_Reg_3052(0x59,0xEB);
    Wrt_Reg_3052(0x60,0x05);
    Wrt_Reg_3052(0x61,0x05);
    Wrt_Reg_3052(0x65,0x0A);
    Wrt_Reg_3052(0x66,0x0A);
    Wrt_Reg_3052(0x80,0x05);
    Wrt_Reg_3052(0x81,0x00);
    Wrt_Reg_3052(0x82,0x02);
    Wrt_Reg_3052(0x83,0x04);
    Wrt_Reg_3052(0x84,0x00);
    Wrt_Reg_3052(0x85,0x00);
    Wrt_Reg_3052(0x86,0x1f);
    Wrt_Reg_3052(0x87,0x1f);
    Wrt_Reg_3052(0x88,0x0a);
    Wrt_Reg_3052(0x89,0x0c);
    Wrt_Reg_3052(0x8A,0x0e);
    Wrt_Reg_3052(0x8B,0x10);
    Wrt_Reg_3052(0x96,0x05);
    Wrt_Reg_3052(0x97,0x00);
    Wrt_Reg_3052(0x98,0x01);
    Wrt_Reg_3052(0x99,0x03);
    Wrt_Reg_3052(0x9A,0x00);
    Wrt_Reg_3052(0x9B,0x00);
    Wrt_Reg_3052(0x9C,0x1f);
    Wrt_Reg_3052(0x9D,0x1f);
    Wrt_Reg_3052(0x9E,0x09);
    Wrt_Reg_3052(0x9F,0x0b);
    Wrt_Reg_3052(0xA0,0x0d);
    Wrt_Reg_3052(0xA1,0x0f);
    Wrt_Reg_3052(0xB0,0x05);
    Wrt_Reg_3052(0xB1,0x1F);
    Wrt_Reg_3052(0xB2,0x03);
    Wrt_Reg_3052(0xB3,0x01);
    Wrt_Reg_3052(0xB4,0x00);
    Wrt_Reg_3052(0xB5,0x00);
    Wrt_Reg_3052(0xB6,0x1f);
    Wrt_Reg_3052(0xB7,0x00);
    Wrt_Reg_3052(0xB8,0x0f);
    Wrt_Reg_3052(0xB9,0x0d);
    Wrt_Reg_3052(0xBA,0x0b);
    Wrt_Reg_3052(0xBB,0x09);
    Wrt_Reg_3052(0xC6,0x05);
    Wrt_Reg_3052(0xC7,0x1F);
    Wrt_Reg_3052(0xC8,0x04);
    Wrt_Reg_3052(0xC9,0x02);
    Wrt_Reg_3052(0xCA,0x00);
    Wrt_Reg_3052(0xCB,0x00);
    Wrt_Reg_3052(0xCC,0x1f);
    Wrt_Reg_3052(0xCD,0x00);
    Wrt_Reg_3052(0xCE,0x10);
    Wrt_Reg_3052(0xCF,0x0e);
    Wrt_Reg_3052(0xD0,0x0c);
    Wrt_Reg_3052(0xD1,0x0a);
    Wrt_Reg_3052(0xFF,0x30);
    Wrt_Reg_3052(0xFF,0x52);
    Wrt_Reg_3052(0xFF,0x00);
    /**
     * 设置颜色色深
     * 3Ah 命令携带的参数即设置 5.3.3. vcom_adj:38H～3Ah 也设置 5.2.24. Interface Pixel Format(3AH)
     * 16bit x101,xxxx
     * 18bit x110,xxxx
     * 24bit x111,xxxx
     * 上面设置过一次 vcom_adj： Wrt_Reg_3052(0x3A,0x4F);
     * 所以这里设置色深要与 0x4F 进行或运算 0100.1111
     */
    //    Wrt_Reg_3052(0x3A,0x66); // 注释：应该是18bit 0110,0110
//            Wrt_Reg_3052(0x3A,0x50); // 从手册搞16bit显示是绿色乱码的
//        Wrt_Reg_3052(0x3A,0x50 | 0x4F); // 从手册搞16bit显示是绿色乱码的
    Wrt_Reg_3052(0x36,0x0A);//正扫0a,反扫09
    Wrt_Reg_3052(0x11,0x00);
    LCD_Delay(200);
    Wrt_Reg_3052(0x29,0x00);
    LCD_Delay(100);

#if 0
    uint8_t i_color = 0;
    for(int i = 0; i < 7 ; i++) {
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
        case 5:
            /* 发送白屏命令，此命令发送后，RGB刷彩色显示都是白色的 */
            WriteCommand(0x23);
            break;
        case 6:
            /* 关闭白屏命令，此命令发送后，RGB刷彩色显示都是黑色的  */
            WriteCommand(0x22);
        default :
            break;
        }
        i_color++;
        LCD_Delay(500);
    }
#endif
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
  * @param  x:      水平坐标，取值范围 0~479
  * @param  y：     垂直坐标，取值范围 0~271
  * @param  width:  图片的水平宽度，最大取值480
  * @param  height: 图片的垂直宽度，最大取值272
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

/**
  * @brief  使用DMA2D在坐标 (x,y) 起始处复制缓冲区到显示区
  * @note   1. 使用DMA2D实现
  *         2. 阻塞式
  *         3. 要绘制的区域不能超过屏幕的显示区域
  *         4. 可在 lv_port_disp.c 文件中，被函数 disp_flush() 调用，用以刷新显示区域
  * @param  x:      水平坐标，取值范围 0~479
  * @param  y：     垂直坐标，取值范围 0~271
  * @param  width:  图片的水平宽度，最大取值480
  * @param  height: 图片的垂直宽度，最大取值272
  * @param  color:  要复制的缓冲区地址
  * @retval None
  */
void LCD_CopyBuffer_IT(uint16_t x, uint16_t y, uint16_t width, uint16_t height,uint32_t *color)
{
    if(transferCompleteDetected == 0)
        return;

    DMA2D->CR     &= ~(DMA2D_CR_START);              //  停止DMA2D
    DMA2D->CR      = DMA2D_M2M;                      //  存储器到存储器

    DMA2D->FGMAR   = (uint32_t)color;                                       // 前景层内存地址
    DMA2D->OMAR    = LCD_MemoryAdd + BytesPerPixel_0*(LCD_Width * y + x);   // 输出区内存地址

    DMA2D->FGOR    = 0;                             // 前景层地址偏移
    DMA2D->OOR     = LCD_Width - width;             // 输出区地址偏移

    DMA2D->FGPFCCR = DMA2D_OUTPUT_RGB565;               // 前景层颜色格式 RGB565 */
    DMA2D->OPFCCR  = DMA2D_OUTPUT_RGB565;               // 输出区颜色格式 RGB565 */

    DMA2D->NLR     = (width<<16)|(height);              // 设定长度和宽度

    DMA2D->CR     |= DMA2D_IT_TC|DMA2D_IT_TE|DMA2D_IT_CE;   // 开启DMA2D中断
    DMA2D->CR     |= DMA2D_CR_START;                        // 启动DMA2D传输

    transferCompleteDetected = 0;                   // 设置传输完成标志
}

#if 0
/**
  * stm32 DMA2D使用中断LVGL,提高LVGL帧率，方式 4
  * https://blog.csdn.net/a2267542848/article/details/111163633
  */
static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    /*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/
    //LTDC_Color_Fill(area->x1,area->y1,area->x2,area->y2,(uint16_t*)(color_p));

    uint32_t h = area->y2 - area->y1;
    uint32_t w = area->x2 - area->x1;

    uint32_t OffLineSrc = LCD_WIDTH - (area->x2 - area->x1 +1);
    uint32_t addr = LCD_FRAME_BUF_ADDR + 2*(1024*area->y1 + area->x1);

    // -- 中断传输

    DMA2D->CR      = 0x00000000UL | (1 << 9);           // 模式
//    DMA2D->CR     &= ~(DMA2D_CR_START);                 // 停止DMA2D
//    DMA2D->CR      = DMA2D_M2M;                         // 存储器到存储器

    DMA2D->FGMAR   = (uint32_t)(uint16_t*)(color_p);    // 前景层内存地址
    DMA2D->OMAR    = (uint32_t)addr;                    // 输出区内存地址

    DMA2D->FGOR    = 0;                                 // 前景层地址偏移
    DMA2D->OOR     = OffLineSrc;                        // 输出区地址偏移

    DMA2D->FGPFCCR = DMA2D_OUTPUT_RGB565;               // 前景层颜色格式 RGB565 */
    DMA2D->OPFCCR  = DMA2D_OUTPUT_RGB565;               // 输出区颜色格式 RGB565 */

    DMA2D->NLR     = (area->y2-area->y1+1) | ((area->x2 -area->x1 +1) << 16); //  设定长度和宽度

    DMA2D->CR     |= DMA2D_IT_TC|DMA2D_IT_TE|DMA2D_IT_CE;   // 开启DMA2D中断
    DMA2D->CR     |= DMA2D_CR_START;                        // 启动DMA2D传输

    g_gpu_state = 1;
}
#endif

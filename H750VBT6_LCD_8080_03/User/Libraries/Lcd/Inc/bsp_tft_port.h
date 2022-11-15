/*
*********************************************************************************************************
*
*	模块名称 : TFT液晶显示器驱动模块
*	文件名称 : bsp_tft_port.h
*	版    本 : V2.0
*	说    明 : 头文件
*
*********************************************************************************************************
*/


#ifndef _BSP_TFT_PORT_H
#define _BSP_TFT_PORT_H

/* 定义LCD显示区域的分辨率 */
#define LCD_30_HEIGHT	240		/* 3.0寸宽屏 高度，单位：像素 */
#define LCD_30_WIDTH	400		/* 3.0寸宽屏 宽度，单位：像素 */

#define LCD_43_HEIGHT	272		/* 4.3寸宽屏 高度，单位：像素 */
#define LCD_43_WIDTH	480		/* 4.3寸宽屏 宽度，单位：像素 */

#define LCD_70_HEIGHT	480		/* 7.0寸宽屏 高度，单位：像素 */
#define LCD_70_WIDTH	800		/* 7.0寸宽屏 宽度，单位：像素 */


/**
 * 给DMA2D使用的，
 * 参考https://github.com/STMicroelectronics/STM32CubeL4//Drivers/BSP/STM32L496G-Discovery/stm32l496g_discovery.h
 */
/**
 * @brief LCD constroller Types Definition
 */
typedef struct
{
  __IO uint16_t REG;
  __IO uint16_t RAM;
} LCD_CONTROLLER_TypeDef;

/** @defgroup STM32L496G_DISCOVERY_FMC FMC LCD Constants
  * @{
  */
/* We use BANK1 as we use FMC_NE1 signal */
#define FMC_BANK1_BASE  ((uint32_t)(0x60000000 | 0x00000000))
//#define FMC_LCD_BASE    ((uint32_t)(0x60000000 | 0x00080000))  /*using A18*/
#define FMC_LCD_BASE    ((uint32_t)(0x60000000 | (1 << (19 + 1))))  /*using A19*/
#define FMC_BANK1_ADDR  ((LCD_CONTROLLER_TypeDef *) FMC_BANK1_BASE)
#define LCD_ADDR        ((LCD_CONTROLLER_TypeDef *) FMC_LCD_BASE)


/* 支持的驱动芯片ID */
enum
{
	IC_5420		= 0x5420,
	IC_4001		= 0x4001,
	IC_61509 	= 0xB509,
	IC_8875 	= 0x0075,
	IC_8876		= 0x0076,
	
	IC_9488 	= 0x9488
};

#define CHIP_STR_5420	"SPFD5420A"
#define CHIP_STR_4001	"OTM4001A"
#define CHIP_STR_61509	"R61509V"
#define CHIP_STR_8875	"RA8875"
#define CHIP_STR_9488	"ILI9488"
#define CHIP_STR_8876	"RA8876"

/* 背景光控制 */
#define BRIGHT_MAX		255
#define BRIGHT_MIN		0
#define BRIGHT_DEFAULT	200
#define BRIGHT_STEP		5

typedef struct
{
	void (*SetBackLight)(uint8_t _bright);
	uint8_t (*GetBackLight)(void);		
	void (*DispOn)(void);
	void (*DispOff)(void);
	void (*ClrScr)(uint16_t _usColor);
	void (*PutPixel)(uint16_t _usX, uint16_t _usY, uint16_t _usColor);
	uint16_t (*GetPixel)(uint16_t _usX, uint16_t _usY);
	void (*DrawLine)(uint16_t _usX1 , uint16_t _usY1 , uint16_t _usX2 , uint16_t _usY2 , uint16_t _usColor);
	void (*DrawRect)(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t _usColor);
	void (*DrawCircle)(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor);
	void (*DrawBMP)(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t *_ptr);
	void (*FillRect)(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t _usColor);	
	void (*FillCircle)(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor);
	void (*DrawHColorLine)(uint16_t _usX1 , uint16_t _usY1, uint16_t _usWidth, const uint16_t *_pColor);
}LCD_DEV_T;

/* 可供外部模块调用的函数 */
void LCD_InitHard(void);
void LCD_GetChipDescribe(char *_str);
uint16_t LCD_GetHeight(void);
uint16_t LCD_GetWidth(void);
void LCD_DispOn(void);
void LCD_DispOff(void);

void LCD_ClrScr(uint16_t _usColor);
void LCD_PutPixel(uint16_t _usX, uint16_t _usY, uint16_t _usColor);
uint16_t LCD_GetPixel(uint16_t _usX, uint16_t _usY);
void LCD_DrawLine(uint16_t _usX1 , uint16_t _usY1 , uint16_t _usX2 , uint16_t _usY2 , uint16_t _usColor);
void LCD_DrawRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t _usColor);
void LCD_DrawCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor);
void LCD_DrawBMP(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t *_ptr);
void LCD_SetBackLight(uint8_t _bright);
uint8_t LCD_GetBackLight(void);
void LCD_Fill_Rect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t _usColor);
void LCD_SetDirection(uint8_t _dir);

void SOFT_FillCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor);
void SOFT_FillQuterCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor, uint8_t _ucMode);
void SOFT_DrawCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor);

void SOFT_DrawRoundRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, 
	uint16_t _usRadius, uint16_t _usColor);
void SOFT_FillRoundRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, 
	uint16_t _usRadius, uint16_t _usColor);	

/* 下面3个变量，主要用于使程序同时支持不同的屏 */
extern uint16_t g_ChipID;			/* 驱动芯片ID */
extern uint16_t g_LcdHeight;		/* 显示屏分辨率-高度 */
extern uint16_t g_LcdWidth;			/* 显示屏分辨率-宽度 */
extern uint8_t g_LcdDirection;		/* 显示方向.0，1，2，3 */

extern LCD_DEV_T g_tLCD;

#endif



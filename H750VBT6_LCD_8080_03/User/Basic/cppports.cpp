/*
 * cppports.cpp
 *
 *  Created on: May 16, 2022
 *      Author: OldGerman
 */

#include "cppports.h"
#include "bsp.h"

extern "C" {
#include "GUI.h"
#include "RGB565_240x160.h"
#include "240x160.h"
#include "480x320_MB1261_FAN_OUT.h"
#include "img1.h"
#include "dma2d.h"

#include "../Share/Libraries/lvgl/lvgl.h"
#include "hal_stm_lvgl/tft/tft.h"
#include "hal_stm_lvgl/touchpad/touchpad.h"
#include "../Share/Libraries/lvgl/demos/lv_demos.h"
}

//	static void MIX_Update();
//	void MIX_Update()
//	{
//		;
//	}

#ifndef DBG_PRINT
#if 0  //< Change 0 to 1 to open debug macro and check program debug information
#define DBG_PRINT usb_printf
#else
	#define DBG_PRINT(...)
	#endif
#endif


static void PrintfInfo(void)
{
	printf("*************************************************************\n\r");

	/* 检测CPU ID */
	{
		uint32_t CPU_Sn0, CPU_Sn1, CPU_Sn2;

		CPU_Sn0 = *(__IO uint32_t*)(0x1FF1E800);
		CPU_Sn1 = *(__IO uint32_t*)(0x1FF1E800 + 4);
		CPU_Sn2 = *(__IO uint32_t*)(0x1FF1E800 + 8);

		printf("CPU : STM32H750VBT6, LQFP100, 主频: %ldMHz\r\n", SystemCoreClock / 1000000);
		printf("UID = %08X %08X %08X\n\r", (unsigned int)CPU_Sn2, (unsigned int)CPU_Sn1, (unsigned int)CPU_Sn0);
	}
}

static void PrintfHelp(void)
{
	printf("*************************************************************\n\r");
	printf("DMA2D 搬运 QSPI FLASH 的 c bitmap 到 FMC 驱动的 3.5寸屏幕显示测试:\r\n\r\n");
	printf("\r\n");
}

static void timerCallBack(){
	HAL_GPIO_TogglePin(LRGB_R_GPIO_Port, LRGB_R_Pin);
}

void btA_CLICKED_func(){
	printf("检测到 KEY A 短按\r\n");
	HAL_GPIO_TogglePin(LRGB_R_GPIO_Port, LRGB_R_Pin);
}

void btB_CLICKED_func(){
	printf("检测到 KEY B 短按\r\n");
	HAL_GPIO_TogglePin(LRGB_R_GPIO_Port, LRGB_R_Pin);
	bsp_StartHardTimer(TIM_CHANNEL_1, 10, timerCallBack);
}

void btA_LONG_PRESSED_func(){
;
}

void btB_LONG_PRESSED_func(){
;
}

static void DMA2D_Config(void)
{
	  HAL_StatusTypeDef hal_status = HAL_OK;

	  /* Configure the DMA2D Mode, Color Mode and output offset */
	  hdma2d.Init.Mode         = DMA2D_M2M; /* DMA2D Mode memory to memory */
	  hdma2d.Init.ColorMode    = DMA2D_OUTPUT_RGB565; /* Output color mode is RGB565 : 16 bpp */
	  hdma2d.Init.OutputOffset = 0x0; /* No offset in output */
	  hdma2d.Init.RedBlueSwap   = DMA2D_RB_REGULAR;      /* No R&B swap for the output image */
	  hdma2d.Init.AlphaInverted = DMA2D_REGULAR_ALPHA;   /* No alpha inversion for the output image */


	  /* Foreground Configuration : Layer 1 */
	  hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
	  hdma2d.LayerCfg[1].InputAlpha = 0xFF; /* Fully opaque */
	  hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB565; /* Foreground layer format is RGB565 : 16 bpp */
	  hdma2d.LayerCfg[1].InputOffset = 0x0; /* No offset in input */
	  hdma2d.LayerCfg[1].RedBlueSwap   = DMA2D_RB_REGULAR;      /* No R&B swap for the input foreground image */
	  hdma2d.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA;   /* No alpha inversion for the input foreground image */

	  hdma2d.Instance = DMA2D;

	  /* DMA2D Initialization */
	  hal_status = HAL_DMA2D_Init(&hdma2d);

	  hal_status = HAL_DMA2D_ConfigLayer(&hdma2d, 1);
}


static void LCD_ImagePreparation(uint16_t x0, uint16_t y0, uint16_t xSize, uint16_t ySize)
{
  	/*
  	 ---------------->---
  	|(_usX，_usY)        |
  	V                    V  _usHeight
  	|                    |
  	 ---------------->---
  		  _usWidth
  	*/

  	ILI9488_SetDispWin(x0, y0, ySize, xSize);
//  	FMC_BANK1_ADDR->REG = 0x2C;
	ILI9488_REG = 0x2C; 			/* 准备读写显存 */
}

void setup(){
	bsp_Init();
	PrintfInfo();
	PrintfHelp();
	HAL_GPIO_WritePin(LRGB_R_GPIO_Port, LRGB_R_Pin, GPIO_PIN_RESET);
//	bsp_tim6_enable_IT();	//全程保持LRGB_B闪烁
	LCD_ClrScr(0xfff);  		/* 清屏，背景蓝色 */
	HAL_Delay(500);
}

void loop(){
#if 0
	while (1) {
		uint16_t x = 0;
		uint16_t y = 0;
		for(int i = 0; i < 8; i++){
			for(int j = 0; j < 8; j++) {
				ILI9488_SetDispWin(0, 0, 320, 480);
				ILI9488_REG = 0x2C;
				DMA2D_Config(240);	//为啥需要传入被刷区域长度以设置 hdma2d.Init.OutputOffset
				uint32_t address = ILI9488_GetCursorAddress(x + i, y + j);
				HAL_DMA2D_Start_IT(&hdma2d,
							   (uint32_t)&RGB565_240x160,               /* Color value in Register to Memory DMA2D mode */
							   address,                              /* DMA2D output buffer */
							   240,              /* width of buffer in pixels */
							   160);            /* height of buffer in pixels */
				HAL_Delay(300);
			}
		}
	}
#elif 0
	while (1) {
		uint16_t x = 0;
		uint16_t y = 0;
		for(int i = 0; i < 8; i++){
			for(int j = 0; j < 8; j++) {
				LCD_ImagePreparation(x+i, y+j, LAYER_SIZE_X, LAYER_SIZE_Y);
				DMA2D_Config();
				HAL_DMA2D_Start_IT(&hdma2d,
						(uint32_t)&RGB565_240x160,    /* Source buffer in format RGB565 and size 240x160                 */
						(uint32_t)0x60100000,   /* LCD data address                                                */
						LAYER_SIZE_X,								//强制每行 1 个像素，
						LAYER_SIZE_Y); //以像素为单位的宽度 x 以像素为单位的高度作为行数，以将 DMA2D 传输与 LCD 配置对齐
				HAL_Delay(300);
			}
		}
	}
#elif 1

    lv_init();
    tft_init();
//    touchpad_init();

//    lv_demo_benchmark();
    lv_demo_music();
//    lv_demo_stress();
//    lv_demo_widgets();

    while (1)
    {
//     checkflush();
//   	 HAL_Delay(5);
   	 lv_task_handler();
    }
#endif
}
#if 0
void BSP_LED_On()
{
	HAL_GPIO_TogglePin(LRGB_B_GPIO_Port, LRGB_B_Pin);
}
/**
  * @brief  DMA2D Transfer completed callback
  * @param  hdma2d: DMA2D handle.
  * @note   This example shows a simple way to report end of DMA2D transfer, and
  *         you can add your own implementation.
  * @retval None
  */
static void TransferComplete(DMA2D_HandleTypeDef *hdma2d)
{
  /* Turn LED2 On */
//  BSP_LED_On(LED2);
  /* The Blended image is now ready for display */
//  blended_image_ready = 1;
}

/**
  * @brief  DMA2D error callbacks
  * @param  hdma2d: DMA2D handle
  * @note   This example shows a simple way to report DMA2D transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
static void TransferError(DMA2D_HandleTypeDef *hdma2d)
{
  /* Turn LED1 On */
//  BSP_LED_On(LED1);
}

/**
  * @brief  On Error Handler on condition TRUE.
  * @param  condition : Can be TRUE or FALSE
  * @retval None
  */
static void OnError_Handler(uint32_t condition)
{
  if(condition)
  {
//    BSP_LED_On(LED1);
    	HAL_GPIO_TogglePin(LRGB_B_GPIO_Port, LRGB_B_Pin);
    	HAL_Delay(500);
    	HAL_GPIO_TogglePin(LRGB_B_GPIO_Port, LRGB_B_Pin);
    	HAL_Delay(500);
 /* Blocking on error */
  }
}
/**
  * @brief DMA2D configuration.
  * @note  This function Configure the DMA2D peripheral :
  *        1) Configure the transfer mode : memory to memory
  *        2) Configure the output color mode as RGB565
  *        3) Configure the transfer from FLASH to SRAM
  *        4) Configure the data size : 240x160 (pixels)
  * @retval
  *  None
  */
void DMA2D_Config(void)
{
	  HAL_StatusTypeDef hal_status = HAL_OK;

	  /* Configure the DMA2D Mode, Color Mode and output offset */
	  hdma2d.Init.Mode         = DMA2D_M2M; /* DMA2D Mode memory to memory */
	  hdma2d.Init.ColorMode    = DMA2D_OUTPUT_RGB565; /* Output color mode is RGB565 : 16 bpp */
	  hdma2d.Init.OutputOffset = 0x0; /* No offset in output */
	  hdma2d.Init.RedBlueSwap   = DMA2D_RB_REGULAR;      /* No R&B swap for the output image */
	  hdma2d.Init.AlphaInverted = DMA2D_REGULAR_ALPHA;   /* No alpha inversion for the output image */

	  /* DMA2D Callbacks Configuration */
	  hdma2d.XferCpltCallback  = TransferComplete;
	  hdma2d.XferErrorCallback = TransferError;

	  /* Foreground Configuration : Layer 1 */
	  hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
	  hdma2d.LayerCfg[1].InputAlpha = 0xFF; /* Fully opaque */
	  hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB565; /* Foreground layer format is RGB565 : 16 bpp */
	  hdma2d.LayerCfg[1].InputOffset = 0x0; /* No offset in input */
	  hdma2d.LayerCfg[1].RedBlueSwap   = DMA2D_RB_REGULAR;      /* No R&B swap for the input foreground image */
	  hdma2d.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA;   /* No alpha inversion for the input foreground image */

	  hdma2d.Instance = DMA2D;

	  /* DMA2D Initialization */
	  hal_status = HAL_DMA2D_Init(&hdma2d);
	  OnError_Handler(hal_status != HAL_OK);

	  hal_status = HAL_DMA2D_ConfigLayer(&hdma2d, 1);
	  OnError_Handler(hal_status != HAL_OK);
}

/**
  * @brief LCD image preparation.
  * @note  Configure image position and size for Discovery STM32L496 LCD
  *        and set LCD in pixel writing mode.
  *        This API must be invoked before transferring the image data to the LCD.
  * @param  x0: first pixel x position
  * @param  y0: first pixel y position
  * @param  xSize: image width (in pixels)
  * @param  ySize: image height (in pixels)
  * @retval None
  */
static void LCD_ImagePreparation(uint16_t x0, uint16_t y0, uint16_t xSize, uint16_t ySize)
{
  	/*
  	 ---------------->---
  	|(_usX，_usY)        |
  	V                    V  _usHeight
  	|                    |
  	 ---------------->---
  		  _usWidth
  	*/

  	ILI9488_SetDispWin(x0, y0, ySize, xSize);
//  	FMC_BANK1_ADDR->REG = 0x2C;
	ILI9488_REG = 0x2C; 			/* 准备读写显存 */
}

/*
*********************************************************************************************************
*	函 数 名: _DMA2D_Copy
*	功能说明: 通过DMA2D从前景层复制指定区域的颜色数据到目标区域
*	形    参: pSrc          颜色数据源地址
*             pDst          颜色数据目的地址
*             xSize         目的区域的X轴大小，即每行像素数
*             ySize         目的区域的Y轴大小，即行数
*             OffLineSrc    前景层图像的行偏移
*             OffLineDst    输出的行偏移
*             PixelFormat   目标区颜色格式
*	返 回 值: 无
*********************************************************************************************************
*/
static void _DMA2D_Copy(void * pSrc,
	                    void * pDst,
						uint32_t xSize,
						uint32_t ySize,
						uint32_t OffLineSrc,
						uint32_t OffLineDst)
{

	/* DMA2D采用存储器到存储器模式, 这种模式是前景层作为DMA2D输入 */
	DMA2D->CR      = 0x00000000UL | (1 << 9);
	DMA2D->FGMAR   = (uint32_t)pSrc;
	DMA2D->OMAR    = (uint32_t)pDst;
	DMA2D->FGOR    = OffLineSrc;
	DMA2D->OOR     = OffLineDst;

	/* 前景层和输出区域都采用的RGB565颜色格式 */
	DMA2D->FGPFCCR = DMA2D_INPUT_RGB565;
	DMA2D->OPFCCR  = DMA2D_INPUT_RGB565;

	DMA2D->NLR     = (uint32_t)(xSize << 16) | (uint16_t)ySize;

	/* 启动传输 */
	DMA2D->CR   |= DMA2D_CR_START;

	/* 等待DMA2D传输完成 */
	while (DMA2D->CR & DMA2D_CR_START) {}
}
#endif


/* Demo:
	*************************************************************
	CPU : STM32H750VBT6, LQFP100, 主频: 400MHz
	UID = 32363235 31305114 001F002C
	*************************************************************
 */


/***
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
//DMA2D_HandleTypeDef hdma2d;	// DMA2D句柄
//LTDC_HandleTypeDef hltdc;		// LTDC句柄


//LCD相关参数结构体
struct	
{
	uint32_t Color;  				//	LCD当前画笔颜色
	uint32_t BackColor;			//	背景色
	uint32_t ColorMode;			// 颜色格式
	uint32_t LayerMemoryAdd;	//	层显存地址
	uint8_t  Layer; 				//	当前层
	uint8_t  Direction;			//	显示方向
	uint8_t  BytesPerPixel;		// 每个像素所占字节数
	uint8_t  ShowNum_Mode;		// 数字显示模式
}LCD;




/*************************************************************************************************
*	函 数 名:	MX_LTDC_Init
*	入口参数:	无
*	返 回 值:	无
*	函数功能:	初始化LTDC引脚的IO口、全局参数、层设置等
*	说    明:	无			
*************************************************************************************************/
#if 0
void MX_LTDC_Init(void)
{
	LTDC_LayerCfgTypeDef pLayerCfg = {0};		// layer0 相关参数
	__HAL_RCC_DMA2D_CLK_ENABLE();					// 使能DMA2D时钟

	hltdc.Instance 		 = LTDC;
	hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;		// 低电平有效
	hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;     // 低电平有效
	hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;     // 低电平有效，要注意的是，很多面板都是高电平有效，但是750需要设置成低电平才能正常显示
	hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;    // 正常时钟信号
  
	hltdc.Init.HorizontalSync 		= HSW - 1;									// 根据屏幕设置参数即可
	hltdc.Init.VerticalSync 		= VSW	-1 ;
	hltdc.Init.AccumulatedHBP		= HBP + HSW -1;
	hltdc.Init.AccumulatedVBP 		= VBP + VSW -1;
	hltdc.Init.AccumulatedActiveW = LCD_Width  + HSW + HBP -1;
	hltdc.Init.AccumulatedActiveH = LCD_Height + VSW + VBP -1;
	hltdc.Init.TotalWidth 			= LCD_Width  + HSW + HBP + HFP - 1; 
	hltdc.Init.TotalHeigh 			= LCD_Height + VSW + VBP + VFP - 1;
	
	hltdc.Init.Backcolor.Red 		= 0;	// 初始背景色，R	
	hltdc.Init.Backcolor.Green 	= 0;  // 初始背景色，G	
	hltdc.Init.Backcolor.Blue 		= 0;	// 初始背景色，B
	
	HAL_LTDC_Init(&hltdc);	// 初始化LTDC参数

/*---------------------------------- layer0 显示配置 --------------------------------*/

	pLayerCfg.WindowX0 			= 0;										// 水平起点
	pLayerCfg.WindowX1 			= LCD_Width;							// 水平终点
	pLayerCfg.WindowY0 			= 0;										// 垂直起点
	pLayerCfg.WindowY1 			= LCD_Height;							// 垂直终点
	pLayerCfg.ImageWidth 		= LCD_Width;                     // 显示区域宽度
	pLayerCfg.ImageHeight 		= LCD_Height;                    // 显示区域高度	
	pLayerCfg.PixelFormat 		= ColorMode_0;							// 颜色格式	

// 配置 layer0 的恒定透明度，最终写入 LTDC_LxCACR 寄存器 
//	需要注意的是，这个参数是直接配置整个 layer0 的透明度，这里设置为255即不透明 
	pLayerCfg.Alpha 				= 255;									// 取值范围0~255，255表示不透明，0表示完全透明

// 设置 layer0 的层混合系数，最终写入 LTDC_LxBFCR 寄存器 
// 该参数用于设置 layer0 和 底层背景 之间的颜色混合系数，计算公式为 ：
// 混合后的颜色 =  BF1 * layer0的颜色 + BF2 * 底层背景的颜色
// 如果 layer0 使用了透明色，则必须配置成 LTDC_BLENDING_FACTOR1_PAxCA 和 LTDC_BLENDING_FACTOR2_PAxCA，否则ARGB中的A通道不起作用
//	更多的介绍可以查阅 参考手册关于 LTDC_LxBFCR 寄存器的介绍
	pLayerCfg.BlendingFactor1 	= LTDC_BLENDING_FACTOR1_CA;		// 混合系数1
	pLayerCfg.BlendingFactor2 	= LTDC_BLENDING_FACTOR2_CA;      // 混合系数2
	
	pLayerCfg.FBStartAdress 	= LCD_MemoryAdd;                 // 显存地址

// 配置 layer0 的初始默认颜色，包括A,R,G,B 的值 ，最终写入 LTDC_LxDCCR 寄存器 
	pLayerCfg.Alpha0 				= 0;			// 初始颜色，A
	pLayerCfg.Backcolor.Blue 	= 0;        //	初始颜色，R
	pLayerCfg.Backcolor.Green 	= 0;        //	初始颜色，G
	pLayerCfg.Backcolor.Red 	= 0;			//	初始颜色，B 
  
	HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, 0);		// 配置layer0

#if ( ( ColorMode_0 == LTDC_PIXEL_FORMAT_RGB888 )||( ColorMode_0 == LTDC_PIXEL_FORMAT_ARGB8888 ) ) // 判断是否使用24位或者32位色

// 因为750每个通道的低位都是采用伪随机抖动输出，如果不开启颜色抖动，则无法正常显示24位或者32位色

	HAL_LTDC_EnableDither(&hltdc); // 开启颜色抖动
	
#endif


/*---------------------------------- layer1 显示配置 --------------------------------*/

#if ( LCD_NUM_LAYERS == 2 )	//如果定义了双层
  
	LTDC_LayerCfgTypeDef pLayerCfg1 = {0};
  
	pLayerCfg1.WindowX0 				= 0;							// 水平起点
	pLayerCfg1.WindowX1 				= LCD_Width;				// 水平终点
	pLayerCfg1.WindowY0 				= 0;							// 垂直起点
	pLayerCfg1.WindowY1 				= LCD_Height;				// 垂直终点
	pLayerCfg1.ImageWidth 			= LCD_Width;         	// 显示区域宽度
	pLayerCfg1.ImageHeight 			= LCD_Height;        	// 显示区域高度		
	pLayerCfg1.PixelFormat 			= ColorMode_1;				// 颜色格式，layer1 应配置为带有透明色的格式，例如ARGB8888或ARGB1555

// 配置 layer1 的恒定透明度，最终写入 LTDC_LxCACR 寄存器 
//	需要注意的是，这个参数是直接配置整个 layer1 的透明度，这里设置为255即不透明 
	pLayerCfg1.Alpha 					= 255;			// 取值范围0~255，255表示不透明，0表示完全透明
	
// 设置 layer1 的层混合系数，最终写入 LTDC_LxBFCR 寄存器 
// 该参数用于设置 layer1 和 (layer0+背景）之间的颜色混合系数，计算公式为 ：
// 混合后的颜色 =  BF1 * layer1的颜色 + BF2 * (layer0+背景混合后的颜色）
// 如果 layer1 使用了透明色，则必须配置成 LTDC_BLENDING_FACTOR1_PAxCA 和 LTDC_BLENDING_FACTOR2_PAxCA，否则ARGB中的A通道不起作用
//	更多的介绍可以查阅 参考手册关于 LTDC_LxBFCR 寄存器的介绍
	pLayerCfg1.BlendingFactor1 	= LTDC_BLENDING_FACTOR1_PAxCA;			// 混合系数1
	pLayerCfg1.BlendingFactor2 	= LTDC_BLENDING_FACTOR2_PAxCA;      	// 混合系数2
	
	pLayerCfg1.FBStartAdress 		= LCD_MemoryAdd + LCD_MemoryAdd_OFFSET; // 显存地址
	

// 配置layer1 的初始默认颜色，包括A,R,G,B 的值 ，最终写入 LTDC_LxDCCR 寄存器 
	pLayerCfg1.Alpha0 				= 0;				// 初始颜色，A
	pLayerCfg1.Backcolor.Red 		= 0;				//	初始颜色，R
	pLayerCfg1.Backcolor.Green 	= 0;           //	初始颜色，G
	pLayerCfg1.Backcolor.Blue 		= 0;           //	初始颜色，B 
	
	HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg1, 1);	// 初始化 layer1 的配置

	#if ( ( ColorMode_1 == LTDC_PIXEL_FORMAT_RGB888 )||( ColorMode_1 == LTDC_PIXEL_FORMAT_ARGB8888 ) ) // 判断是否使用24位或者32位色

	// 因为750每个通道的低位都是采用伪随机抖动输出，如果不开启颜色抖动，则无法正常显示24位或者32位色
	
		HAL_LTDC_EnableDither(&hltdc); // 开启颜色抖动
		
	#endif

#endif

	HAL_LTDC_ProgramLineEvent(&hltdc, 0 );			// 设置行中断，第0行
	HAL_NVIC_SetPriority(LTDC_IRQn, 0xE, 0);		// 设置优先级
	HAL_NVIC_EnableIRQ(LTDC_IRQn);					// 使能中断
	
// LTDC在初始化之后，上电的瞬间会有一个短暂的白屏，
//	即使一开始就将背光引脚拉低并且屏幕背光引脚用电阻下拉还是会有这个现象，
//	如果需要消除这个现象，可以在初始化完毕之后，进行一个短暂的延时再打开背光
//
//	HAL_Delay(200);	// 延时200ms

	LCD_Backlight_ON;							// 开启背光	

}
#endif

/***************************************************************************************************************************************
*	函 数 名: LCD_CopyBuffer
*
*	入口参数: x - 水平坐标，取值范围 0~479
*			 	 y - 垂直坐标，取值范围 0~271
*			 	 width  - 图片的水平宽度，最大取值480
*				 height - 图片的垂直宽度，最大取值272
*				 *color - 要复制的缓冲区地址
*				
*	函数功能: 在坐标 (x,y) 起始处复制缓冲区到显示区
*
*	说    明: 1. 使用DMA2D实现
*				 2. 要绘制的区域不能超过屏幕的显示区域
*				 3. 在 lv_port_disp.c 文件中，被函数 disp_flush() 调用，用以刷新显示区域			 
*
*****************************************************************************************************************************************/

void LCD_CopyBuffer(uint16_t x, uint16_t y, uint16_t width, uint16_t height,uint32_t *color)
{

	DMA2D->CR	  &=	~(DMA2D_CR_START);				//	停止DMA2D
	DMA2D->CR		=	DMA2D_M2M;							//	存储器到存储器
	DMA2D->FGPFCCR	=	LTDC_PIXEL_FORMAT_RGB565;		//	设置颜色格式
   DMA2D->FGOR    =  0;										// 
	DMA2D->OOR		=	LCD_Width - width;				//	设置行偏移 	
	DMA2D->FGMAR   =  (uint32_t)color;		
	DMA2D->OMAR		=	LCD_MemoryAdd + BytesPerPixel_0*(LCD_Width * y + x);	// 地址;
	DMA2D->NLR		=	(width<<16)|(height);			//	设定长度和宽度		
	DMA2D->CR	  |=	DMA2D_CR_START;					//	启动DMA2D
		
	while (DMA2D->CR & DMA2D_CR_START) ;			//	等待传输完成

}


/**************************************************************************************************************************************************************************************************************************************************************************FANKE***/
// 实验平台：反客 STM32H750核心板
//


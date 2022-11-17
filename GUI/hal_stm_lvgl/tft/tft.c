/**
 * @file tft.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_conf.h"
#include "../GUI/lvgl/lvgl.h"
#include <string.h>
#include <stdlib.h>

#include "tft.h"
#include "stm32h7xx.h"
#include "bsp.h"
#include "dma2d.h"

struct debugArray {
	uint32_t x1;
	uint32_t y1;
	uint32_t x2;
	uint32_t y2;
	uint32_t width;
	uint32_t height;
}DebugArray;


//#include "stm32h7b3i_discovery.h"
//#include "stm32h7b3i_discovery_lcd.h"
//#include "stm32h7b3i_discovery_sdram.h"
//#include "stm32h7b3i_discovery_ts.h"
//#include "../Components/rk043fn48h/rk043fn48h.h"	//RGB 4.3Inch
static void LCD_ImagePreparation(uint16_t x0, uint16_t y0, uint16_t xSize, uint16_t ySize);
static void LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

__IO uint32_t TIMESTAMP_LCD_TE = 0;
/*********************
 *      DEFINES
 *********************/
#if LV_COLOR_DEPTH != 16 && LV_COLOR_DEPTH != 24 && LV_COLOR_DEPTH != 32
#error LV_COLOR_DEPTH must be 16, 24, or 32
#endif

/**********************
 *      TYPEDEFS
 **********************/
#if LV_USE_GPU_STM32_DMA2D
__IO uint32_t   transferCompleteDetected = 0;  /* DMA2D Transfer Complete flag */
HAL_StatusTypeDef HAL_Status = HAL_OK;
#endif
/**********************
 *  STATIC PROTOTYPES
 **********************/
/*These 3 functions are needed by LittlevGL*/
static void ex_disp_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t * color_p);
static void ex_disp_clean_dcache(lv_disp_drv_t *drv);
void my_rounder_cb(lv_disp_drv_t * disp_drv, lv_area_t * area);

#if LV_USE_GPU_STM32_DMA2D
static void DMA2D_Config(uint32_t xSize);
static void TransferComplete(DMA2D_HandleTypeDef *hdma2d);
static void TransferError(DMA2D_HandleTypeDef *hdma2d);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_disp_drv_t disp_drv;
lv_disp_drv_t * ptr_disp_drv = &disp_drv;

#if LV_COLOR_DEPTH == 16
typedef uint16_t uintpixel_t;
#elif LV_COLOR_DEPTH == 24 || LV_COLOR_DEPTH == 32
typedef uint32_t uintpixel_t;
#endif

static lv_disp_t *our_disp = NULL;

#if LV_USE_GPU_STM32_DMA2D
//static int32_t            x1_flush;
//static int32_t            y1_flush;
//static int32_t            x2_flush;
//static int32_t            y2_fill;
//static int32_t            y_fill_act;
static const lv_color_t * buf_to_flush;
#endif


//static lv_color_t buf1_1[TFT_HOR_RES * TFT_VER_RES] __attribute__((section(".RAM_D1_Array")));
static lv_color_t buf1_1[TFT_HOR_RES * TFT_VER_RES / 2] __attribute__((section(".RAM_D1_Array")));
static lv_color_t buf1_2[TFT_HOR_RES * TFT_VER_RES /2] __attribute__((section(".RAM_D1_Array")));

/**********************
 *      MACROS
 **********************/

/**
 * Initialize your display here
 */

void tft_init(void)
{
//	LCD_ImagePreparation(0,0, 480, 320);
	ILI9488_QuitWinMode();	//对全屏开窗口

	/* There is only one display on STM32 */
	if(our_disp != NULL)
		abort();

//    BSP_LCD_Init(0,LCD_ORIENTATION_LANDSCAPE);
//	BSP_LCD_DisplayOn(0);

   /*-----------------------------
	* Create a buffer for drawing
	*----------------------------*/

   /* LittlevGL requires a buffer where it draws the objects. The buffer's has to be greater than 1 display row*/
#if 1
	static lv_disp_draw_buf_t disp_buf_1;
	lv_disp_draw_buf_init(&disp_buf_1, buf1_1, buf1_2, TFT_HOR_RES * TFT_VER_RES / 2);   /*Initialize the display buffer*/
#else
	static lv_disp_draw_buf_t disp_buf_1;
	lv_disp_draw_buf_init(&disp_buf_1, buf1_1, NULL, TFT_HOR_RES * TFT_VER_RES);   /*Initialize the display buffer*/
#endif


	/*-----------------------------------
	* Register the display in LittlevGL
	*----------------------------------*/

	lv_disp_drv_init(&disp_drv);                    /*Basic initialization*/

	/*Set up the functions to access to your display*/

	/*Set the resolution of the display*/
	disp_drv.hor_res = TFT_HOR_RES;
	disp_drv.ver_res = TFT_VER_RES;

	 /*Required for Example 3)*/
//	disp_drv.full_refresh = 1;	//单缓冲全尺寸也可以用，就是tnnd没有全屏刷新，局部刷新对齐？导致的撕裂

	/*Used to copy the buffer's content to the display*/
	disp_drv.flush_cb = ex_disp_flush;

	/*
	 * https://docs.lvgl.io/master/porting/display.html#draw-buffer
	 * 对要重绘的区域坐标进行四舍五入。例如，2x2 像素可以转换为 2x8。
	 * 如果显示控制器只能刷新具有特定高度或宽度的区域（通常为 8 px 高度，单色显示器），则可以使用它。
	 */
	disp_drv.rounder_cb = my_rounder_cb;

	//软件旋转屏幕
//	disp_drv.sw_rotate = 1;   // add for rotation
//	disp_drv.rotated = LV_DISP_ROT_90;   // add for rotation


	/* 可选：当 lvgl 需要任何影响渲染的 CPU 缓存被清理时调用 */
	disp_drv.clean_dcache_cb = ex_disp_clean_dcache;

	/*Set a display buffer*/
	disp_drv.draw_buf = &disp_buf_1;

	/*Finally register the driver*/
	our_disp = lv_disp_drv_register(&disp_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

void my_rounder_cb(lv_disp_drv_t * disp_drv, lv_area_t * area)
{
	  /* Update the areas as needed.
	   * For example it makes the area to start only on 8th rows and have Nx8 pixel height.
	   * 例如，它使该区域仅从第 8 行开始，并且具有 Nx8 像素高度。*/
	   area->y1 = area->y1 & 0xfff8;	//1111,1111,1111,1000
	   area->x1 = area->x1 & 0xfff8;
	   area->y2 = (area->y2 & 0xfff8) + 8 - 1;
	   area->x2 = (area->x2 & 0xfff8) + 8 - 1;
}

/* Flush the content of the internal buffer the specific area on the display
 * You can use DMA or any hardware acceleration to do this operation in the background but
 * 'lv_flush_ready()' has to be called when finished
 * This function is required only when LV_VDB_SIZE != 0 in lv_conf.h*/
static void ex_disp_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t * color_p)
{
#if LV_USE_GPU_STM32_DMA2D
	    int32_t x1 = area->x1;
		int32_t x2 = area->x2;
		int32_t y1 = area->y1;
		int32_t y2 = area->y2;
		/*Return if the area is out the screen*/

		if(x2 < 0) return;
		if(y2 < 0) return;
		if(x1 > TFT_HOR_RES - 1) return;
		if(y1 > TFT_VER_RES - 1) return;

		/*Truncate the area to the screen*/
//		int32_t act_x1 = x1 < 0 ? 0 : x1;
//		int32_t act_y1 = y1 < 0 ? 0 : y1;
//		int32_t act_x2 = x2 > TFT_HOR_RES - 1 ? TFT_HOR_RES - 1 : x2;
//		int32_t act_y2 = y2 > TFT_VER_RES - 1 ? TFT_VER_RES - 1 : y2;
//
//		x1_flush = act_x1;
//		y1_flush = act_y1;
//		x2_flush = act_x2;
//		y2_fill = act_y2;
//		y_fill_act = act_y1;


		buf_to_flush = color_p;

		/*
			 * https://forum.lvgl.io/t/stm32-dma2d-fsmc-lvgl/4317/3
			 * SCB_CleanInvalidateDCache();
			 * 在写入显示器之前调用。您需要使缓存无效，以便 DMA2D 的更改反映在 CPU 中
			 */
		SCB_CleanInvalidateDCache();
		SCB_InvalidateICache();

#if 0
		uint32_t address =
				hlcd_ltdc.LayerCfg[Lcd_Ctx[0].ActiveLayer].FBStartAdress
				+
				(
						(
								(Lcd_Ctx[0].XSize*area->y1)
								+
								area->x1
						)
								*
								Lcd_Ctx[0].BppFactor
				);
#endif
		LCD_ImagePreparation(area->x1, area->y1, lv_area_get_width(area), lv_area_get_height(area));
//		LCD_Address_Set(area->x1, area->y1, area->x2, area->y2);
//		LCD_ImagePreparation(0,0, TFT_HOR_RES, TFT_VER_RES);
//		ILI9488_REG = 0x2C; 			/* 准备读写显存 */
		DMA2D_Config(lv_area_get_width(area));	//为啥需要传入被刷区域长度以设置 hdma2d.Init.OutputOffset
#if 1
//		if( ((HAL_GetTick() - TIMESTAMP_LCD_TE < 7) || TIMESTAMP_LCD_TE == 0)) {
//			HAL_GPIO_TogglePin(LRGB_R_GPIO_Port, LRGB_R_Pin);
//			HAL_GPIO_TogglePin(LRGB_R_GPIO_Port, LRGB_R_Pin);

			HAL_Status = HAL_DMA2D_Start_IT(&hdma2d,
										   (uint32_t)buf_to_flush,               /* Color value in Register to Memory DMA2D mode */
										   (uint32_t)0x60100000,                              /* DMA2D output buffer */
										   lv_area_get_width(area),              /* width of buffer in pixels */
										   lv_area_get_height(area));            /* height of buffer in pixels */
//			 transferCompleteDetected = 0;
//		}
#elif 0
		HAL_Status = HAL_DMA2D_Start_IT(&hdma2d,
									   (uint32_t)buf_to_flush,               /* Color value in Register to Memory DMA2D mode */
									   (uint32_t)0x60100000,                              /* DMA2D output buffer */
									   1,              /* width of buffer in pixels */
									   lv_area_get_width(area) * lv_area_get_height(area));            /* height of buffer in pixels */

#else
		/*
		 * https://github.com/lvgl/lv_port_stm32h7b3i_disco/blob/master/hal_stm_lvgl/tft/tft.c
		 * uint32_t address =
		 * 			hlcd_ltdc.LayerCfg[Lcd_Ctx[0].ActiveLayer].FBStartAdress  			//多图层的使能图层的起始地址
		 * 			+ 	//	加上
		 * 			(((Lcd_Ctx[0].XSize*area->y1) + area->x1)  		//显示屏XSize，即宽像素数量 ，乘以 y1 的积 加上x1，就是 相对于起始第一个像素的偏移像素个数
		 * 			*	//	乘以
		 * 			Lcd_Ctx[0].BppFactor );		//	每个像素的byte数量，最终得到(x1，y1)坐标表示的像素在 LTDC 硬件图层中的地址
		 *
		 */
		uint32_t address = ILI9488_GetCursorAddress(area->x1, area->y1);
		HAL_Status = HAL_DMA2D_Start_IT(&hdma2d,
									   (uint32_t)buf_to_flush,               /* Color value in Register to Memory DMA2D mode */
									   address,                              /* DMA2D output buffer */
									   lv_area_get_width(area) ,              /* width of buffer in pixels */
									   lv_area_get_height(area));            /* height of buffer in pixels */
#endif

		  /*##-4- Wait until DMA2D transfer is over ################################################*/
//		  while(transferCompleteDetected == 0) {;}
//		  lv_disp_flush_ready(&disp_drv);
//
//		DebugArray.x1 = area->x1;
//		DebugArray.y1 = area->y1;
//		DebugArray.x2 = area->x2;
//		DebugArray.y2 = area->y2;
//		DebugArray.width = lv_area_get_width(area);
//		DebugArray.height = lv_area_get_height(area);

		return;

#else
		uint16_t * fb = (uint16_t *) LCD_LAYER_0_ADDRESS;
		uint16_t stride = disp_drv.hor_res;
		fb += area->y1 * stride;
		fb += area->x1;
		lv_coord_t w = lv_area_get_width(area);
	    int32_t y;
	    for(y = area->y1; y <= area->y2; y++) {
			lv_memcpy(fb, color_p, w * 2);
			fb += stride;
			color_p += w;
	    }
	    lv_disp_flush_ready(&disp_drv);
#endif

}

//void lv_disp_flush_ready_callback(){
//	lv_disp_flush_ready(&disp_drv);
//}




static void ex_disp_clean_dcache(lv_disp_drv_t *drv)
{
    SCB_CleanInvalidateDCache();
}

#if LV_USE_GPU_STM32_DMA2D
/**
  * @brief DMA2D configuration.
  * @note  This function Configure the DMA2D peripheral :
  *        1) Configure the transfer mode : register to memory
  *        2) Configure the color to be used to fill user defined area.
  * @retval
  *  None
  */

static void DMA2D_Config(uint32_t xSize)
{

	  /* Configure the DMA2D Mode, Color Mode and output offset */
	  hdma2d.Init.Mode         = DMA2D_M2M; /* DMA2D Mode memory to memory */
	  hdma2d.Init.ColorMode    = DMA2D_OUTPUT_RGB565; /* Output color mode is RGB565 : 16 bpp */
	  hdma2d.Init.OutputOffset = 0x0; /* No offset in output */
//	  hdma2d.Init.OutputOffset = 480 - xSize;
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
//	  hdma2d.LayerCfg[1].InputOffset = 480 - xSize;
	  hdma2d.LayerCfg[1].RedBlueSwap   = DMA2D_RB_REGULAR;      /* No R&B swap for the input foreground image */
	  hdma2d.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA;   /* No alpha inversion for the input foreground image */

	hdma2d.Instance = DMA2D;

	/* DMA2D Initialization */
	if (HAL_DMA2D_Init(&hdma2d) == HAL_OK)
	{
		if (HAL_DMA2D_ConfigLayer(&hdma2d, 1)==HAL_OK) {;}
		else
		{
			while(1) {;}
		}

	}
	else
	{
		while(1) {;}
	}

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
	transferCompleteDetected = 1;
	lv_disp_flush_ready(&disp_drv);
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

}


static void LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2){
	ILI9488_Address_Set(x1, y1, x2, y2);
	ILI9488_REG = 0x2C; 			/* 准备读写显存 */
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

#if 0
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	//在中断到来的时候，并且此时数据准备好了，才去发送数据到屏
	/*
	 * 当Tear中断来时候，在Tear中断的上升沿之后发送数据，由于发送速度比较慢，在屏显示的第一个周期内，拿到的数据依然是内置memory旧帧，
	 * 只有在第二次拿数据，即从d位置开始，拿到的数据才是新帧数据，这样的话，就需要两个Tear中断才能显示一帧数据
	 */
	if((GPIO_Pin == GPIO_PIN_0)){
//		transferCompleteDetected = 0;
		lv_disp_flush_ready(&disp_drv);
		TIMESTAMP_LCD_TE = HAL_GetTick();
	}
}
#endif
#endif

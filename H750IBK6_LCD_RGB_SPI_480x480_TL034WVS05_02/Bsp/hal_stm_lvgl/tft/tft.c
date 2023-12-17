/**
 * @file tft.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_conf.h"
#include "../lvgl/lvgl.h"
#include <string.h>
#include <stdlib.h>

#include "tft.h"
#include "stm32h7xx.h"
#include "bsp.h"
#include "dma2d.h"
#include "lcd_rgb.h"

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
//static void DMA2D_Config(uint32_t xSize);
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
//static lv_color_t buf1_1[TFT_HOR_RES * TFT_VER_RES / 2] __attribute__((section(".RAM_D1_Array")));
//static lv_color_t buf1_2[TFT_HOR_RES * TFT_VER_RES /2] __attribute__((section(".RAM_D1_Array")));

static lv_color_t buf1_1[TFT_HOR_RES * TFT_VER_RES] __attribute__((section(".RAM_SDRAM1_Array")));
//static lv_color_t buf1_2[TFT_HOR_RES * TFT_VER_RES] __attribute__((section(".RAM_SDRAM2_Array")));

/**********************
 *      MACROS
 **********************/

/**
 * Initialize your display here
 */

void tft_init(void)
{
//	LCD_ImagePreparation(0,0, 480, 320); // 测试刷一张图片

//	ILI9488_QuitWinMode();	//对全屏开窗口

	/* There is only one display on STM32 */
	if(our_disp != NULL)
		abort();

//    BSP_LCD_Init(0,LCD_ORIENTATION_LANDSCAPE);
//	BSP_LCD_DisplayOn(0);

   /*-----------------------------
	* Create a buffer for drawing
	*----------------------------*/

   /* LittlevGL requires a buffer where it draws the objects. The buffer's has to be greater than 1 display row*/
#if 0
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
	disp_drv.full_refresh = 1;	//单缓冲全尺寸也可以用，就是tnnd没有全屏刷新，局部刷新对齐？导致的撕裂

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
static void ex_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t * color_p)
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

    buf_to_flush = color_p;

    /*
     * https://forum.lvgl.io/t/stm32-dma2d-fsmc-lvgl/4317/3
     * SCB_CleanInvalidateDCache();
     * 在写入显示器之前调用。您需要使缓存无效，以便 DMA2D 的更改反映在 CPU 中
     */
    SCB_CleanInvalidateDCache();
    SCB_InvalidateICache();

    LCD_CopyBuffer(area->x1, area->y1, area->x2 - area->x1+1, area->y2 - area->y1 +1,(uint32_t*)color_p);

    /*IMPORTANT!!! 阻塞等待在这里调用，非阻塞在 DMA2D 传输完成的回调函数中调用
     *Inform the graphics library that you are ready with the flushing*/
    lv_disp_flush_ready(disp_drv);
#endif
}


static void ex_disp_clean_dcache(lv_disp_drv_t *drv)
{
    SCB_CleanInvalidateDCache();
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

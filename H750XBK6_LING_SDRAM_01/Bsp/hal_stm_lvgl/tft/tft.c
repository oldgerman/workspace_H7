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
#include "ltdc.h"
#include "dma2d.h"
#include "stm32h7xx_it.h"

#include "lcd_rgb.h"

#include "cmsis_os.h"
//#include "semphr.h"         //提供信号量
#include "common_inc.h"

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
//#include "../Components/rk043fn48h/rk043fn48h.h"    //RGB 4.3Inch

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
osSemaphoreId     sem_ltdc_irq;
__IO uint32_t   transferCompleteDetected;  /* DMA2D Transfer Complete flag */
HAL_StatusTypeDef HAL_Status = HAL_OK;
#endif
/**********************
 *  STATIC PROTOTYPES
 **********************/
/*These 3 functions are needed by LittlevGL*/
static void disp_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t * color_p);
static void ex_disp_clean_dcache(lv_disp_drv_t *drv);
void my_rounder_cb(lv_disp_drv_t * disp_drv, lv_area_t * area);

#if LV_USE_GPU_STM32_DMA2D

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
#endif

//static uint32_t ltdc_clpt_count = 0;

//static lv_color_t buf1_1[TFT_HOR_RES * TFT_VER_RES] __attribute__((section(".RAM_D1_Array")));
//static lv_color_t buf1_1[TFT_HOR_RES * TFT_VER_RES / 2] __attribute__((section(".RAM_D1_Array")));
//static lv_color_t buf1_2[TFT_HOR_RES * TFT_VER_RES /2] __attribute__((section(".RAM_D1_Array")));

// 带宽不够的话可以考虑用 512KB AXI SRAM 做半个屏幕的缓冲区，外部 SDRAM 做 LTDC 的显存，DMA2D把 内部 AXI SRAM 的数据复制到 SDRAM
//static lv_color_t buf1_1[TFT_HOR_RES * TFT_VER_RES / 2] __attribute__((section(".RAM_D1_Array")));

// 双缓冲+硬件垂直消隐，两个缓冲都得开全屏缓冲区
static lv_color_t buf1_1[TFT_HOR_RES * TFT_VER_RES] __attribute__((section(".sdram1_rank0")));
static lv_color_t buf1_2[TFT_HOR_RES * TFT_VER_RES] __attribute__((section(".sdram1_rank1")));

//static lv_color_t buf1_1[TFT_HOR_RES * TFT_VER_RES] __attribute__((section(".sdram2_rank1")));
//static lv_color_t buf1_1[TFT_HOR_RES * TFT_VER_RES] __attribute__((section(".sdram2_rank2")));
//static lv_color_t buf1_2[TFT_HOR_RES * TFT_VER_RES] __attribute__((section(".sdram2_rank3")));


/**********************
 *      MACROS
 **********************/

/**
  * @brief This function handles LTDC global interrupt.
  */
void LTDC_IRQHandler(void)
{
    HAL_LTDC_LineEventCallback(NULL);
}

/**
 * Initialize your display here
 */

void tft_init(void)
{
    /* 初始化LCD */
    LCD_Init();

    /* 烧屏专用 */
//    for(;;) {
//        LCD_Clear(TFT_RED);
//        HAL_Delay(500);
//        LCD_Clear(TFT_BLUE);
//        HAL_Delay(500);
//        LCD_Clear(TFT_GREEN);
//        HAL_Delay(500);
//        LCD_Clear(TFT_WHITE);
//        HAL_Delay(500);
//        LCD_Clear(TFT_BLACK);
//        HAL_Delay(500);
//    }

    /* 清屏为黑色 */
//    LCD_Clear(TFT_BLACK);

    /* 初始化 LTDC 垂直消隐中断 */
    HAL_NVIC_SetPriority(LTDC_IRQn, 14, 0);
    HAL_NVIC_EnableIRQ(LTDC_IRQn);
    HAL_LTDC_ProgramLineEvent(&hltdc, LCD_T_VPW + LCD_T_VBP + LCD_T_VD); // 使能行中断位置

    /* There is only one display on STM32 */
    if(our_disp != NULL)
        abort();

   /*-----------------------------
    * Create a buffer for drawing
    *----------------------------*/

   /* LittlevGL requires a buffer where it draws the objects. The buffer's has to be greater than 1 display row*/
#if 0
    static lv_disp_draw_buf_t disp_buf_1;
//    lv_disp_draw_buf_init(&disp_buf_1, buf1_1, NULL, TFT_HOR_RES * TFT_VER_RES);   /*Initialize the display buffer*/
    lv_disp_draw_buf_init(&disp_buf_1, buf1_1, NULL, TFT_HOR_RES * TFT_VER_RES / 2);   /*Initialize the display buffer*/
#else
    static lv_disp_draw_buf_t disp_buf_1;
    lv_disp_draw_buf_init(&disp_buf_1, buf1_1, buf1_2, TFT_HOR_RES * TFT_VER_RES);   /*Initialize the display buffer*/
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
    disp_drv.full_refresh = 1;    //单缓冲全尺寸也可以用，就是tnnd没有全屏刷新，局部刷新对齐？导致的撕裂

    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = disp_flush;

    /*
     * https://docs.lvgl.io/master/porting/display.html#draw-buffer
     * 对要重绘的区域坐标进行四舍五入。例如，2x2 像素可以转换为 2x8。
     * 如果显示控制器只能刷新具有特定高度或宽度的区域（通常为 8 px 高度，单色显示器），则可以使用它。
     */
//    disp_drv.rounder_cb = my_rounder_cb;

    //软件旋转屏幕
//    disp_drv.sw_rotate = 1;   // add for rotation
//    disp_drv.rotated = LV_DISP_ROT_90;   // add for rotation

    /* 可选：当 lvgl 需要任何影响渲染的 CPU 缓存被清理时调用 */
    disp_drv.clean_dcache_cb = ex_disp_clean_dcache;

    /*Set a display buffer*/
    disp_drv.draw_buf = &disp_buf_1;

    /*Finally register the driver*/
    our_disp = lv_disp_drv_register(&disp_drv);

    //  创建并释放信号量
//    transferCompleteDetected = 1
    osSemaphoreDef(sem_ltdc_irq);
    sem_ltdc_irq = osSemaphoreNew(
            1,                              // 可用令牌的最大数量 1
            1,                              // 可用令牌的初始数量 1
            osSemaphore(sem_ltdc_irq));    // 信号量属性
    osSemaphoreRelease(sem_ltdc_irq);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

void my_rounder_cb(lv_disp_drv_t * disp_drv, lv_area_t * area)
{
      /* Update the areas as needed.
       * For example it makes the area to start only on 8th rows and have Nx8 pixel height.
       * 例如，它使该区域仅从第 8 行开始，并且具有 Nx8 像素高度。*/
       area->y1 = area->y1 & 0xfff8;    //1111,1111,1111,1000
       area->x1 = area->x1 & 0xfff8;
       area->y2 = (area->y2 & 0xfff8) + 8 - 1;
       area->x2 = (area->x2 & 0xfff8) + 8 - 1;
}

volatile bool disp_flush_enabled = true;

/* Enable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_enable_update(void)
{
    disp_flush_enabled = true;
}

/* Disable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_disable_update(void)
{
    disp_flush_enabled = false;
}

/* Flush the content of the internal buffer the specific area on the display
 * You can use DMA or any hardware acceleration to do this operation in the background but
 * 'lv_flush_ready()' has to be called when finished
 * This function is required only when LV_VDB_SIZE != 0 in lv_conf.h*/
static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t * color_p)
{
    if(disp_flush_enabled) {
        int32_t x1 = area->x1;
        int32_t x2 = area->x2;
        int32_t y1 = area->y1;
        int32_t y2 = area->y2;
        /*Return if the area is out the screen*/

        if(x2 < 0) return;
        if(y2 < 0) return;
        if(x1 > TFT_HOR_RES - 1) return;
        if(y1 > TFT_VER_RES - 1) return;


        /*
         * https://forum.lvgl.io/t/stm32-dma2d-fsmc-lvgl/4317/3
         * SCB_CleanInvalidateDCache();
         * 在写入显示器之前调用。您需要使缓存无效，以便 DMA2D 的更改反映在 CPU 中
         */
        SCB_CleanInvalidateDCache();
        SCB_InvalidateICache();
#if LV_USE_GPU_STM32_DMA2D
    //    LCD_CopyBuffer_IT(
    //            area->x1,
    //            area->y1,
    //            area->x2 - area->x1+1,  // width
    //            area->y2 - area->y1 +1, // height
    //            (uint32_t*)color_p
    //        );
#endif
    //    static int i = 0;
    //    if(i)
    //        return;
    //    i = 1;

        // 永远等待信号量
        osSemaphoreAcquire(sem_ltdc_irq, osWaitForever);
    //    while (wTransferState== 0){}
    //    wTransferState = 0;
        // 切换LTDC显存的缓冲区
        // 用户不需要调用 DMA2D，LVGL 8.3 库已经完成了，用户仅仅切换 LTDC 显存地址即可
        __HAL_LTDC_LAYER(&hltdc, 0)->CFBAR =(uint32_t)color_p;
        __HAL_LTDC_RELOAD_IMMEDIATE_CONFIG(&hltdc);
    }
    /*IMPORTANT!!!
     *Inform the graphics library that you are ready with the flushing*/

    lv_disp_flush_ready(disp_drv);
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
void My_DMA2D_CpltCallback(DMA2D_HandleTypeDef *hdma2d)
{
//    if(transferCompleteDetected == 0)
//    {
//        transferCompleteDetected = 1;
//        lv_disp_flush_ready(&disp_drv);
//    }
}

/**
  * @brief  DMA2D error callbacks
  * @param  hdma2d: DMA2D handle
  * @note   This example shows a simple way to report DMA2D transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
void My_DMA2D_ErrorCallback(DMA2D_HandleTypeDef *hdma2d)
{

}

/**
  * @brief  Line Event callback.
  * @param  hltdc  pointer to a LTDC_HandleTypeDef structure that contains
  *                the configuration information for the LTDC.
  * @retval None
  */
void HAL_LTDC_LineEventCallback(LTDC_HandleTypeDef *hltdc)
{
    LTDC->ICR = (uint32_t)LTDC_IER_LIE; // 清除行中断标志

    /* 释放信号量 */
//    wTransferState = 1;
    osSemaphoreRelease(sem_ltdc_irq);
}

/**
 * @file tft.h
 *
 */

#ifndef DISP_H
#define DISP_H

#ifdef __cplusplus
extern "C" {
#endif
/*********************
 *      INCLUDES
 *********************/
#include "main.h"
#include <stdint.h>
#include "../lvgl/src/misc/lv_color.h"
#include "../lvgl/src/misc/lv_area.h"

/*********************
 *      DEFINES
 *********************/
#define TFT_HOR_RES LCD_T_HD
#define TFT_VER_RES LCD_T_VD

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void tft_init(void);
void checkflush();
void My_DMA2D_CpltCallback(DMA2D_HandleTypeDef *hdma2d);
void My_DMA2D_ErrorCallback(DMA2D_HandleTypeDef *hdma2d);
void disp_enable_update(void);
void disp_disable_update(void);
/**********************
 *      MACROS
 **********************/

extern __IO uint32_t   transferCompleteDetected;

#ifdef __cplusplus
}
#endif
#endif

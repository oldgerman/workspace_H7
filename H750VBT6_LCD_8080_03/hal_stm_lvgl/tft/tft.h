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
#include <stdint.h>
#include "../Share/Libraries/lvgl/src/misc/lv_color.h"
#include "../Share/Libraries/lvgl/src/misc/lv_area.h"

/*********************
 *      DEFINES
 *********************/
#define TFT_HOR_RES 320
#define TFT_VER_RES 480

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void tft_init(void);

void checkflush();
/**********************
 *      MACROS
 **********************/
#ifdef __cplusplus
}
#endif
#endif

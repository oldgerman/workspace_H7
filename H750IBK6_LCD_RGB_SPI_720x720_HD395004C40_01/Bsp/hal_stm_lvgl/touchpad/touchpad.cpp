/**
 * @file indev.c
 *
 */
/*********************
 *      INCLUDES
 *********************/
#include "touchpad.h"
#include "bsp.h"
#include "hal_stm_lvgl/tft/tft.h"
#include "../lvgl/src/hal/lv_hal.h"
#include"lcd_touch.h"
/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void touchpad_read(lv_indev_drv_t *drv, lv_indev_data_t *data);

/**********************
 *  STATIC VARIABLES
 **********************/
TAMC_GT911 tp = TAMC_GT911(LCD_T_HD, LCD_T_VD);

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialize your input devices here
 */
void touchpad_init(void)
{
    /* 初始化触摸屏 */
    tp.begin();
    tp.setRotation(ROTATION_INVERTED);

#if 1
    static lv_indev_drv_t indev_drv;                /*Descriptor of an input device driver*/
    lv_indev_drv_init(&indev_drv);                  /*Basic initialization*/
    indev_drv.type = LV_INDEV_TYPE_POINTER;         /*The touchpad is pointer type device*/
    indev_drv.read_cb = touchpad_read;

    lv_indev_drv_register(&indev_drv);
#endif
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
uint16_t debug_tpy = 0;
uint16_t debug_tpx = 0;
/**
 * Read an input device
 * @param indev_id id of the input device to read
 * @param x put the x coordinate here
 * @param y put the y coordinate here
 * @return true: the device is pressed, false: released
 */
static void touchpad_read(lv_indev_drv_t *indev, lv_indev_data_t *data)
{
#if 1
    /* Read your touchpad */
    static int16_t last_x = 0;
    static int16_t last_y = 0;

    tp.read();

    if (tp.isTouched){
        for (int i = 0; i < tp.touches; i++){
            printf("Touch %d : x: %d  y: %d  size: %d\r\n",
                    i+1,
                    tp.points[i].x,
                    tp.points[i].y,
                    tp.points[i].size);
        }

        data->point.x = tp.points[0].x;
        data->point.y = tp.points[0].y;
        debug_tpx = data->point.x;
        debug_tpy = data->point.y;
        last_x = data->point.x;
        last_y = data->point.y;
        data->state = LV_INDEV_STATE_PRESSED;
    }
    else
    {
        data->point.x = last_x;
        data->point.y = last_y;
        data->state = LV_INDEV_STATE_RELEASED;
    }
#endif
}

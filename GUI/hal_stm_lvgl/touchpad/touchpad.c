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
#include "lvgl/src/hal/lv_hal.h"
#include "ft6x36_reg.h"
#include "bsp_touch_port.h"
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
//static ft6x36_td_status_t  td_status;	// 检测到有效的触摸点个数

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
//	TS_Init_t hTS;
//	hTS.Width = TFT_HOR_RES;
//	hTS.Height = TFT_VER_RES;
//	hTS.Orientation = TS_SWAP_XY;
//	hTS.Accuracy = 0;
//
//    BSP_TS_Init(TS_INSTANCE, &hTS);

    static lv_indev_drv_t indev_drv;                /*Descriptor of an input device driver*/
    lv_indev_drv_init(&indev_drv);                  /*Basic initialization*/
    indev_drv.type = LV_INDEV_TYPE_POINTER;         /*The touchpad is pointer type device*/
    indev_drv.read_cb = touchpad_read;

    lv_indev_drv_register(&indev_drv);

}

/**********************
 *   STATIC FUNCTIONS
 **********************/
uint16_t debug_tpy = 0;
uint16_t debug_tpx = 0;
uint16_t debug_tpy_map = 0;
/**
 * Read an input device
 * @param indev_id id of the input device to read
 * @param x put the x coordinate here
 * @param y put the y coordinate here
 * @return true: the device is pressed, false: released
 */
static void touchpad_read(lv_indev_drv_t *indev, lv_indev_data_t *data)
{
    /* Read your touchpad */
    static int16_t last_x = 0;
    static int16_t last_y = 0;
    touch_update();
    if(ft6x36_reg_td.data.td_status.number_of_touch_points)
    {
    		/* FT6236 X坐标 0~255+62，317很接近320 */
            data->point.x =
            		(((uint16_t)ft6x36_reg_td.data.p1_xh.touch_x_position_h) << 8) |
					ft6x36_reg_td.data.p1_xl.touch_x_position_l;
            /* FT6236 Y坐标 0~255+207，462不满480, 菜单应该需要实现一个电容触摸校准，然后这里用map()映射一下校准*/
            debug_tpx = data->point.x;

            debug_tpy =
            		(((uint16_t)ft6x36_reg_td.data.p1_yh.touch_y_position_h) << 8) |
					ft6x36_reg_td.data.p1_yl.touch_y_position_l;

            data->point.y = fmap(
            	(((uint16_t)ft6x36_reg_td.data.p1_yh.touch_y_position_h) << 8) |
            							ft6x36_reg_td.data.p1_yl.touch_y_position_l,
										0, 462, 0, 480);
            debug_tpy_map = data->point.y;


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
}

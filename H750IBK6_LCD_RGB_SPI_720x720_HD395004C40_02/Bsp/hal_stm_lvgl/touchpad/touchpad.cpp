/**
  ******************************************************************************
  * @file        touchpad.cpp
  *
  * @author      OldGerman
  * @created on  Dec 19, 2023
  * @brief
  ******************************************************************************
  * @attention
  *
  * Copyright (C) 2022 OldGerman.
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see https://www.gnu.org/licenses/.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "touchpad.h"
//#include "touchPointFSM.h"

/* Private typedef -----------------------------------------------------------*/
typedef bool uEventTouched_t;

/* Private define ------------------------------------------------------------*/
#define DEBUG_PRINT(...)
// #define DEBUG_PRINT  printf

/* Private macro -------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
TAMC_GT911 tp = TAMC_GT911(LCD_T_HD, LCD_T_VD);

/* Exported variables --------------------------------------------------------*/
// TouchPointFSM touchPointFSM;
TouchPointFSM touchPointFSM(true, 700, 700);

/* Private variables ---------------------------------------------------------*/
const uint8_t ucEventTouchedNum = 3; // 消息队列 FIFO 深度，实测手一直放在屏幕上消息队列可读消息数在1到3之间
osMessageQueueId_t xMsgQueueTouched;
osMessageQueueId_t xMsgQueueTouchData;

/* Private function prototypes -----------------------------------------------*/
static void touchpad_read(lv_indev_drv_t *drv, lv_indev_data_t *data);

/* Function implementations --------------------------------------------------*/
/**
 * @brief 初始化输入设备
 */
void touchpad_init(void)
{
    /* 初始化触摸屏 */
    tp.begin();
    tp.setRotation(ROTATION_INVERTED);

    static lv_indev_drv_t indev_drv;        /*Descriptor of an input device driver*/
    lv_indev_drv_init(&indev_drv);          /*Basic initialization*/
    indev_drv.type = LV_INDEV_TYPE_POINTER; /*The touchpad is pointer type device*/
    indev_drv.read_cb = touchpad_read;
    lv_indev_drv_register(&indev_drv);

#ifdef DEBUG_TOUCH_ZOOM_POINTS
    /* 显示鼠标光标方便调试 */
    // Set cursor. For simplicity set a HOME symbol now.
    lv_indev_t *mouse_indev = lv_indev_drv_register(&indev_drv);
    LV_IMG_DECLARE(mouse_cursor_icon)
    lv_obj_t *mouse_cursor = lv_img_create(lv_scr_act());
    lv_img_set_src(mouse_cursor, &mouse_cursor_icon);
    lv_indev_set_cursor(mouse_indev, mouse_cursor);
#endif

    // 初始化消息队列
    xMsgQueueTouched = osMessageQueueNew(ucEventTouchedNum, sizeof(uEventTouched_t), NULL);
    xMsgQueueTouchData = osMessageQueueNew(1, sizeof(TouchData_t), NULL);
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == EXTI11_Pin)
    {
        /* 向消息队列写消息，通知写事件 */
        static const uEventTouched_t msg = true;
        osMessageQueuePut(
            xMsgQueueTouched,
            &msg, // 指向消息的指针，会使用 memcpy 拷贝消息地址上的数据，不是直接传递地址
            0U,   // 消息优先级 0
            0U);  // 写阻塞时间
    }
}

/**
 * @brief  打印GT911 5点的坐标和触摸面积
 */
void TP_DEBUG_PRINT(TAMC_GT911 &tp)
{
    for (int i = 0; i < tp.touches; i++)
    {
        DEBUG_PRINT("Touch %d : x: %d  y: %d  size: %d\r\n",
                    i + 1,
                    tp.points[i].x,
                    tp.points[i].y,
                    tp.points[i].size);
    }
}

/**
 * @brief Read an input device
 * @param indev_id id of the input device to read
 * @param x put the x coordinate here
 * @param y put the y coordinate here
 * @return true: the device is pressed, false: released
 */
static void touchpad_read(lv_indev_drv_t *indev, lv_indev_data_t *data)
{
    /* Read your touchpad */
    TouchPointFSM::OutputData out;
    uEventTouched_t msg;
    osStatus_t osStatus;

//    uint8_t ucQueueCount = osMessageQueueGetCount(xMsgQueueTouched);
//    DEBUG_PRINT("消息队列剩余消息数：%d\r\n", ucQueueCount);

    osStatus = osMessageQueueGet(xMsgQueueTouched, &msg, 0U, 0); // wait for message

    /* 消息队列有消息，判定为手指触摸屏幕 */
    if (osStatus == osOK)
    {
        /* 两次取到消息后读 GT911 的间隔时间可能小于 10ms，小于 GT911 数据更新周期，会读出异常的数据 */
        static TickType_t xLastWakeTime = xTaskGetTickCount();
        /**
         * 参数2是20ms，该延时时间必须大于GT911的更新中断脉冲周期10ms，
         * 若设为 10ms 则消息队列可能为 0，会使本实现失败（根据消息队列有消息来判断手持续放在屏幕上）
         */
        if (waitTimeOS(&xLastWakeTime, 20))
        {
            tp.read();
        }

        if (tp.bufferHaveTouchPoints)
        {
            TP_DEBUG_PRINT(tp);
            
            // 乱序初始化结构体数组
            TouchPointFSM::InputData act = {
                .pressed = 1,
                .num = tp.touches,
                .points = {
                    [0] = {
                        .x = (int16_t)tp.points[0].x,
                        .y = (int16_t)tp.points[0].y,
                    },
                    [1] = {
                        .x = (int16_t)tp.points[1].x,
                        .y = (int16_t)tp.points[1].y,
                    }
                }
            };
            out = touchPointFSM.update(act);
        }
    }
    /* 消息队列为空，判定为手指离开屏幕 */
    else
    {
        TouchPointFSM::InputData points = {
            .pressed = 0,
            .num = 0
        };
        out = touchPointFSM.update(points);
    }

    if(out.pressed) {
        data->state = LV_INDEV_STATE_PRESSED; // 未检测到触摸点输入设备的状态是释放
        /* 打包数据给LVGL */
        data->point.x = out.point.x;
        data->point.y = out.point.y;

        if(out.zooming)
        {
            /* 打包数据给自定义的缩放回调函数 */
            TouchData_t touchData = {
                .point = {
                    .x = out.point.x,
                    .y = out.point.y
                },
                .zoom = {
                    .x = out.zoom.x,
                    .y = out.zoom.y,
                    .h = out.zoom.h
                }
            };
            osMessageQueuePut(
                    xMsgQueueTouchData,
                    &touchData,     // 指向消息的指针，会使用 memcpy 拷贝消息地址上的数据，不是直接传递地址
                    0U,             // 消息优先级 0
                    0U);            // 写阻塞时间
        }
    }
    else {
        data->state = LV_INDEV_STATE_RELEASED; // 未检测到触摸点输入设备的状态是释放
    }

    DEBUG_PRINT("state_ = %d, x = %d, y = %d\r\n", data->state, data->point.x, data->point.y);
}

/**
  ******************************************************************************
  * @file        LinearSmooth5p2.c
  * @author      caicaptain2
  * @modify      oldgerman
  *              使用 arm math 改写
  * @created on  2022年3月11日
  * @brief       
  ******************************************************************************
  * @attention
  * [信号与系统] 五点二次线性平滑的函数分享 1楼
  * https://www.armbbs.cn/forum.php?mod=viewthread&tid=111378&extra=page%3D1
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Function implementations --------------------------------------------------*/

/**
 * @brief  5点2次线性平滑
 * @param  data:  输入数据，输出数据
 * @param  size:  data数据的个数
 * @retval 数据数量的合法性
 */
uint8_t LinearSmooth5p2(float *data, uint16_t size)
{
    float output[5]; // 5点缓存
    uint16_t j = 0;
    float param[5][5] = {
        // 每一行的数字之和为35
        {31,  9, -3, -5,  3},
        { 9, 13, 12,  6, -5},
        {-3, 12, 17, 12, -3},
        {-5,  6, 12, 13,  9},
        { 3, -5, -3,  9, 31},
    };

    if (size < 5)
    {
        return 0; // 数据太少, 不处理
    }
    else
    {
        output[0] = (
              param[0][0] * data[0]
            + param[0][1] * data[1]
            + param[0][2] * data[2]
            + param[0][3] * data[3]
            + param[0][4] * data[4]) / 35;
        output[1] = (
              param[1][0] * data[0]
            + param[1][1] * data[1]
            + param[1][2] * data[2]
            + param[1][3] * data[3]
            + param[1][4] * data[4]) / 35;
        output[3] = (
              param[3][0] * data[size - 5]
            + param[3][1] * data[size - 4]
            + param[3][2] * data[size - 3]
            + param[3][3] * data[size - 2]
            + param[3][4] * data[size - 1]) / 35;
        output[4] = (
              param[4][0] * data[size - 5]
            + param[4][1] * data[size - 4]
            + param[4][2] * data[size - 3]
            + param[4][3] * data[size - 2]
            + param[4][4] * data[size - 1]) / 35;

        for (j = 2; j <= size - 3; j++)
        {
            output[2] = (
                  param[2][0] * data[j - 2]
                + param[2][1] * data[j - 1]
                + param[2][2] * data[j    ]
                + param[2][3] * data[j + 1]
                + param[2][4] * data[j + 2]) / 35;
                
            data[j] = output[2];
        }

        data[0] = output[0];
        data[1] = output[1];
        data[size - 2] = output[3];
        data[size - 1] = output[4];

        return 1; // 成功转换
    }
}

/**
 * @brief 5点2次线性平滑，使用 arm math 加速计算
 * @param  data:  输入数据，输出数据
 * @param  size:  data数据的个数
 * @retval 数据数量的合法性
 */
uint8_t armLinearSmooth5p2(float *data, uint16_t size)
{
    // TODO
    float output[5]; // 5点缓存
    uint16_t j = 0;
    float param[5][5] = {
        // 每一行的数字之和为35
        {31,  9, -3, -5,  3},
        { 9, 13, 12,  6, -5},
        {-3, 12, 17, 12, -3},
        {-5,  6, 12, 13,  9},
        { 3, -5, -3,  9, 31},
    };

    if (size < 5)
    {
        return 0; // 数据太少, 不处理
    }
    else
    {
        output[0] = (
              param[0][0] * data[0]
            + param[0][1] * data[1]
            + param[0][2] * data[2]
            + param[0][3] * data[3]
            + param[0][4] * data[4]) / 35;
        output[1] = (
              param[1][0] * data[0]
            + param[1][1] * data[1]
            + param[1][2] * data[2]
            + param[1][3] * data[3]
            + param[1][4] * data[4]) / 35;
        output[3] = (
              param[3][0] * data[size - 5]
            + param[3][1] * data[size - 4]
            + param[3][2] * data[size - 3]
            + param[3][3] * data[size - 2]
            + param[3][4] * data[size - 1]) / 35;
        output[4] = (
              param[4][0] * data[size - 5]
            + param[4][1] * data[size - 4]
            + param[4][2] * data[size - 3]
            + param[4][3] * data[size - 2]
            + param[4][4] * data[size - 1]) / 35;

        for (j = 2; j <= size - 3; j++)
        {
            output[2] = (
                  param[2][0] * data[j - 2]
                + param[2][1] * data[j - 1]
                + param[2][2] * data[j    ]
                + param[2][3] * data[j + 1]
                + param[2][4] * data[j + 2]) / 35;
                
            data[j] = output[2];
        }

        data[0] = output[0];
        data[1] = output[1];
        data[size - 2] = output[3];
        data[size - 1] = output[4];

        return 1; // 成功转换
    }
}

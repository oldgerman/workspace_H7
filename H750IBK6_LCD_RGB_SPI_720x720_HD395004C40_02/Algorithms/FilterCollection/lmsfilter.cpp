/**
  ******************************************************************************
  * @file        lms_filter.cpp
  * @modify      OldGerman
  * @created on  2023年12月22日
  * @brief       
  ******************************************************************************
  * @attention
  * 安富莱_STM32-V7开发板_第2版DSP数字信号处理教程（V2.7）
  * 第49章 STM32H7 的自适应滤波器实现，无需 Matlab 生成系数（支持实时滤波）
  * 文件名称 : testlmsfilter.c
  * 版    本 : V1.0
  * 说    明 : LMS最小均方自适应滤波器
  * 修改记录 :
  * 版本号   日期         作者        说明
  * V1.0    2020-09-17   Eric2013     首发
  * Copyright (C), 2020-2030, 安富莱电子 www.armfly.com
  ******************************************************************************
  */

#if 0
/* Includes ------------------------------------------------------------------*/
#include "lmsfilter.h"                 /* 底层硬件驱动 */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/


/* Private macro -------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
const float32_t OrigalData1[TEST_LENGTH_SAMPLES];
const float32_t MixData1[TEST_LENGTH_SAMPLES];
const float32_t OrigalData[TEST_LENGTH_SAMPLES];
const float32_t MixData[TEST_LENGTH_SAMPLES];

/* Private constants ---------------------------------------------------------*/

/* Exported variables --------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/


/* Private function prototypes -----------------------------------------------*/
/* Function implementations --------------------------------------------------*/
/**
 * 我需要在每个捏合事件开始时，初始化滤波器，在捏合回调内逐点实时滤波（每次回调只传一个点），输出滤波器的点
 * 预测需要4个lms滤波器，写成class好了
 * 需要对单点的xy坐标滤波 //!< 这个地方还把焦点的偏移作用到单点给LVGL的思路错了，应该只把焦点传给LVGL的对象，从单点切换到焦点是，软件通知LVGL假松手一次释放单点坐标到焦点坐标
 * 需要对焦点的xy坐标滤波 //!< 仅对焦点x和y坐标滤波
 */
/*
*********************************************************************************************************
*    函 数 名: arm_lms_f32_test3
*    功能说明: 10Hz正弦波 + 20Hz正弦波 + 30Hz正弦波 + 高斯分布白噪声 + 均匀分布白噪声，滤除高斯分布白噪声 + 均匀分布白噪声
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void LMSFilter::init()
{
    arm_lms_norm_instance_f32 lmsS;
    float32_t  *inputF32, *outputF32, *inputREF, *outputERR;


    /* 如果是实时性的滤波，仅需清零一次 */
    memset(lmsCoeffs32,0,sizeof(lmsCoeffs32));
    memset(lmsStateF32,0,sizeof(lmsStateF32));

    /* 初始化输入输出缓存指针 */
    inputREF = (float32_t *)&OrigalData1[0];    /* 原始理想波形 */
    inputF32 = (float32_t *)&MixData1[0];       /* 合成的噪声波形 */
    outputF32 = (float32_t *)&testOutput[0];    /* 滤波后输出波形 */
    outputERR = (float32_t *)&test_f32_ERR[0];  /* 误差数据 */

    /* 归一化LMS初始化 */
    arm_lms_norm_init_f32 (&lmsS,                         /* LMS结构体 */
                           NUM_TAPS,                      /* 滤波器系数个数 */
                            (float32_t *)&lmsCoeffs32[0], /* 滤波 */
                            &lmsStateF32[0],              /* 滤波器系数 */
                            0.1,                          /* 步长 */
                            blockSize);                   /* 处理的数据个数 */

}

void LMSFilter::update()
{
    /* 实现LMS自适应滤波，这里每次处理1个点 */
    for(uint32_t i=0; i < numBlocks; i++)
    {

        arm_lms_norm_f32(&lmsS, /* LMS结构体 */
                        inputF32 + (i * blockSize),   /* 输入数据 */
                        inputREF + (i * blockSize),   /* 输出数据 */
                        outputF32 + (i * blockSize),  /* 参考数据 */
                        outputERR + (i * blockSize),  /* 误差数据 */
                        blockSize);                      /* 处理的数据个数 */

    }
//    /* 打印滤波后结果 */
//    for(i=0; i<TEST_LENGTH_SAMPLES; i++)
//    {
//        printf("%f, %f, %f\r\n", MixData1[i], outputF32[i], test_f32_ERR[i]);
//    }

}

/**
  * @brief  初始化动态内存API
  * @param  Malloc	function wrapper
  * @free  	Free	function wrapper
  * @retval None
  */
void LMSFilter::initMemoryHeapAPI(
		std::function<void* (size_t size, size_t alignment)>	Aligned_malloc,
		std::function<void  (void* ptr_aligned)>				Aligned_free,
		std::function<void  (void* ptr, size_t alignment)> 		Aligned_detect)
{
	aligned_malloc = Aligned_malloc;
	aligned_free = Aligned_free;
	aligned_detect = Aligned_detect;
}
/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
#endif

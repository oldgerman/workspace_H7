/*
*********************************************************************************************************
*
*	模块名称 : SD卡Fat文件系统和SD卡模拟演示模块。
*	文件名称 : demo_sd_fatfs.c
*	版    本 : V1.0
*	说    明 : 该例程移植FatFS文件系统（版本 R0.12c），演示如何创建文件、读取文件、创建目录和删除文件
*			   并测试了文件读写速度.支持以下6个功能，用户通过电脑端串口软件发送数字给开发板即可:
*              1 - 显示根目录下的文件列表
*              2 - 创建一个新文件armfly.txt
*              3 - 读armfly.txt文件的内容
*              4 - 创建目录
*              5 - 删除文件和目录
*              6 - 读写文件速度测试
*
*              另外注意，SD卡模拟U盘是用的MicroUSB接口。
*	修改记录 :
*		版本号   日期         作者        说明
*		V1.0    2018-12-12   Eric2013    正式发布
*
*	Copyright (C), 2018-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef DEMO_SD_FATFS_H_
#define DEMO_SD_FATFS_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"

/* Exported types ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
void DemoFatFS(uint8_t cmd);

uint32_t delectThenCreateWaveFile();
uint32_t initExistingWaveFile();
uint32_t readWaveFile(uint32_t addr, uint32_t size, uint8_t* pData);
uint32_t writeWaveFile(uint32_t addr, uint32_t size, uint8_t* pData);

#ifdef __cplusplus
}
#endif

#endif /* DEMO_SD_FATFS_H_ */

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/

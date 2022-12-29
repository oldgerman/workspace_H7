/*
 * frtos_spi_conf.h
 *
 *  Created on: 2022年12月26日
 *      @Author: OldGerman (过气德国佬)
 */

#ifndef BSP_FRTOS_SPI_CONF_H_
#define BSP_FRTOS_SPI_CONF_H_

#ifdef __cplusplus

extern "C" {
#endif

#ifndef RTOS_EN
#define RTOS_EN 1	//	0不使用RTOS，修改为1使用RTOS
#endif

#if RTOS_EN
#include "cmsis_os.h"
#else
#define osDelay
#endif

/* 重定义SPI SCK时钟 */
#define FRTOS_SPI_BAUDRATEPRESCALER_2		SPI_BAUDRATEPRESCALER_2
#define FRTOS_SPI_BAUDRATEPRESCALER_4      SPI_BAUDRATEPRESCALER_4
#define FRTOS_SPI_BAUDRATEPRESCALER_8     	SPI_BAUDRATEPRESCALER_8
#define FRTOS_SPI_BAUDRATEPRESCALER_16     SPI_BAUDRATEPRESCALER_16
#define FRTOS_SPI_BAUDRATEPRESCALER_32    	SPI_BAUDRATEPRESCALER_32
#define FRTOS_SPI_BAUDRATEPRESCALER_64   	SPI_BAUDRATEPRESCALER_64
#define FRTOS_SPI_BAUDRATEPRESCALER_128   	SPI_BAUDRATEPRESCALER_128
#define FRTOS_SPI_BAUDRATEPRESCALER_256  	SPI_BAUDRATEPRESCALER_256

/* 将缓冲区编译到非cache区的宏 */
#ifndef  RAM_REGION_NO_CACHE
//#define RAM_REGION_NO_CACHE	//待定
#define  RAM_REGION_NO_CACHE	__attribute__((section(".RAM_D2_Array")))	// 非cache缓冲区变量放在.RAM_D2
#endif
#ifndef RAM_REGION_NO_CACHE
#error "macro 'RAM_REGION_NO_CACHE' not defined"
#endif

/* 将函数运行域编译到RAM的宏 */
#ifndef IRAM_ATTR
//#define IRAM_ATTR				//待定
/**
 * @notice 	.ld和.s文件（链接脚本、启动文件）都需要修改
 * 			方法请参考1：https://blog.csdn.net/whj123999/article/details/119977388?spm=1001.2014.3001.5501
 * 			方法请参考2：UM2609 2.5.7.2 Place code in RAM
 */
#define IRAM_ATTR	__attribute__((section(".RAM_CODE")))	// 时间关键程序从FLASH拷贝到ITCMRAM中执行
#endif
#ifndef IRAM_ATTR
#error "macro 'IRAM_ATTR' not defined"
#endif

#ifdef __cplusplus

}		/* extern "C" */
#endif	/* __cplusplus */
#endif /* BSP_FRTOS_SPI_CONF_H_ */

//ALIGN_32BYTES();	/* 32字节对齐宏：例如 ALIGN_32BYTES(uint8_t _pRxData[_size])  */

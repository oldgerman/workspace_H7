/*
 * Dev_Inf.c
 *
 */
#include "Dev_Inf.h"
#include "quadspi.h"

/* This structure contains information used by ST-LINK Utility to program and erase the device */
#if defined (__ICCARM__)
__root struct StorageInfo const StorageInfo  =  {
#else
struct StorageInfo const StorageInfo = {
#endif
		"STM32H750VB_W25Q64JV", 	 	     // Device Name + version number
		NOR_FLASH,                  		 // Device Type
		0x90000000,                			 // Device Start Address
		MEMORY_FLASH_SIZE,                 	 // Device Size in Bytes
		MEMORY_PAGE_SIZE,                    // Programming Page Size
		0xFF,                                // Initial Content of Erased Memory

		// Specify Size and Address of Sectors (view example below)
		{ { (MEMORY_FLASH_SIZE / MEMORY_SECTOR_SIZE),  // Sector Numbers,
				(uint32_t) MEMORY_SECTOR_SIZE },       //Sector Size

				{ 0x00000000, 0x00000000 } }
};
//#if defined (__ICCARM__)
//__root struct StorageInfo const StorageInfo  =  {
//#else
//struct StorageInfo const StorageInfo =  {
//#endif
//    "STM32H750VB_W25Q64JV", /* 算法名，添加算法到STM32CubeProg安装目录会显示此名字 */
//    NOR_FLASH,                      /* 设备类型 */
//    0x90000000,                     /* Flash起始地址 */
//    32 * 1024 * 1024,               /* Flash大小，32MB */
//    4*1024,                         /* 编程页大小 */
//    0xFF,                           /* 擦除后的数值 */
//    512 , 64 * 1024,                /* 块个数和块大小 */
//    0x00000000, 0x00000000,
//};

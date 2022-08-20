/*
*********************************************************************************************************
*
*	模块名称 : QSPI Flash读写演示程序。
*	文件名称 : demo_qspi_flash.c
*	版    本 : V1.0
*	说    明 : 安富莱STM32-V7开发板标配的QSPI Flash型号为 W25Q256JV, 32M字节
*
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2020-11-01 Eric2013  正式发布
*
*	Copyright (C), 2020-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "demo_spi_flash.h"
#include "bsp.h"


#define TEST_ADDR		0		/* 读写测试地址 */
#define TEST_SIZE		1024	/* 读写测试数据大小 */

/* 仅允许本文件内调用的函数声明 */
static void sfDispMenu(void);
static void sfReadTest(void);
static void sfWriteTest(void);
static void sfErase(void);
static void sfViewData(uint32_t _uiAddr);
static void sfWriteAll(uint8_t _ch);
static void sfTestReadSpeed(void);

ALIGN_32BYTES(uint8_t buf[TEST_SIZE]);        /* 这种做32字节对齐，主要是方便DMA方式使用 */

ALIGN_32BYTES(uint8_t SpeedTestbuf[16*1024]); /* 仅用于读速度测试目的 */

/*
*********************************************************************************************************
*	函 数 名: DemoSpiFlash
*	功能说明: QSPI读写例程
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void DemoSpiFlash(void)
{
	uint8_t cmd;
	uint32_t uiReadPageNo = 0, id;

	/* 检测串行Flash OK */
	id = QSPI_ReadID();
	printf("检测到串行Flash, ID = %08X, 型号: WM25Q%dJV\r\n", (unsigned int)id, QSPI_FLASH_SIZE_MBit);
//	printf(" 容量 : 32M字节, 扇区大小 : 4096字节, 页大小：256字节\r\n");

	sfDispMenu();		/* 打印命令提示 */
    
//    bsp_StartAutoTimer(0, 200); /* 启动1个200ms的自动重装的定时器，软件定时器0 */
	while(1)
	{
//		bsp_Idle();		/* 这个函数在bsp.c文件。用户可以修改这个函数实现CPU休眠和喂狗 */
//
//        /* 判断软件定时器0是否超时 */
//        if(bsp_CheckTimer(0))
//        {
//            /* 每隔200ms 进来一次 */
//            bsp_LedToggle(2);
//        }
        
		if (comGetChar(COM1, &cmd))	/* 从串口读入一个字符(非阻塞方式) */
		{
			switch (cmd)
			{
				case '1':
					printf("\r\n【1 - 读QSPI Flash, 地址:0x%X ,长度:%d字节】\r\n", TEST_ADDR, TEST_SIZE);
					sfReadTest();	/* 读串行Flash数据，并打印出来数据内容 */
					break;

				case '2':
					printf("\r\n【2 - 写QSPFlash, 地址:0x%X,长度:%d字节】\r\n", TEST_ADDR, TEST_SIZE);
					sfWriteTest();	/* 写串行Flash数据，并打印写入速度 */
					break;

				case '3':
					printf("\r\n【3 - 写QSPI Flash前10KB空间, 全0x55】\r\n");
					sfWriteAll(0x55);/* 擦除串行Flash数据，实际上就是写入全0xFF */
					break;

				case '4':
					printf("\r\n【4 - 读整个QSPI Flash, %dM字节】\r\n", QSPI_FLASH_SIZES/(1024*1024));
					sfTestReadSpeed(); /* 读整个串行Flash数据，测试速度 */
					break;
				
				case 'y':
				case 'Y':
					printf("\r\n【Y - 擦除整个QSPI Flash】\r\n");
					printf("整个Flash擦除完毕大概需要几分钟，请耐心等待");
					sfErase();		/* 擦除串行Flash数据，实际上就是写入全0xFF */
					break;

				case 'z':
				case 'Z': /* 读取前1K */
					if (uiReadPageNo > 0)
					{
						uiReadPageNo--;
					}
					else
					{
						printf("已经是最前\r\n");
					}
					sfViewData(uiReadPageNo * 1024);
					break;

				case 'x':
				case 'X': /* 读取后1K */
					if (uiReadPageNo < QSPI_FLASH_SIZES / 1024 - 1)
					{
						uiReadPageNo++;
					}
					else
					{
						printf("已经是最后\r\n");
					}
					sfViewData(uiReadPageNo * 1024);
					break;

				default:
					sfDispMenu();	/* 无效命令，重新打印命令提示 */
					break;
			}
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: sfReadTest
*	功能说明: 读串行Flash测试
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void sfReadTest(void)
{
	uint32_t i;

	/* 清缓冲区为0x55 */
	memset(buf, 0x55, TEST_SIZE);
	QSPI_ReadBuffer(buf, 0, 1);
	printf("读取第1个字节，数据如下\r\n");
	printf(" %02X \r\n", buf[0]);
	
	/* 清空缓冲区为AA */
	memset(buf, 0xAA, TEST_SIZE);
	QSPI_ReadBuffer(buf, 0, 10);
	printf("读取前10个字节，数据如下\r\n");	
	for (i = 0; i < 10; i++)
	{
		printf(" %02X", buf[i]);
	}
	printf("\r\n");
	
	/* 清空缓冲区 */
	memset(buf, 0, TEST_SIZE);
	QSPI_ReadBuffer(buf, 0, TEST_SIZE);
	printf("读串行Flash成功，数据如下\r\n");

	/* 打印数据 */
	for (i = 0; i < TEST_SIZE; i++)
	{
		printf(" %02X", buf[i]);

		if ((i & 31) == 31)
		{
			printf("\r\n");	/* 每行显示16字节数据 */
		}
		else if ((i & 31) == 15)
		{
			printf(" - ");
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: sfWriteTest
*	功能说明: 写串行Flash测试
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void sfWriteTest(void)
{
	uint32_t i;
	uint32_t iTime1, iTime2;

	/* 填充测试缓冲区 */
	for (i = 0; i < TEST_SIZE; i++)
	{
		buf[i] = i;
	}

	QSPI_EraseSector(TEST_ADDR);
	
	iTime1 = bsp_GetRunTime();	/* 记下开始时间 */
	for(i = 0; i< TEST_SIZE; i += QSPI_PAGE_SIZE)
	{
		if (QSPI_WriteBuffer(buf, TEST_ADDR + i, QSPI_PAGE_SIZE) == 0)
		{
			printf("写串行Flash出错！\r\n");
			return;
		}		
	}
	
	iTime2 = bsp_GetRunTime();	/* 记下结束时间 */
	printf("写串行Flash成功！\r\n");
	
	/* 打印读速度 */
	printf("数据长度: %d字节, 写耗时: %ldms, 写速度: %ldB/s\r\n", TEST_SIZE, iTime2 - iTime1, (TEST_SIZE * 1000) / (iTime2 - iTime1));
//	printf("数据长度: %d字节\r\n", TEST_SIZE);
}

/*
*********************************************************************************************************
*	函 数 名: sfTestReadSpeed
*	功能说明: 测试串行Flash读速度。读取整个串行Flash的数据，最后打印结果
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void sfTestReadSpeed(void)
{
	uint32_t i;
	int32_t iTime1, iTime2;
	uint32_t uiAddr;

	
	iTime1 = bsp_GetRunTime();	/* 记下开始时间 */
	uiAddr = 0;
	for (i = 0; i < QSPI_FLASH_SIZES / (16*1024); i++, uiAddr += 16*1024)
	{
		QSPI_ReadBuffer(SpeedTestbuf, uiAddr, 16*1024);
	}
	iTime2 = bsp_GetRunTime();	/* 记下结束时间 */

	/* 打印读速度 */
	printf("数据长度: %d字节, 读耗时: %ldms, 读速度: %ld KB/s\r\n", QSPI_FLASH_SIZES, iTime2 - iTime1, QSPI_FLASH_SIZES / (iTime2 - iTime1));
}

/*
*********************************************************************************************************
*	函 数 名: sfWriteAll
*	功能说明: 写QSPI全部数据
*	形    参：_ch : 写入的数据
*	返 回 值: 无
*********************************************************************************************************
*/
static void sfWriteAll(uint8_t _ch)
{
	uint32_t i;
	int32_t iTime1, iTime2;
	uint32_t uiAddr;

	/* 填充测试缓冲区 */
	for (i = 0; i < TEST_SIZE; i++)
	{
		buf[i] = _ch;
	}
	
	/* 先擦除前12KB空间 */
	QSPI_EraseSector(0);
	QSPI_EraseSector(4096);
	QSPI_EraseSector(8192);
	
	iTime1 = bsp_GetRunTime();	/* 记下开始时间 */
	uiAddr = 0;
	for (i = 0; i < 10*1024/QSPI_PAGE_SIZE; i++, uiAddr += QSPI_PAGE_SIZE)
	{
		
		QSPI_WriteBuffer(buf, uiAddr, QSPI_PAGE_SIZE);
		printf(".");
		if (((i + 1) % 128) == 0)
		{
			printf("\r\n");
		}
	}
	
	iTime2 = bsp_GetRunTime();	/* 记下结束时间 */
	printf("\r\n");
	
	/* 打印读速度 */
	printf("数据长度: %d字节, 写耗时: %ldms, 写速度: %ldKB/s\r\n", 10*1024, iTime2 - iTime1, (10 * 1024) / (iTime2 - iTime1));
}

/*
*********************************************************************************************************
*	函 数 名: sfErase
*	功能说明: 擦除串行Flash
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void sfErase(void)
{
	uint32_t i;
	int32_t iTime1, iTime2;
	uint32_t uiAddr;
	
	
	iTime1 = bsp_GetRunTime();	/* 记下开始时间 */
	
	uiAddr = 0;
	printf("\r\n");
	for (i = 0; i < QSPI_FLASH_SIZES / QSPI_SECTOR_SIZE; i++, uiAddr += QSPI_SECTOR_SIZE)
	{
		QSPI_EraseSector(uiAddr);
		
		printf(".");
		if (((i + 1) % 128) == 0)
		{
			printf("%ld%%\r\n", i*100 /(QSPI_FLASH_SIZES / QSPI_SECTOR_SIZE));
		}
	}
	
	iTime2 = bsp_GetRunTime();	/* 记下结束时间 */

	/* 打印读速度 */
	printf("擦除串行Flash完成！, 耗时: %ldms\r\n", iTime2 - iTime1);
	return;
}

/*
*********************************************************************************************************
*	函 数 名: sfViewData
*	功能说明: 读串行Flash并显示，每次显示1K的内容
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void sfViewData(uint32_t _uiAddr)
{
	uint32_t i;

	QSPI_ReadBuffer(buf, _uiAddr,  1024);		/* 读数据 */
	printf("地址：0x%08X; 数据长度 = 1024\r\n", (unsigned int)_uiAddr);

	/* 打印数据 */
	for (i = 0; i < 1024; i++)
	{
		printf(" %02X", buf[i]);

		if ((i & 31) == 31)
		{
			printf("\r\n");	/* 每行显示16字节数据 */
		}
		else if ((i & 31) == 15)
		{
			printf(" - ");
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: sfDispMenu
*	功能说明: 显示操作提示菜单
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void sfDispMenu(void)
{
	printf("\r\n*******************************************\r\n");
	printf("请选择操作命令:\r\n");
	printf("【1 - 读QSPI Flash, 地址:0x%X,长度:%d字节】\r\n", TEST_ADDR, TEST_SIZE);
	printf("【2 - 写QSPI Flash, 地址:0x%X,长度:%d字节】\r\n", TEST_ADDR, TEST_SIZE);
	printf("【3 - 写QSPI Flash前10KB空间, 全0x55】\r\n");
	printf("【4 - 读整个串行Flash, 测试读速度】\r\n");
	printf("【Z - 读取前1K，地址自动减少】\r\n");
	printf("【X - 读取后1K，地址自动增加】\r\n");
	printf("【Y - 擦除整个串行Flash，整片32MB擦除大概300秒左右】\r\n");
	printf("其他任意键 - 显示命令提示\r\n");
	printf("\r\n");
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/

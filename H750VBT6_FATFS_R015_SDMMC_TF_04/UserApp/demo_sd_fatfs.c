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
*	@modfiy:
*      日期         修改人       说明
*      2023-03-17   OldGerman    整理注释格式
*                                消除本文件的编译警告
*                                对接fibre框架处理的USB命令解析
*                                支持不同IO/SIZE读写测速，范围 1~ 128 Block，以markdown表格语法打印结果
*
*      日期         修改人       说明
*      2023-03-19   OldGerman    将armfly.txt里写入的测试文本使用单独的全局变量保存
*
*********************************************************************************************************
*/
/* Includes ------------------------------------------------------------------*/
#include "demo_sd_fatfs.h"
#include "common_inc.h"
#include "bsp.h"
#include "ff.h"				/* FatFS文件系统模块*/
#include "ff_gen_drv.h"
#include "sdmmc.h"

/* Private typedef -----------------------------------------------------------*/
/* 用于读写测速时是否校验文件 */
typedef enum{
	file_verify_false = 0,
	file_verify_true = 1,
}file_verify_t;

/* Private define ------------------------------------------------------------*/
/* 用于测试读写速度 */
#define TEST_FILE_LEN			(8*1024*1024) 	/* 用于测试的文件长度: 8192KB */
#define BUF_SIZE				(64*1024)		/* 每次读写SD卡的最大数据长度: 64KB*/

/* Private macro -------------------------------------------------------------*/
/** 将一些 FATFS 变量和缓冲区编译到指定 RAM
  * 注意：STM32H750 的 SDMMC1 仅支持 RAM_D1，而 SDMMC2 支持 RAM_D1、RAM_D2
  */
#ifndef  RAM_D1
#define  RAM_D1	 __attribute__((section(".RAM_D1_Array"))) 	// 放在 .RAM_D1 (AXI SRAM)
#endif
#ifndef RAM_D1
#error "macro 'RAM_D1' not defined"
#endif

/* Exported constants --------------------------------------------------------*/
/* HAL库 SD_HandleTypeDef 句柄*/
extern SD_HandleTypeDef hsd1; // 仅用于在 ViewRootDir() 中获取卡速度信息

/* Private constants ---------------------------------------------------------*/
/* 在.data段的测试文本内容 */
char DataSec_ArmflyTxt[] = {"FatFS Write Demo \r\n www.armfly.com \r\n"};

/* Exported variables --------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* FATFS文件系统对象 */
RAM_D1 FATFS fs;
/* 文件对象 */
RAM_D1 FIL file;
/* 文件夹对象 */
DIR DirInf;
/* 文件信息对象 */
FILINFO FileInf;
/* SD卡逻辑驱动路径，比盘符0，就是"0:/" */
char DiskPath[4];

/** @attention
  * 一定要将以下读写缓冲区配置为32字节对齐
  *   非32字节对齐会导致 sd_diskio.c 中的读写函数高频使用 memcpy 将缓冲区的分块复制
  *   到 512Byte 大小的 Scratch Buffer ，导致IDMA传输使用效率极低的单个块读写函数
  *   SDMMC_CmdWriteSingleBlock() 和 SDMMC_CmdReadSingleBlock()，而不是使用批量块
  *   读写函数 SDMMC_CmdReadMultiBlock() 和 SDMMC_CmdWriteMultiBlock()
  */
/* FatFs的读取临时缓冲区 */
RAM_D1 ALIGN_32BYTES(char FsReadBuf[1024]);
/* FatFs的写入临时缓冲区 */
RAM_D1 ALIGN_32BYTES(char FsWriteBuf[1024]);
/* 测试的读写临时缓冲区 */
//RAM_D1 ALIGN_32BYTES(uint8_t g_TestBuf[BUF_SIZE]);

/* FatFs API的返回值 */
static const char * FR_Table[]=
{
	"FR_OK：成功",				                             /* (0) Succeeded */
	"FR_DISK_ERR：底层硬件错误",			                 /* (1) A hard error occurred in the low level disk I/O layer */
	"FR_INT_ERR：断言失败",				                     /* (2) Assertion failed */
	"FR_NOT_READY：物理驱动没有工作",			             /* (3) The physical drive cannot work */
	"FR_NO_FILE：文件不存在",				                 /* (4) Could not find the file */
	"FR_NO_PATH：路径不存在",				                 /* (5) Could not find the path */
	"FR_INVALID_NAME：无效文件名",		                     /* (6) The path name format is invalid */
	"FR_DENIED：由于禁止访问或者目录已满访问被拒绝",         /* (7) Access denied due to prohibited access or directory full */
	"FR_EXIST：文件已经存在",			                     /* (8) Access denied due to prohibited access */
	"FR_INVALID_OBJECT：文件或者目录对象无效",		         /* (9) The file/directory object is invalid */
	"FR_WRITE_PROTECTED：物理驱动被写保护",		             /* (10) The physical drive is write protected */
	"FR_INVALID_DRIVE：逻辑驱动号无效",		                 /* (11) The logical drive number is invalid */
	"FR_NOT_ENABLED：卷中无工作区",			                 /* (12) The volume has no work area */
	"FR_NO_FILESYSTEM：没有有效的FAT卷",		             /* (13) There is no valid FAT volume */
	"FR_MKFS_ABORTED：由于参数错误f_mkfs()被终止",	         /* (14) The f_mkfs() aborted due to any parameter error */
	"FR_TIMEOUT：在规定的时间内无法获得访问卷的许可",		 /* (15) Could not get a grant to access the volume within defined period */
	"FR_LOCKED：由于文件共享策略操作被拒绝",				 /* (16) The operation is rejected according to the file sharing policy */
	"FR_NOT_ENOUGH_CORE：无法分配长文件名工作区",		     /* (17) LFN working buffer could not be allocated */
	"FR_TOO_MANY_OPEN_FILES：当前打开的文件数大于_FS_SHARE", /* (18) Number of open files > _FS_SHARE */
	"FR_INVALID_PARAMETER：参数无效"	                     /* (19) Given parameter is invalid */
};

/* Private function prototypes -----------------------------------------------*/
static void DispMenu(void);
static void ViewRootDir(void);
static void CreateNewFile(void);
static void ReadFileData(void);
static void CreateDir(void);
static void DeleteDirFile(void);
//static void WriteFileTest(file_verify_t file_verify);

/* Function implementations --------------------------------------------------*/

#define FAT32_CLUSTER_SIZE_32KB		(32*1024) 		//FAT32簇大小 32KB
#define FAT32_CLUSTER_SIZE_64KB		(64*1024) 		//FAT32簇大小 64KB
#define EXFAT_CLUSTER_SIZE_128KB	(128*1024) 		//exFAT簇大小 128KB
#define FAT_CLUSTER_SIZE			FAT32_CLUSTER_SIZE_64KB

//#define WAVE_FILE_SIZE		(16*1024*1024)	//波形文件大小8MB
#define WAVE_FILE_SIZE		(128*1024*1024)	//波形文件大小128MB
#define CLMT_ARRAY_ELEMENT_SIZE sizeof(DWORD) //CLMT数组元素大小

#define SZ_TBL	((WAVE_FILE_SIZE / FAT_CLUSTER_SIZE + 1) * 2) // clmt = 8194 （32K）/ 2050（128K）

/** 在使用快速搜索模式前，必须将 CLMT 在内存中的缓冲区创建到 DWORD 数组中 **/
/* DreamShell/firmware/isoldr/loader/fs/fat/fs.c 中 SZ_TBL 为 32 */
RAM_D1 DWORD clmt[SZ_TBL]; /* Cluster link map table buffer *//* 簇链接映射表缓冲区 */

uint32_t delectThenCreateWaveFile()
{
	FRESULT res = FR_OK;
	char path[32];
	char* filename = "WAVE.txt";
	sprintf(path, "%s%s", DiskPath, filename);

	printf("\r\n尝试删除并新建 %s ...\r\n", filename);

 	/* 挂载文件系统 */
	res += f_mount(&fs, DiskPath, 0);			/* Mount a logical drive */
	if (res != FR_OK) {
		printf("挂载文件系统失败 (%s)\r\n", FR_Table[res]);
	}

	/* 删除文件 */
	res = f_unlink(path);
	if (res == FR_OK) {
		printf("删除 %s 成功\r\n", filename);
	}
	else if (res == FR_NO_FILE) {
		printf("删除 %s 无效, 没有发现此文件\r\n", filename);
	}
	else {
		printf("删除 %s 失败 (错误代码 = %d) 文件只读\r\n", filename, res);
	}

	res += f_open(&file, path,
			FA_CREATE_ALWAYS);
	if (res == FR_OK) {
		printf("创建新文件 %s 成功\r\n", filename);
	}
	else {
		printf("创建新文件 %s 失败 (错误代码 = %d) \r\n", filename, res);
	}

	/* 关闭文件 */
	res += f_close(&file);

	/* 卸载文件系统 */
	res += f_mount(NULL, DiskPath, 0);

	printf("删除并新建 %s (%s)\r\n\r\n", filename, FR_Table[res]);

	return res;
}

uint32_t initExistingWaveFile()
{
	FRESULT res = FR_OK;
	FRESULT res_sum = FR_OK;
	char path[32];
	char* filename = "WAVE.txt";
	sprintf(path, "%s%s", DiskPath, filename);

	printf("\r\n尝试初始化已有的 %s ...\r\n", filename);

 	/* 挂载文件系统 */
	res = f_mount(&fs, DiskPath, 0);			/* Mount a logical drive */
	if (res != FR_OK) {
		printf("挂载文件系统失败 (%s)\r\n", FR_Table[res]);
	} else {
		printf("挂载文件系统成功\r\n\r\n");
	}
	res_sum += res;

	/* 打开文件 */
	/* 以 写入+读取 的方式访问打开的文件
	 * FA_CREATE_ALWAYS: 创建一个新文件。如果文件存在，它将被截断并覆盖
	 * 					(不论已有文件大小多大，截断覆盖后的显示大小都为0字节)
	 * 					截断覆盖后的文件预分配大小和原有文件一样的，都是128MB
	 * 					那么紧接着创建的CLMT还是和截断覆盖前的一样
	 * 					如果是删除了原有文件再 FA_CREATE_ALWAYS，那么紧接着
	 * 					创建的CLMT会随着文件边读边改变
	 */
	res = f_open(&file, path, FA_WRITE | FA_READ);
	if (res == FR_OK)
		printf("%s 打开成功\r\n", filename);
	else {
		printf("%s 打开失败(%s)\r\n", filename, FR_Table[res]);
	}
	res_sum += res;

	/* 慢速搜索模式下 */
	res = f_lseek (&file, WAVE_FILE_SIZE); 			/* 使用f_lseek() 拓展文件大小（集群预分配）为 128MB */
    if (res || f_tell(&file) != WAVE_FILE_SIZE)	{	/* 检查文件大小增加是否成功 *//* 检查文件是否扩展成功 */
    	printf("%s 扩展文件大小（集群预分配）%dMB 失败(%s)\r\n", filename, WAVE_FILE_SIZE / 1024 / 1024, FR_Table[res]);
    } else {
    	printf("%s 扩展文件大小（集群预分配）%dMB 成功\r\n", filename, WAVE_FILE_SIZE / 1024 / 1024);
    }
    res_sum += res;

//    res += f_close(&file);

	/* 慢速搜索下先 f_lseek 偏移一个文件的大小 */
	res = f_lseek(&file, 0);	/* This is normal seek (cltbl is nulled on file open) */
	res_sum += res;

	/* 设定快速搜索模式 */
	(&file)->cltbl = clmt;
	clmt[0] = SZ_TBL;
	res = f_lseek(&file, CREATE_LINKMAP);
    if (res != FR_OK)	{
    	printf("%s 设置快速搜索模式失败(%s)\r\n", filename, FR_Table[res]);
    } else {
    	printf("%s 设置快速搜索模式成功\r\n", filename);
    }
	res_sum += res;

	printf("初始化已有的 %s (%s)\r\n", filename, FR_Table[res_sum]);

	return res;
}

uint32_t writeWaveFile(uint32_t addr, uint32_t size, uint8_t* pData)
{
	FRESULT  res = FR_OK;
	uint32_t bw;

	res += f_lseek(&file, addr);
	res += f_write(&file, pData, size, (UINT* )&bw);
	res += f_sync(&file);

	return res;
}

uint32_t readWaveFile(uint32_t addr, uint32_t size, uint8_t* pData)
{
	FRESULT  res = FR_OK;
	uint32_t bw;

	res += f_lseek(&file, addr);
	res += f_read(&file, pData, size, (UINT* )&bw);
	res += f_sync(&file);

	return res;
}

/*
*********************************************************************************************************
*	函 数 名: DemoFatFS
*	功能说明: FatFS文件系统演示主程序
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void DemoFatFS(uint8_t cmd)
{
	printf("\r\n");
	switch (cmd)
	{
	    case '0':
	    	DispMenu();
	    	break;
		case '1':
			printf("【1 - ViewRootDir】\r\n");
			ViewRootDir();		/* 显示SD卡根目录下的文件名 */
			break;

		case '2':
			printf("【2 - CreateNewFile】\r\n");
			CreateNewFile();	/* 创建一个新文件,写入一个字符串 */
			break;

		case '3':
			printf("【3 - ReadFileData】\r\n");
			ReadFileData();		/* 读取根目录下armfly.txt的内容 */
			break;

		case '4':
			printf("【4 - CreateDir】\r\n");
			CreateDir();		/* 创建目录 */
			break;

		case '5':
			printf("【5 - DeleteDirFile】\r\n");
			DeleteDirFile();	/* 删除目录和文件 */
			break;
#if 0
		case '6':
			printf("【6 - TestSpeed】\r\n");
			WriteFileTest(file_verify_false);	/* 读写文件速度测试，不校验文件 */
			break;

		case '7':
			printf("【7 - TestSpeed】\r\n");
			WriteFileTest(file_verify_true);	/* 读写文件速度测试，并校验文件 */
			break;
#endif
		default:
			break;
	}
}

/*
*********************************************************************************************************
*	函 数 名: DispMenu
*	功能说明: 显示操作提示菜单
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispMenu(void)
{
	printf("\r\n------------------------------------------------\r\n");
	printf("请选择操作命令:\r\n");
	printf("1 - 显示根目录下的文件列表\r\n");
	printf("2 - 创建一个新文件armfly.txt\r\n");
	printf("3 - 读armfly.txt文件的内容\r\n");
	printf("4 - 创建目录\r\n");
	printf("5 - 删除文件和目录\r\n");
	printf("6 - 读写文件速度测试，不校验文件\r\n");
	printf("7 - 读写文件速度测试，并校验文件\r\n");
}

/*
*********************************************************************************************************
*	函 数 名: ViewRootDir
*	功能说明: 显示SD卡根目录下的文件名
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void ViewRootDir(void)
{
	FRESULT result;
	uint32_t cnt = 0;
	FILINFO fno;
	
 	/* 挂载文件系统 */
	result = f_mount(&fs, DiskPath, 0);	/* Mount a logical drive */
	if (result != FR_OK)
	{
		printf("挂载文件系统失败 (%s)\r\n", FR_Table[result]);
	}

    /* 打印卡速度信息 */
    if(hsd1.SdCard.CardSpeed == CARD_NORMAL_SPEED)
    {
        printf("Normal Speed Card <12.5MB/S, MAX Clock < 25MHz, Spec Version 1.01\r\n");
    }
    else if (hsd1.SdCard.CardSpeed == CARD_HIGH_SPEED)
    {
        printf("High Speed Card <25MB/s, MAX Clock < 50MHz, Spec Version 2.00\r\n");
    }
    else if (hsd1.SdCard.CardSpeed == CARD_ULTRA_HIGH_SPEED)
    {
        printf("UHS-I SD Card <50MB/S for SDR50, DDR50 Cards, MAX Clock < 50MHz OR 100MHz\r\n");
        printf("UHS-I SD Card <104MB/S for SDR104, MAX Clock < 108MHz, Spec version 3.01\r\n");
    }

	/* 打开根文件夹 */
	result = f_opendir(&DirInf, DiskPath); /* 如果不带参数，则从当前目录开始 */
	if (result != FR_OK)
	{
		printf("打开根目录失败  (%s)\r\n", FR_Table[result]);
		return;
	}

	printf("属性        |  文件大小 | 短文件名 | 长文件名\r\n");
	for (cnt = 0; ;cnt++)
	{
		result = f_readdir(&DirInf, &FileInf); 		/* 读取目录项，索引会自动下移 */
		if (result != FR_OK || FileInf.fname[0] == 0)
		{
			break;
		}

		if (FileInf.fname[0] == '.')
		{
			continue;
		}

		/* 判断是文件还是子目录 */
		if (FileInf.fattrib & AM_DIR)
		{
			printf("(0x%02d)目录  ", FileInf.fattrib);
		}
		else
		{
			printf("(0x%02d)文件  ", FileInf.fattrib);
		}

		f_stat(FileInf.fname, &fno);
		
		/* 打印文件大小, 最大4G */
		printf(" %10d", (int)fno.fsize);


		printf("  %s\r\n", (char *)FileInf.fname);	/* 长文件名 */
	}

	/* 卸载文件系统 */
	 f_mount(NULL, DiskPath, 0);
}

/*
*********************************************************************************************************
*	函 数 名: CreateNewFile
*	功能说明: 在SD卡创建一个新文件，文件内容填写“www.armfly.com”
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void CreateNewFile(void)
{
	FRESULT result;
	uint32_t bw;
	char path[32];


 	/* 挂载文件系统 */
	result = f_mount(&fs, DiskPath, 0);			/* Mount a logical drive */
	if (result != FR_OK)
	{
		printf("挂载文件系统失败 (%s)\r\n", FR_Table[result]);
	}

	/* 打开文件 */
	sprintf(path, "%sarmfly.txt", DiskPath);
	result = f_open(&file, path,
			FA_CREATE_ALWAYS | FA_WRITE); //以创建+写入的方式访问打开的文件，若不存在此文件就创建，若存在就不再次创建
	if (result == FR_OK)
	{
		printf("armfly.txt 文件打开成功\r\n");
	}
	else
	{
		printf("armfly.txt 文件打开失败  (%s)\r\n", FR_Table[result]);
	}

	/* 重置缓冲区的数据为待写入内容 */
	//strlen 遇到'\0'停止，字符串大小不计算'\0'在内
//    memcpy((void *)FsWriteBuf, DataSec_ArmflyTxt, strlen(DataSec_ArmflyTxt) + 1);
	//strlen 是函数需要调用，耗费时间长，sizeof是运算符，使用sizeof更好
    memcpy((void *)FsWriteBuf, DataSec_ArmflyTxt, sizeof(DataSec_ArmflyTxt));

	/* 写一串数据 */
	result = f_write(&file, FsWriteBuf, sizeof(DataSec_ArmflyTxt), (UINT* )&bw);

	if (result == FR_OK)
	{
		printf("armfly.txt 文件写入成功\r\n");
	}
	else
	{
		printf("armfly.txt 文件写入失败  (%s)\r\n", FR_Table[result]);
	}

	/* 关闭文件*/
	f_close(&file);

	/* 卸载文件系统 */
	f_mount(NULL, DiskPath, 0);
}

/*
*********************************************************************************************************
*	函 数 名: ReadFileData
*	功能说明: 读取文件armfly.txt前128个字符，并打印到串口
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void ReadFileData(void)
{
	FRESULT result;
	uint32_t bw;
	char path[64];

	
 	/* 挂载文件系统 */
	result = f_mount(&fs, DiskPath, 0);			/* Mount a logical drive */
	if (result != FR_OK)
	{
		printf("挂载文件系统失败 (%s)\r\n", FR_Table[result]);
	}

	/* 打开文件 */
	sprintf(path, "%sarmfly.txt", DiskPath);
	result = f_open(&file, path, FA_OPEN_EXISTING | FA_READ);
	if (result !=  FR_OK)
	{
		printf("Don't Find File : armfly.txt\r\n");
		return;
	}

	/* 读取文件 */
	result = f_read(&file, FsReadBuf, sizeof(FsReadBuf), (UINT* )&bw);
	if (bw > 0)
	{
		FsReadBuf[bw] = 0;
		printf("\r\narmfly.txt 文件内容 : \r\n%s\r\n", FsReadBuf);
	}
	else
	{
		printf("\r\narmfly.txt 文件内容 : \r\n");
	}

	/* 关闭文件*/
	f_close(&file);

	/* 卸载文件系统 */
	f_mount(NULL, DiskPath, 0);
}

/*
*********************************************************************************************************
*	函 数 名: CreateDir
*	功能说明: 在SD卡根目录创建Dir1和Dir2目录，在Dir1目录下创建子目录Dir1_1
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void CreateDir(void)
{
	FRESULT result;
	char path[64]; 

	
 	/* 挂载文件系统 */
	result = f_mount(&fs, DiskPath, 0);			/* Mount a logical drive */
	if (result != FR_OK)
	{
		printf("挂载文件系统失败 (%s)\r\n", FR_Table[result]);
	}

	/* 创建目录/Dir1 */
	sprintf(path, "%sDir1", DiskPath);
	result = f_mkdir(path);
	if (result == FR_OK)
	{
		printf("f_mkdir Dir1 Ok\r\n");
	}
	else if (result == FR_EXIST)
	{
		printf("Dir1 目录已经存在(%d)\r\n", result);
	}
	else
	{
		printf("f_mkdir Dir1 失败 (%s)\r\n", FR_Table[result]);
		return;
	}

	/* 创建目录/Dir2 */
	sprintf(path, "%sDir2", DiskPath);
	result = f_mkdir(path);
	if (result == FR_OK)
	{
		printf("f_mkdir Dir2 Ok\r\n");
	}
	else if (result == FR_EXIST)
	{
		printf("Dir2 目录已经存在(%d)\r\n", result);
	}
	else
	{
		printf("f_mkdir Dir2 失败 (%s)\r\n", FR_Table[result]);
		return;
	}

	/* 创建子目录 /Dir1/Dir1_1	   注意：创建子目录Dir1_1时，必须先创建好Dir1 */
	sprintf(path, "%sDir1/Dir1_1", DiskPath);
	result = f_mkdir(path); /* */
	if (result == FR_OK)
	{
		printf("f_mkdir Dir1_1 成功\r\n");
	}
	else if (result == FR_EXIST)
	{
		printf("Dir1_1 目录已经存在 (%d)\r\n", result);
	}
	else
	{
		printf("f_mkdir Dir1_1 失败 (%s)\r\n", FR_Table[result]);
		return;
	}

	/* 卸载文件系统 */
	f_mount(NULL, DiskPath, 0);
}

/*
*********************************************************************************************************
*	函 数 名: DeleteDirFile
*	功能说明: 删除SD卡根目录下的 armfly.txt 文件和 Dir1，Dir2 目录
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DeleteDirFile(void)
{
	FRESULT result;
	uint8_t i;
	char path[64]; 
	
 	/* 挂载文件系统 */
	result = f_mount(&fs, DiskPath, 0);			/* Mount a logical drive */
	if (result != FR_OK)
	{
		printf("挂载文件系统失败 (%s)\r\n", FR_Table[result]);
	}
	
	/* 删除目录/Dir1 【因为还存在目录非空（存在子目录)，所以这次删除会失败】*/
	sprintf(path, "%sDir1", DiskPath);
	result = f_unlink(path);
	if (result == FR_OK)
	{
		printf("删除目录Dir1成功\r\n");
	}
	else if (result == FR_NO_FILE)
	{
		printf("没有发现文件或目录 :%s\r\n", "/Dir1");
	}
	else
	{
		printf("删除Dir1失败(错误代码 = %d) 文件只读或目录非空\r\n", result);
	}

	/* 先删除目录/Dir1/Dir1_1 */
	sprintf(path, "%sDir1/Dir1_1", DiskPath);
	result = f_unlink(path);
	if (result == FR_OK)
	{
		printf("删除子目录/Dir1/Dir1_1成功\r\n");
	}
	else if ((result == FR_NO_FILE) || (result == FR_NO_PATH))
	{
		printf("没有发现文件或目录 :%s\r\n", "/Dir1/Dir1_1");
	}
	else
	{
		printf("删除子目录/Dir1/Dir1_1失败(错误代码 = %d) 文件只读或目录非空\r\n", result);
	}

	/* 先删除目录/Dir1 */
	sprintf(path, "%sDir1", DiskPath);
	result = f_unlink(path);
	if (result == FR_OK)
	{
		printf("删除目录Dir1成功\r\n");
	}
	else if (result == FR_NO_FILE)
	{
		printf("没有发现文件或目录 :%s\r\n", "/Dir1");
	}
	else
	{
		printf("删除Dir1失败(错误代码 = %d) 文件只读或目录非空\r\n", result);
	}

	/* 删除目录/Dir2 */
	sprintf(path, "%sDir2", DiskPath);
	result = f_unlink(path);
	if (result == FR_OK)
	{
		printf("删除目录 Dir2 成功\r\n");
	}
	else if (result == FR_NO_FILE)
	{
		printf("没有发现文件或目录 :%s\r\n", "/Dir2");
	}
	else
	{
		printf("删除Dir2 失败(错误代码 = %d) 文件只读或目录非空\r\n", result);
	}

	/* 删除文件 armfly.txt */
	sprintf(path, "%sarmfly.txt", DiskPath);
	result = f_unlink(path);
	if (result == FR_OK)
	{
		printf("删除文件 armfly.txt 成功\r\n");
	}
	else if (result == FR_NO_FILE)
	{
		printf("没有发现文件或目录 :%s\r\n", "armfly.txt");
	}
	else
	{
		printf("删除armfly.txt失败(错误代码 = %d) 文件只读或目录非空\r\n", result);
	}

	/* 删除文件 speed1.txt */
	for (i = 0; i < 10; i++)
	{
		sprintf(path, "%sSpeed%02d.txt", DiskPath, i);/* 每写1次，序号递增 */	
		result = f_unlink(path);
		if (result == FR_OK)
		{
			printf("删除文件%s成功\r\n", path);
		}
		else if (result == FR_NO_FILE)
		{
			printf("没有发现文件:%s\r\n", path);
		}
		else
		{
			printf("删除%s文件失败(错误代码 = %d) 文件只读或目录非空\r\n", path, result);
		}
	}

	/* 卸载文件系统 */
	f_mount(NULL, DiskPath, 0);
}

#if 0
/*
*********************************************************************************************************
*	函 数 名: WriteFileTest
*	功能说明: 测试文件读写速度
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void WriteFileTest(file_verify_t file_verify)
{
	uint8_t s_ucTestSn = 0;

	printf("| IO SIZE | 写速度 | 写耗时 | 读速度 | 读耗时 | 测试文件名称 | 测试文件大小 | 校验文件数据 |\r\n");
	printf("| ------- | ------ | ------ | ------ | ------ | ------------ | ------------ | ------------ |\r\n");
	/* 依次读写测试 1、2、4、8、16、32、64、128个 Block( 1 Block = 512byte) */
	for(uint16_t nth = 0; nth < 8; nth++)
	{
		uint32_t buf_size = 0;
		buf_size = 512 << nth;

       /**
         * 校验文件标记
         * -1： N/A    (未开启校验)
         *  0： ERROR  (校验出错)
         *  1:  OK     (无错误)
         */
		int8_t flag_file_verify = -1;

		FRESULT result;
		char path[64];
		uint32_t bw;
		uint32_t i,k;
		uint32_t runtime1,runtime2,timelen;
		uint8_t err = 0;

		/* 检查buf_size大小 */
		if(buf_size > BUF_SIZE){
			buf_size = BUF_SIZE;
		}

		for (i = 0; i < sizeof(g_TestBuf); i++)
		{
			g_TestBuf[i] = (i / 512) + '0';
		}

		/* 挂载文件系统 */
		result = f_mount(&fs, DiskPath, 0);			/* Mount a logical drive */
		if (result != FR_OK)
		{
			printf("挂载文件系统失败 (%s), 终止测试\r\n", FR_Table[result]);
			/* 卸载文件系统 */
			f_mount(NULL, DiskPath, 0);
			return;
		}
	
		/* 打开文件 */
		sprintf(path, "%sSpeed%02d.txt", DiskPath, s_ucTestSn++); /* 每写1次，序号递增 */
		result = f_open(&file, path, FA_CREATE_ALWAYS | FA_WRITE);
	
		/* printf IO SIZE */
		if(buf_size == 512) {
		   printf("| 512B ");
		}
		else {
			printf("| %ldKB ", (buf_size /1024));
		}



		runtime1 = bsp_GetRunTime();	/* 读取系统运行时间 */

		/* 开始写文件测试 */
		for (i = 0; i < TEST_FILE_LEN / buf_size; i++)
		{
			result = f_write(&file, g_TestBuf, buf_size, (UINT* )&bw);
			if (result != FR_OK)
			{
				err = 1;
				printf("\r\n %s文件写失败\r\n", path);
				break;
			}
		}
		runtime2 = bsp_GetRunTime();	/* 读取系统运行时间 */

		if (err == 0)
		{
			timelen = (runtime2 - runtime1);
			/* printf 写速度 | 写耗时 */
			printf("| %ldKB/S | %ldms ",
				((TEST_FILE_LEN / 1024) * 1000) / timelen,
				timelen);
		}
	
		f_close(&file);		/* 关闭文件*/


		/* 开始读文件测试 */
		result = f_open(&file, path, FA_OPEN_EXISTING | FA_READ);
		if (result !=  FR_OK)
		{
			printf("\r\n 没有找到文件: %s\r\n", path);
			return;
		}

		runtime1 = bsp_GetRunTime();	/* 读取系统运行时间 */
		for (i = 0; i < TEST_FILE_LEN / buf_size; i++)
		{
			result = f_read(&file, g_TestBuf, buf_size, (UINT* )&bw);
			if (result == FR_OK)
			{
				/* 校验文件 */
				if(file_verify)
				{
					flag_file_verify = 1;
					/* 比较写入的数据是否正确，此语句会导致读卡速度结果降低到 3.5MBytes/S */
					for (k = 0; k < buf_size; k++)
					{
						if (g_TestBuf[k] != (k / 512) + '0')
						{
							err = 1;
//							printf("\r\n 文件读成功，但是数据出错\r\n");
							flag_file_verify = 0;
							break;
						}
					}
					if (err == 1)
					{
						break;
					}
				}
			}
			else
			{
				err = 1;
				printf("\r\n Speed1.txt 文件读失败\r\n");
				break;
			}
		}

		runtime2 = bsp_GetRunTime();	/* 读取系统运行时间 */

		if (err == 0)
		{
			timelen = (runtime2 - runtime1);
			/*printf 读速度 | 读耗时 */
			printf("| %ldKB/S | %ldms ",
				 ((TEST_FILE_LEN / 1024) * 1000) / timelen,
				 timelen);
		}

		/* printf 测试文件名称 | 测试文件大小| 校验文件数据  */
		printf("| %s | %dKB | %s |\r\n", path, (TEST_FILE_LEN /1024),
				(flag_file_verify == -1) ? ("N/A")   :
				(flag_file_verify ==  0) ? ("ERROR") :
			    ("OK"));

		/* 关闭文件*/
		f_close(&file);

		/* 卸载文件系统 */
		f_mount(NULL, DiskPath, 0);
	}
	printf("\r\n 测试完毕\r\n");
}
#endif
/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/

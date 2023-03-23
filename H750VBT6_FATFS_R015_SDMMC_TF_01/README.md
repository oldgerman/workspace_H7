## H750VBT6_FATFS_R015_SDMMC_TF_01

> 创建日期：2023-03-22

## 关于

本工程用于测试本id自研的瓦片切片算法，其产生的数据在 FATFS + TF卡 上读写的实时性

记录了使用 CubeMX 自动生成的 FATFS R0.12C版本 ST团队写的配置文件和BSP驱动 适配 FATFS R0.15 的过程

- 开发环境：STM32CubeIDE v1.11.2 + STM32CubeMX v6.6
- 包版本：STM32CubeH7 V1.11.0 / 04-Nov-2022
- 主RAM：DTCM
- FATFS：R0.15

## USB命令

USB命令的JSON文件在 VOFA+ 文件夹内，可导入伏特加上位机方便测试

## FATFS R0.15 适配 CubeMX R0.12C

### R0.12C  f_lseek()  的BUG

由于 CubeMX 6.6 + STM32CubeH7 V1.11.0 / 04-Nov-2022 生成的 FATFS 版本还是 13年 的 R0.12C，本工程在使用 f_lseek() 移到文件读写指针参数为非0地址时，会返回  FR_INT_ERR，看到这篇症状相同的帖子：[Getting FR_INT_ERR when using f_seek in ff.c - ST Community](https://community.st.com/s/question/0D53W000010vQCQSA2/getting-frinterr-when-using-fseek-in-ffc)，应该是 旧版本 R0.12C的BUG，ChaN老师在 FATFS 的版本更新日志中表明，此 BUG 在 R0.13 中修复，现在是 2023 年，最新版本 为 R0.15，又修复了不少BUG，那就用 CubeMX 自动生成的 FATFS 配置文件 适配最新版的 

### 下载 FATFS R0.15 源码，并在 Path 中添加路径

ChaN 老师网站的下载链接：Download: [FatFs R0.15 (zip)](http://elm-chan.org/fsw/ff/arc/ff15.zip)

解压到 本工程根目录即可，会生成一个 ff15 文件夹，将此文件夹路径添加到 Path 的 Source Location，将此文件夹内的 source 文件夹路径 添加到 Path 的 Includes 

### 适配 ffconf.h

新建 `ffconf_r0.15_by_r013c.h` 解决

### 适配 用户同步函数

在 CubeMX 自动生成的 Middlewares/Third_Party/FatFs/option/syscall.c 中有

```c
/* R0.13C 的4个用户同步函数 */
int ff_cre_syncobj ( BYTE vol, _SYNC_t *sobj )
int ff_del_syncobj ( _SYNC_t sobj )
int ff_req_grant ( _SYNC_t sobj )
void ff_rel_grant ( _SYNC_t sobj )

/* ff malloc 和 free 默认不使能 */
#if _USE_LFN == 3	/* LFN with a working buffer on the heap */
void* ff_memalloc ( UINT msize )
void ff_memfree ( void* mblock )
#endif
```

但 R0.15 版本这几个函数都没了，在ff15/documents/updates.html中有以下说明：

> User provided synchronization functions, `ff_cre_syncobj`, `ff_del_syncobj`, `ff_req_grant` and `ff_rel_grant`, needed when `FF_FS_REENTRANT` are replaced with `ff_mutex_create`, `ff_mutex_delete`, `ff_mutex_take` and `ff_mutex_give` respectively. For example, see `ffsystem.c`.
>
> 翻译：
>
> 用户提供的同步函数，`ff_cre_syncobj`、`ff_del_syncobj`、`ff_req_grant` 和 `ff_rel_grant`。当使能 `FF_FS_REENTRANT` 时，需要分别替换为 `ff_mutex_create`、`ff_mutex_delete`、`ff_mutex_take` 和 `ff_mutex_give`。示例请参见 `ffsystem.c`

ok，看看 ffsystem.c ，cubeMX 我配置为 _FS_REENTRANT 对应使能 FF_FS_REENTRANT，我需要修改此文件的 4个 同步函数 调用 Middlewares/Third_Party/FatFs/option/syscall.c 中的四个同步函数 

```c
#if FF_FS_REENTRANT	/* Mutal exclusion */
/*------------------------------------------------------------------------*/
/* Definitions of Mutex                                                   */
/*------------------------------------------------------------------------*/

#define OS_TYPE	0	/* 0:Win32, 1:uITRON4.0, 2:uC/OS-II, 3:FreeRTOS, 4:CMSIS-RTOS */
```

ffsystem.c 中使能 OS_TYPE 4 是 CMSIS-RTOS V1的，但我的工程是 V2的，这样  ff_mutex_create 中的写法编译就会报错，需要修改如下：

```c
int ff_mutex_create (	/* Returns 1:Function succeeded or 0:Could not create the mutex */
	int vol				/* Mutex ID: Volume mutex (0 to FF_VOLUMES - 1) or system mutex (FF_VOLUMES) */
)
{
    ...
#elif OS_TYPE == 4	/* CMSIS-RTOS */
#if (osCMSIS < 0x20000U)
	osMutexDef(cmsis_os_mutex);
	Mutex[vol] = osMutexCreate(osMutex(cmsis_os_mutex));
#else
    Mutex[vol] = osMutexNew(NULL);
#endif
	return (int)(Mutex[vol] != NULL);
#endif
}
```

### 适配 ffunicode.c 

ff15 中的 ffunicode.c中的数据 与 cubemx自动生成的  Middlewares/Third_Party/FatFs/option/cc936.c（GBK编码） 中存在重复定义，在过滤器中排除 cc936.c 即可。实测这样配置后，USB可以正常打印 FATFS 解析文件名和目录名的 GBK 编码的中文字符

### Path 过滤器的配置

| 文件夹：FATFS                                                | 文件夹：Middlewares                                          | 文件夹：ff15                                                 |
| ------------------------------------------------------------ | ------------------------------------------------------------ | ------------------------------------------------------------ |
| ![R0.12C适配R0.15_过滤器设置-FATFS](Images/R0.12C适配R0.15_过滤器设置-FATFS.png) | ![R0.12C适配R0.15_过滤器设置-Middlewares](Images/R0.12C适配R0.15_过滤器设置-Middlewares.png) | ![R0.12C适配R0.15_过滤器设置-ff15](Images/R0.12C适配R0.15_过滤器设置-ff15.png) |

备注：sd_diskio.c 的副本在UserApp 路径下，自定义了Cache配置并修复了BUG，副本出处：[H750VBT6_FATFS_SDMMC_TF_01](https://github.com/oldgerman/workspace_H7/tree/master/H750VBT6_FATFS_SDMMC_TF_01)

## 测试

### 各层缓冲区参数

| 层编号 | 瓦片大小 | 瓦片缓冲区大小 | 瓦片缓冲区地址 | 缓冲区大小 | 缓冲区发送周期 | DRAM 当前共使用 | DRAM 当前剩余 | DRAM 历史最少可用 |
| ------ | -------- | -------------- | -------------- | ---------- | -------------- | --------------- | ------------- | ----------------- |
| 0      | 1        | 2048           | 0x30000010     | 2048       | 2048           | 2072            | 129000        | 129000            |
| 1      | 2        | 2048           | 0x30000818     | 4096       | 1024           | 4128            | 126944        | 126944            |
| 2      | 4        | 2048           | 0x30001020     | 8192       | 512            | 6184            | 124888        | 124888            |
| 3      | 8        | 2048           | 0x30001828     | 16384      | 256            | 8240            | 122832        | 122832            |
| 4      | 16       | 2048           | 0x30002030     | 32768      | 128            | 10296           | 120776        | 120776            |
| 5      | 32       | 2048           | 0x30002838     | 65536      | 64             | 12352           | 118720        | 118720            |
| 6      | 64       | 2048           | 0x30003040     | 131072     | 32             | 14408           | 116664        | 116664            |
| 7      | 128      | 2048           | 0x30003848     | 262144     | 16             | 16464           | 114608        | 114608            |
| 8      | 256      | 2048           | 0x30004050     | 524288     | 8              | 18520           | 112552        | 112552            |
| 9      | 512      | 2048           | 0x30004858     | 1048576    | 4              | 20576           | 110496        | 110496            |
| 10     | 1024     | 2048           | 0x30005060     | 2097152    | 2              | 22632           | 108440        | 108440            |
| 11     | 2048     | 2048           | 0x30005868     | 4194304    | 1              | 24688           | 106384        | 106384            |
| 12     | 4096     | 4096           | 0x30006070     | 8388608    | 1              | 28792           | 102280        | 102280            |
| 13     | 8192     | 8192           | 0x30007078     | 16777216   | 1              | 36992           | 94080         | 94080             |
| 14     | 16384    | 16384          | 0x30009080     | 33554432   | 1              | 53384           | 77688         | 77688             |

### 不同 Block 大小下的读写速度

| IO SIZE | 写速度   | 写耗时  | 读速度    | 读耗时 | 测试文件名称 | 测试文件大小 | 校验文件数据 |
| ------- | -------- | ------- | --------- | ------ | ------------ | ------------ | ------------ |
| 512B    | 333KB/S  | 24579ms | 1462KB/S  | 5603ms | Speed00.txt  | 8192KB       | N/A          |
| 1KB     | 610KB/S  | 13419ms | 2539KB/S  | 3226ms | Speed01.txt  | 8192KB       | N/A          |
| 2KB     | 1197KB/S | 6840ms  | 4168KB/S  | 1965ms | Speed02.txt  | 8192KB       | N/A          |
| 4KB     | 2771KB/S | 2956ms  | 6638KB/S  | 1234ms | Speed03.txt  | 8192KB       | N/A          |
| 8KB     | 3423KB/S | 2393ms  | 8551KB/S  | 958ms  | Speed04.txt  | 8192KB       | N/A          |
| 16KB    | 8650KB/S | 947ms   | 9787KB/S  | 837ms  | Speed05.txt  | 8192KB       | N/A          |
| 32KB    | 8982KB/S | 912ms   | 10502KB/S | 780ms  | Speed06.txt  | 8192KB       | N/A          |
| 64KB    | 7699KB/S | 1064ms  | 10556KB/S | 776ms  | Speed07.txt  | 8192KB       | N/A          |

### CPU利用率

向层缓冲区不同写入频率下的CPU利用率：

| 32Hz 12%                                                     | 100Hz 40%                                                    |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| ![32Hz写110MB_CPU利用率_12_percent](Images/32Hz写110MB_CPU利用率_12_percent.png) | ![100Hz写110MB_CPU利用率_40_percent](Images/100Hz写110MB_CPU利用率_40_percent.png) |

### 波形文件写入频率

> *已有文件：表示已有写入 100MB 左右的文件，当前是覆盖写入，使用前一次写入文件的 CLMT，这样 f_lseek() 的执行会更快

| 备注      | 设置频率（Hz） | 实际频率（Hz） | 实际频率波动（Hz） | CPU 占用率（%） | 吞吐量（KB/s） | 备注                    |
| --------- | -------------- | -------------- | ------------------ | --------------- | -------------- | ----------------------- |
| 已有文件* | 200            | 165            | 105 - 167          | 41              | N/A            | 很不稳定                |
| 已有文件* | 100            | 100            | 97 - 104           | 40              | 3217.5         | 很稳定，CPU占用率过高   |
| 新建文件  | 200            | 94 - 110       | 81- 126            | 40              | N/A            | 很不稳定                |
| 新建文件  | 100            | 100            | 74 -121            | 40              | N/A            | 不太稳定，CPU占用率过高 |
| 新建文件  | 32             | 32.2           | 32.0- 32.5         | 12              | 1029.6         | 有在项目中应用的可能    |
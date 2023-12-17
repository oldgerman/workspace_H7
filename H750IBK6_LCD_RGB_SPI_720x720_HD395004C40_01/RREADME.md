## 在LVGL崩掉时，LTDC=CLK=36MHz时，LVGL缓冲区、LTDC内存、 都用 SDRAM2

```
[17:29:00.791] $SDRAM_TEST
[17:29:00.791] 
[17:29:00.791] 测试SDRAM1: 
[17:29:00.792] *****************************************************************************************************
[17:29:00.792] 进行速度测试>>>
[17:29:01.077] 以16位数据宽度写入数据，大小：32 MB，耗时: 285 ms, 写入速度：112.28 MB/s
[17:29:01.489] 读取数据完毕，大小：32 MB，耗时: 413 ms, 读取速度：77.48 MB/s
[17:29:01.490] *****************************************************************************************************
[17:29:01.490] 进行数据校验>>>
[17:29:02.030] 16位数据宽度读写通过，以8位数据宽度写入数据
[17:29:02.030] 写入完毕，读取数据并比较...
[17:29:02.030] 8位数据宽度读写通过
[17:29:02.030] SDRAM读写测试通过，系统正常
[17:29:02.030] 
[17:29:02.030] 测试SDRAM2: 
[17:29:02.030] *****************************************************************************************************
[17:29:02.030] 进行速度测试>>>
[17:29:02.364] 以16位数据宽度写入数据，大小：32 MB，耗时: 332 ms, 写入速度：96.39 MB/s
[17:29:04.832] 读取数据完毕，大小：32 MB，耗时: 2470 ms, 读取速度：12.96 MB/s
[17:29:04.832] *****************************************************************************************************
[17:29:04.833] 进行数据校验>>>
[17:29:07.674] 16位数据宽度读写通过，以8位数据宽度写入数据
[17:29:07.674] 写入完毕，读取数据并比较...
[17:29:07.674] 8位数据宽度读写通过
[17:29:07.674] SDRAM读写测试通过，系统正常
[17:29:15.592] $CPU_INFO
[17:29:15.593] ---------------------------------------------
[17:29:15.593] 任务名 任务状态 优先级 剩余栈 任务序号
[17:29:15.593] UsbServerTask                  	X	32	133	6
[17:29:15.593] ledTask                        	R	24	94	7
[17:29:15.593] IDLE                           	R	0	108	3
[17:29:15.593] commTask                       	B	24	71	5
[17:29:15.593] Tmr Svc                        	B	2	219	4
[17:29:15.593] usbIrqTask                     	B	32	91	1
[17:29:15.593] ---------------------------------------------
[17:29:15.594] 任务名 运行计数 使用率
[17:29:15.594] UsbServerTask                  	412854		3%
[17:29:15.594] ledTask                        	11125053		95%
[17:29:15.594] IDLE                           	60444		<1%
[17:29:15.594] commTask                       	0		<1%
[17:29:15.594] usbIrqTask                     	0		<1%
[17:29:15.594] Tmr Svc                        	0		<1%
[17:29:15.594] ---------------------------------------------
[17:29:15.594] 
```

## 在LVGL崩掉时，LTDC=CLK=18MHz时，LVGL缓冲区、LTDC内存、 都用 SDRAM2

```
[17:36:33.031] $SDRAM_TEST
[17:36:33.032] [led_task] sysTick 323 ms
[17:36:33.032] 
[17:36:33.032] 测试SDRAM1: 
[17:36:33.032] *****************************************************************************************************
[17:36:33.033] 进行速度测试>>>
[17:36:33.250] 以16位数据宽度写入数据，大小：32 MB，耗时: 216 ms, 写入速度：148.15 MB/s
[17:36:33.586] 读取数据完毕，大小：32 MB，耗时: 336 ms, 读取速度：95.24 MB/s
[17:36:33.586] *****************************************************************************************************
[17:36:33.586] 进行数据校验>>>
[17:36:34.041] 16位数据宽度读写通过，以8位数据宽度写入数据
[17:36:34.041] 写入完毕，读取数据并比较...
[17:36:34.041] 8位数据宽度读写通过
[17:36:34.041] SDRAM读写测试通过，系统正常
[17:36:34.041] 
[17:36:34.041] 测试SDRAM2: 
[17:36:34.041] *****************************************************************************************************
[17:36:34.041] 进行速度测试>>>
[17:36:34.300] 以16位数据宽度写入数据，大小：32 MB，耗时: 259 ms, 写入速度：123.55 MB/s
[17:36:36.256] 读取数据完毕，大小：32 MB，耗时: 1956 ms, 读取速度：16.36 MB/s
[17:36:36.256] *****************************************************************************************************
[17:36:36.256] 进行数据校验>>>
[17:36:38.510] 16位数据宽度读写通过，以8位数据宽度写入数据
[17:36:38.510] 写入完毕，读取数据并比较...
[17:36:38.510] 8位数据宽度读写通过
[17:36:38.510] SDRAM读写测试通过，系统正常
[17:36:40.881] $CPU_INFO
[17:36:40.882] ---------------------------------------------
[17:36:40.882] 任务名 任务状态 优先级 剩余栈 任务序号
[17:36:40.882] UsbServerTask                  	X	32	126	6
[17:36:40.882] ledTask                        	R	24	94	7
[17:36:40.882] IDLE                           	R	0	107	3
[17:36:40.882] commTask                       	B	24	71	5
[17:36:40.882] Tmr Svc                        	B	2	220	4
[17:36:40.882] usbIrqTask                     	B	32	91	1
[17:36:40.882] ---------------------------------------------
[17:36:40.882] 任务名 运行计数 使用率
[17:36:40.882] UsbServerTask                  	109538		18%
[17:36:40.882] ledTask                        	400290		67%
[17:36:40.882] IDLE                           	56482		9%
[17:36:40.883] commTask                       	0		<1%
[17:36:40.883] usbIrqTask                     	0		<1%
[17:36:40.883] Tmr Svc                        	0		<1%
[17:36:40.883] ---------------------------------------------
[17:36:40.883] 
```

## STM32 + LVGL 帧率提升策略 LTDC

[STM32 + LVGL 帧率提升策略 LTDC](https://www.bilibili.com/video/BV1nu41117Tx/?spm_id_from=333.337.search-card.all.click&vd_source=e6ad3ca74f59d33bf575de5aa7ceb52e)

## RGB屏 闪屏问题

NV3052C 在 跑LVGL卡死时，若使用 30帧 LTDC 自刷新滤率，则全屏会很快地轻微闪烁

NV3052C 在 跑LVGL卡死时，若使用 60帧 LTDC 自刷新滤率，则全屏没有肉眼可见闪烁

我估计 RGB 时钟太低导致 每两帧间刷新时，显示残影的时间延长了，导致亮度变化

## 硬汉

> # [【实战技能】基于硬件垂直消隐的多缓冲技术在LVGL, emWin，GUIX和TouchGFX应用，含视频教程](https://www.cnblogs.com/armfly/p/17565420.html)
>
> **原贴地址：https://www.armbbs.cn/forum.php?mod=viewthread&tid=120114**
>
> 这两天研究了下LVGL的持单缓冲，双缓冲和配合硬件消隐的双缓冲的实现（已经分享V5，V6和V7开发板的程序模板），特别是这个整屏缓冲方案，这几款GUI的实现基本是一样的，所以专门开了一期视频做个分享。
>
>
> **视频：**
> https://www.bilibili.com/video/BV1rF411Q7A7/

对于标志裸机可以用标志变量，RTOS必须要用信号量

对于硬件双缓冲垂直消隐，硬汉哥配置的缓冲区是

```c
/** LTDC 显存地址
  * void MX_LTDC_Init(void) 
  * pLayerCfg.FBStartAdress = LCD_MEM_ADDR;
  * 路径 C:\Users\PSA\Downloads\V7-6001_LVGL8 Template(V1.0)\User\bsp\src\bsp_tft_h7.c
/* 显存地址 */
pLayerCfg.FBStartAdress = LCDH7_FRAME_BUFFER;	// SDRAM 起始地址 0xC0000000，分配2MB给LTDC显存使用
#define LCDH7_FRAME_BUFFER		SDRAM_LCD_BUF1
/* LCD显存,第1页, 分配2M字节 */
#define SDRAM_LCD_BUF1 		EXT_SDRAM_ADDR
#define EXT_SDRAM_ADDR  	((uint32_t)0xC0000000)

/**
 * LVGL的双缓冲区，第一个缓冲区复用了 LTDC的显存，第二个缓冲区紧挨着第一个缓冲区，但 中断里也会切换 LTDC 的显存到第二个缓冲区
 * 路径：C:\Users\PSA\Downloads\V7-6001_LVGL8 Template(V1.0)\Project\MDK-ARM(AC5)\RTE\LVGL\lv_port_disp_template.c
 */
    /* Example for 3) also set disp_drv.full_refresh = 1 below*/
#elif defined Doublebuffering
    static lv_disp_draw_buf_t draw_buf_dsc_3; 
    static lv_color_t buf_3_1[MY_DISP_HOR_RES * MY_DISP_VER_RES] __MEMORY_AT(0xC0000000);  /*A screen sized buffer*/
    static lv_color_t buf_3_2[MY_DISP_HOR_RES * MY_DISP_VER_RES] __MEMORY_AT(0xC00BB800);  /*Another screen sized buffer*/

	lv_disp_draw_buf_init(&draw_buf_dsc_3, buf_3_1, buf_3_2,
							MY_DISP_VER_RES * MY_DISP_HOR_RES);   /*Initialize the display buffer*/
#endif
```



LTDC 中断优先级，硬汉配置是 14

```c
// c:\Users\PSA\Downloads\V7-6001_LVGL8 Template(V1.0)\User\bsp\src\bsp_tft_h7.c
/* 使能行中断 */

HAL_NVIC_SetPriority(LTDC_IRQn, 0xE, 0);
HAL_NVIC_EnableIRQ(LTDC_IRQn);
```

使能行中断位置

```c
void MX_LTDC_Init(void)
{
  /* USER CODE BEGIN LTDC_Init 2 */
  // 使能行中断位置
  HAL_LTDC_ProgramLineEvent(&hltdc, LCD_T_VPW + LCD_T_VBP + LTC_T_VD);
  /* USER CODE END LTDC_Init 2 */
}
```

## 只进入一次 HAL_LTDC_LineEventCallback 的问题

把 HAL 的 HAL_LTDC_IRQHandler(&hltdc); 注释掉，直接按照硬汉哥的，直接把这两行操作写在 LTDC_IRQHandler() 即可解决

```c
/**
  * @brief This function handles LTDC global interrupt.
  */
void LTDC_IRQHandler(void)
{
  /* USER CODE BEGIN LTDC_IRQn 0 */

  /* USER CODE END LTDC_IRQn 0 */
  //HAL_LTDC_IRQHandler(&hltdc);
  /* USER CODE BEGIN LTDC_IRQn 1 */
    HAL_LTDC_LineEventCallback(NULL);
  /* USER CODE END LTDC_IRQn 1 */
}

void HAL_LTDC_LineEventCallback(LTDC_HandleTypeDef *hltdc)
{
    LTDC->ICR = (uint32_t)LTDC_IER_LIE; // 清除中断标志

    /* 释放信号量 */
//    wTransferState = 1;
    osSemaphoreRelease(sem_ltdc_irq);
}
```

## 软件对运行帧率的影响因素

### LV_MEM_SIZE 动态内存空间不够

### 绘制的对象需要重绘的像素点多

### 计算量大

## 运行LVGL基准测试卡死的问题

我设置的64K运行会lv_demo_benchmark的Image示例屏幕会卡死：

```c

static scene_dsc_t scenes[] = {
......

//    {.name = "Image RGB",                    .weight = 20, .create_cb = img_rgb_cb},
//    {.name = "Image ARGB",                   .weight = 20, .create_cb = img_argb_cb},
//    {.name = "Image chorma keyed",           .weight = 5, .create_cb = img_ckey_cb},
//    {.name = "Image indexed",                .weight = 5, .create_cb = img_index_cb},
//    {.name = "Image alpha only",             .weight = 5, .create_cb = img_alpha_cb},
//
//    {.name = "Image RGB recolor",            .weight = 5, .create_cb = img_rgb_recolor_cb},
//    {.name = "Image ARGB recolor",           .weight = 20, .create_cb = img_argb_recolor_cb},
//    {.name = "Image chorma keyed recolor",   .weight = 3, .create_cb = img_ckey_recolor_cb},
//    {.name = "Image indexed recolor",        .weight = 3, .create_cb = img_index_recolor_cb},
//
//    {.name = "Image RGB rotate",             .weight = 3, .create_cb = img_rgb_rot_cb},
//    {.name = "Image RGB rotate anti aliased", .weight = 3, .create_cb = img_rgb_rot_aa_cb},
//    {.name = "Image ARGB rotate",            .weight = 5, .create_cb = img_argb_rot_cb},
//    {.name = "Image ARGB rotate anti aliased", .weight = 5, .create_cb = img_argb_rot_aa_cb},
//    {.name = "Image RGB zoom",               .weight = 3, .create_cb = img_rgb_zoom_cb},
//    {.name = "Image RGB zoom anti aliased",  .weight = 3, .create_cb = img_rgb_zoom_aa_cb},
//    {.name = "Image ARGB zoom",              .weight = 5, .create_cb = img_argb_zoom_cb},
//    {.name = "Image ARGB zoom anti aliased", .weight = 5, .create_cb = img_argb_zoom_aa_cb},
......
}
```

加大 lv_conf.h 的 LV_MEM_SIZE

[默认的 LV_MEM_SIZE 太小，无法完成运行基准测试 issue:3434](https://github.com/lvgl/lvgl/issues/3434)

但只加大这个 MEM SIZE 不行，还得指定 LVGL 在 H7 的哪块内存范围里动态分配内存，所以得指定内存池的起始地址（可以定义到H7的任意内存池地址）

```c
# define LV_MEM_ADR          0    
```

我觉得来个大手笔：把512K AXI SRAM 都给 LVGL

```c
#  define LV_MEM_SIZE    (512U * 1024U)          /*[bytes]*/

/*Set an address for the memory pool instead of allocating it as a normal array. Can be in external SRAM too.*/
#  define LV_MEM_ADR          0x24000000     /*0: unused*/
```

加大到512KB后，还是不能运行 Image 示例，算了，反正也不需要 LVGL 软件 PNG 或 JPG 解码，那么这个LVGL内存池可以改为 64KB 或硬汉的 100KB

```c

/*=========================
   MEMORY SETTINGS
 *=========================*/

/*1: use custom malloc/free, 0: use the built-in `lv_mem_alloc()` and `lv_mem_free()`*/
#define LV_MEM_CUSTOM      0
#if LV_MEM_CUSTOM == 0
/*Size of the memory available for `lv_mem_alloc()` in bytes (>= 2kB)*/
#  define LV_MEM_SIZE    (100U * 1024U)          /*[bytes]*/

/*Set an address for the memory pool instead of allocating it as a normal array. Can be in external SRAM too.*/
#  define LV_MEM_ADR          0     /*0: unused*/
    /*Instead of an address give a memory allocator that will be called to get a memory pool for LVGL. E.g. my_malloc*/
    #if LV_MEM_ADR == 0
        //#define LV_MEM_POOL_INCLUDE your_alloc_library  /* Uncomment if using an external allocator*/
        //#define LV_MEM_POOL_ALLOC   your_alloc          /* Uncomment if using an external allocator*/
    #endif

#else       /*LV_MEM_CUSTOM*/
#  define LV_MEM_CUSTOM_INCLUDE <stdlib.h>   /*Header for the dynamic memory function*/
#  define LV_MEM_CUSTOM_ALLOC     malloc
#  define LV_MEM_CUSTOM_FREE      free
#  define LV_MEM_CUSTOM_REALLOC   realloc
#endif     /*LV_MEM_CUSTOM*/

/*Number of the intermediate memory buffer used during rendering and other internal processing mechanisms.
 *You will see an error log message if there wasn't enough buffers. */
#define LV_MEM_BUF_MAX_NUM 16

/*Use the standard `memcpy` and `memset` instead of LVGL's own functions. (Might or might not be faster).*/
#define LV_MEMCPY_MEMSET_STD    0
```

[lvgl的内存管理函数的TLSF动态内存管理算法](https://blog.csdn.net/kelleo/article/details/122835525)

> lvgl的内存分配和释放提供了两套方案，可以通过lv_conf.h头文件中的宏LV_MEM_CUSTOM来控制使用哪个方案，
>
> 该宏定义值为0，则表示使用lvgl内置的内存分配函数lv_mem_alloc()和lv_mem_free()；
>
> 该宏定义值为1，则表示使用自定义“malloc()/free()/realloc()”
>
> [LiteOS内存管理：TLSF算法](https://www.jianshu.com/p/01743e834432)
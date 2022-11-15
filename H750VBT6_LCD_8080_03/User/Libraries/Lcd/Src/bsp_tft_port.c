/*
*********************************************************************************************************
*
*	模块名称 : TFT液晶显示器底层硬件驱动接口
*	文件名称 : bsp_tft_port.c
*	版    本 : V1.1
*	说    明 : 底层硬件驱动。不涉及控件绘制。
*	修改记录 :
*		版本号  日期        作者    说明
*		v1.0    2015-08-21 armfly  将和硬件紧密相关的函数从 bsp_tft_lcd.c 移到本文件
*		V1.1	2016-04-23 armfly  
*
*	Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"

/* 下面3个变量，主要用于使程序同时支持不同的屏 */
uint16_t g_ChipID = IC_9488;		/* 驱动芯片ID */
uint16_t g_LcdHeight = 480;			/* 显示屏分辨率-高度 */
uint16_t g_LcdWidth = 320;			/* 显示屏分辨率-宽度 */
uint8_t s_ucBright;					/* 背光亮度参数 */
uint8_t g_LcdDirection;				/* 显示方向.0，1，2，3 */

static void LCD_CtrlLinesConfig(void);
static void LCD_FSMCConfig(void);
static void LCD_HardReset(void);

void SOFT1_DrawCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t _usColor);
void SOFT_DrawQuterCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor, uint8_t _ucMode);

LCD_DEV_T g_tLCD;

/*
*********************************************************************************************************
*	函 数 名: LCD_InitHard
*	功能说明: 初始化LCD
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_InitHard(void)
{
	uint32_t id;

	/* 配置LCD控制口线GPIO */
	LCD_CtrlLinesConfig();

	/* 配置FSMC接口，数据总线 */
	LCD_FSMCConfig();

	LCD_HardReset();	/* 硬件复位 （STM32-V5 无需），针对其他GPIO控制LCD复位的产品 */
	
	/* FSMC重置后必须加延迟才能访问总线设备  */
	HAL_Delay(200);
	

	id = ILI9488_ReadID();
	
	if (id == 0x548066)
	{
		g_ChipID = IC_9488;
		ILI9488_InitHard();			/* 初始化ILI9488芯片 */
	}

	
	if (g_ChipID == IC_9488)
	{
		g_tLCD.DispOn = ILI9488_DispOn;
		g_tLCD.DispOff = ILI9488_DispOff;
		g_tLCD.ClrScr = ILI9488_ClrScr;
		g_tLCD.PutPixel = ILI9488_PutPixel;
		g_tLCD.GetPixel = ILI9488_GetPixel;
		g_tLCD.DrawLine = ILI9488_DrawLine;
		g_tLCD.DrawRect = ILI9488_DrawRect;
		g_tLCD.DrawCircle = ILI9488_DrawCircle;
		g_tLCD.DrawBMP = ILI9488_DrawBMP;
		g_tLCD.FillRect = ILI9488_FillRect;
		g_tLCD.FillCircle = ILI9488_FillCircle;
		g_tLCD.DrawHColorLine = ILI9488_DrawHColorLine;
	}

	ILI9488_SetDirection(3);
	//LCD_ClrScr(CL_BLUE);	 /* 清屏，显示全黑 */
	//LCD_SetBackLight(255); /* 打开背光，设置为缺省亮度 */
}

/*
*********************************************************************************************************
*	函 数 名: LCD_HardReset
*	功能说明: 硬件复位. 针对复位口线由GPIO控制的产品。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_HardReset(void)
{
#if 0	
	GPIO_InitTypeDef GPIO_InitStructure;

	/* 使能 GPIO时钟 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	
	/* 配置背光GPIO为推挽输出模式 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_ResetBits(GPIOB, GPIO_Pin_1);
	bsp_DelayMS(20);
	GPIO_SetBits(GPIOB, GPIO_Pin_1);
#endif	
}

/*
*********************************************************************************************************
*	函 数 名: LCD_GetChipDescribe
*	功能说明: 读取LCD驱动芯片的描述符号，用于显示
*	形    参: char *_str : 描述符字符串填入此缓冲区
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_GetChipDescribe(char *_str)
{
	switch (g_ChipID)
	{
		case IC_5420:
			strcpy(_str, CHIP_STR_5420);
			break;

		case IC_4001:
			strcpy(_str, CHIP_STR_4001);
			break;

		case IC_61509:
			strcpy(_str, CHIP_STR_61509);
			break;

		case IC_8875:
			strcpy(_str, CHIP_STR_8875);
			break;

		case IC_8876:
			strcpy(_str, CHIP_STR_8875);
			break;		

		case IC_9488:
			strcpy(_str, CHIP_STR_9488);
			break;

		default:
			strcpy(_str, "Unknow");
			break;
	}
}

/*
*********************************************************************************************************
*	函 数 名: LCD_GetHeight
*	功能说明: 读取LCD分辨率之高度
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
uint16_t LCD_GetHeight(void)
{
	return g_LcdHeight;
}

/*
*********************************************************************************************************
*	函 数 名: LCD_GetWidth
*	功能说明: 读取LCD分辨率之宽度
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
uint16_t LCD_GetWidth(void)
{
	return g_LcdWidth;
}

/*
*********************************************************************************************************
*	函 数 名: LCD_DispOn
*	功能说明: 打开显示
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DispOn(void)
{
	g_tLCD.DispOn();
}

/*
*********************************************************************************************************
*	函 数 名: LCD_DispOff
*	功能说明: 关闭显示
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DispOff(void)
{
	g_tLCD.DispOn();
}

/*
*********************************************************************************************************
*	函 数 名: LCD_ClrScr
*	功能说明: 根据输入的颜色值清屏
*	形    参: _usColor : 背景色
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_ClrScr(uint16_t _usColor)
{
	g_tLCD.ClrScr(_usColor);
}

/*
*********************************************************************************************************
*	函 数 名: LCD_PutPixel
*	功能说明: 画1个像素
*	形    参:
*			_usX,_usY : 像素坐标
*			_usColor  : 像素颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_PutPixel(uint16_t _usX, uint16_t _usY, uint16_t _usColor)
{
	g_tLCD.PutPixel(_usX, _usY, _usColor);
}

/*
*********************************************************************************************************
*	函 数 名: LCD_GetPixel
*	功能说明: 读取1个像素
*	形    参:
*			_usX,_usY : 像素坐标
*			_usColor  : 像素颜色
*	返 回 值: RGB颜色值
*********************************************************************************************************
*/
uint16_t LCD_GetPixel(uint16_t _usX, uint16_t _usY)
{
	return g_tLCD.GetPixel(_usX, _usY);
}

/*
*********************************************************************************************************
*	函 数 名: LCD_DrawLine
*	功能说明: 采用 Bresenham 算法，在2点间画一条直线。
*	形    参:
*			_usX1, _usY1 : 起始点坐标
*			_usX2, _usY2 : 终止点坐标
*			_usColor     : 颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DrawLine(uint16_t _usX1 , uint16_t _usY1 , uint16_t _usX2 , uint16_t _usY2 , uint16_t _usColor)
{
	g_tLCD.DrawLine(_usX1 , _usY1 , _usX2, _usY2 , _usColor);
}

/*
*********************************************************************************************************
*	函 数 名: LCD_DrawRect
*	功能说明: 绘制水平放置的矩形。
*	形    参:
*			_usX,_usY: 矩形左上角的坐标
*			_usHeight : 矩形的高度
*			_usWidth  : 矩形的宽度
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DrawRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t _usColor)
{
	g_tLCD.DrawRect(_usX, _usY, _usHeight, _usWidth, _usColor);
}

/*
*********************************************************************************************************
*	函 数 名: LCD_Fill_Rect
*	功能说明: 用一个颜色值填充一个矩形。【emWin 中有同名函数 LCD_FillRect，因此加了下划线区分】
*	形    参:
*			_usX,_usY: 矩形左上角的坐标
*			_usHeight : 矩形的高度
*			_usWidth  : 矩形的宽度
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_Fill_Rect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t _usColor)
{
	g_tLCD.FillRect(_usX, _usY, _usHeight, _usWidth, _usColor);
}

/*
*********************************************************************************************************
*	函 数 名: LCD_DrawCircle
*	功能说明: 绘制一个圆，笔宽为1个像素
*	形    参:
*			_usX,_usY  : 圆心的坐标
*			_usRadius  : 圆的半径
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DrawCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor)
{
	g_tLCD.DrawCircle(_usX, _usY, _usRadius, _usColor);
}

/*
*********************************************************************************************************
*	函 数 名: LCD_FillCircle
*	功能说明: 填充一个圆
*	形    参:
*			_usX,_usY  : 圆心的坐标
*			_usRadius  : 圆的半径
*			_usColor   : 填充的颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void _LCD_FillCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor)
{
	g_tLCD.FillCircle(_usX, _usY, _usRadius, _usColor);
}

/*
*********************************************************************************************************
*	函 数 名: LCD_DrawBMP
*	功能说明: 在LCD上显示一个BMP位图，位图点阵扫描次序: 从左到右，从上到下
*	形    参:
*			_usX, _usY : 图片的坐标
*			_usHeight  : 图片高度
*			_usWidth   : 图片宽度
*			_ptr       : 图片点阵指针
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DrawBMP(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t *_ptr)
{
	g_tLCD.DrawBMP(_usX, _usY, _usHeight, _usWidth, _ptr);
}

/*
*********************************************************************************************************
*	函 数 名: LCD_CtrlLinesConfig
*	功能说明: 配置LCD控制口线，FSMC管脚设置为复用功能
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
/*
	安富莱STM32-V5开发板接线方法:

	PD0/FSMC_D2
	PD1/FSMC_D3
	PD4/FSMC_NOE		--- 读控制信号，OE = Output Enable ， N 表示低有效
	PD5/FSMC_NWE		--- 写控制信号，WE = Output Enable ， N 表示低有效
	PD8/FSMC_D13
	PD9/FSMC_D14
	PD10/FSMC_D15
	PD13/FSMC_A18		--- 地址 RS
	PD14/FSMC_D0
	PD15/FSMC_D1

	PE4/FSMC_A20		--- 和主片选一起译码
	PE5/FSMC_A21		--- 和主片选一起译码
	PE7/FSMC_D4
	PE8/FSMC_D5
	PE9/FSMC_D6
	PE10/FSMC_D7
	PE11/FSMC_D8
	PE12/FSMC_D9
	PE13/FSMC_D10
	PE14/FSMC_D11
	PE15/FSMC_D12

	PG12/FSMC_NE4		--- 主片选（TFT, OLED 和 AD7606）

	PI3/TP_INT			--- 触摸芯片中断 （对于RA8875屏，是RA8875输出的中断)  本例程未使用硬件中断

	---- 下面是 TFT LCD接口其他信号 （FSMC模式不使用）----
	PD3/LCD_BUSY		--- 触摸芯片忙       （RA8875屏是RA8875芯片的忙信号)
	PF6/LCD_PWM			--- LCD背光PWM控制  （RA8875屏无需此脚，背光由RA8875控制)

	PI10/TP_NCS			--- 触摸芯片的片选		(RA8875屏无需SPI接口触摸芯片）
	PB3/SPI3_SCK		--- 触摸芯片SPI时钟		(RA8875屏无需SPI接口触摸芯片）
	PB4/SPI3_MISO		--- 触摸芯片SPI数据线MISO(RA8875屏无需SPI接口触摸芯片）
	PB5/SPI3_MOSI		--- 触摸芯片SPI数据线MOSI(RA8875屏无需SPI接口触摸芯片）
*/
static void LCD_CtrlLinesConfig(void)
{
	//由CubeMX完成
#if 0
	GPIO_InitTypeDef gpio_init_structure;

	/* 使能 GPIO时钟 */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();

	/* 使能FMC时钟 */
	__HAL_RCC_FSMC_CLK_ENABLE();

	/* 设置 GPIOD 相关的IO为复用推挽输出 */
	gpio_init_structure.Mode = GPIO_MODE_AF_PP;
	gpio_init_structure.Pull = GPIO_PULLUP;
	gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	gpio_init_structure.Alternate = GPIO_AF12_FSMC;

	/* 配置GPIOD */
	gpio_init_structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5  |
	                            GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_13 | GPIO_PIN_14 |
	                            GPIO_PIN_15;
	HAL_GPIO_Init(GPIOD, &gpio_init_structure);

	/* 配置GPIOE */
	gpio_init_structure.Pin = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 |
	                          GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	HAL_GPIO_Init(GPIOE, &gpio_init_structure);

	/* 配置GPIOG */
	gpio_init_structure.Pin = GPIO_PIN_12;
	HAL_GPIO_Init(GPIOG, &gpio_init_structure);
#endif
}

/*
*********************************************************************************************************
*	函 数 名: LCD_FSMCConfig
*	功能说明: 配置FSMC并口访问时序
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void LCD_FSMCConfig(void)
{
	//由CubeMX完成
#if 0
/* 
	   TFT-LCD，OLED和AD7606公用一个FMC配置，如果都开启，请以FMC速度最慢的为准。
	   从而保证所有外设都可以正常工作。
	*/
	SRAM_HandleTypeDef hsram = {0};
	FMC_NORSRAM_TimingTypeDef SRAM_TimingRead = {0};
	FMC_NORSRAM_TimingTypeDef SRAM_TimingWrite = {0};
		
	hsram.Instance  = FMC_NORSRAM_DEVICE;
	hsram.Extended  = FMC_NORSRAM_EXTENDED_DEVICE;
	
	/* FMC使用的HCLK，主频168MHz，1个FMC时钟周期就是5.95ns */
	SRAM_TimingRead.AddressSetupTime       = 4;  /* 4*5.95ns，地址建立时间，范围0 -15个FMC时钟周期个数 */
	SRAM_TimingRead.AddressHoldTime        = 0;  /* 地址保持时间，配置为模式A时，用不到此参数 范围1 -15个时钟周期个数 */
//	SRAM_TimingRead.DataSetupTime          = 8;  /* 6*5.95ns，数据保持时间，范围1 -255个时钟周期个数 */
	SRAM_TimingRead.DataSetupTime          = 6;  /* 6*5.95ns，数据保持时间，范围1 -255个时钟周期个数 */
	SRAM_TimingRead.BusTurnAroundDuration  = 1;  /* 两个连续数据读写的时间间隔*/
	SRAM_TimingRead.CLKDivision            = 0;  /* 此配置用不到这个参数 */
	SRAM_TimingRead.DataLatency            = 0;  /* 此配置用不到这个参数 */
	SRAM_TimingRead.AccessMode             = FSMC_ACCESS_MODE_A; /* 配置为模式A */
	
	SRAM_TimingWrite.AddressSetupTime       = 4;  /* 4*5.95ns，地址建立时间，范围0 -15个FMC时钟周期个数 */
	SRAM_TimingWrite.AddressHoldTime        = 0;  /* 地址保持时间，配置为模式A时，用不到此参数 范围1 -15个时钟周期个数 */
	SRAM_TimingWrite.DataSetupTime          = 6;  /* 8*5.95ns，数据保持时间，范围1 -255个时钟周期个数 */
	SRAM_TimingWrite.BusTurnAroundDuration  = 1;  /* 两个连续数据读写的时间间隔*/
	SRAM_TimingWrite.CLKDivision            = 0;  /* 此配置用不到这个参数 */
	SRAM_TimingWrite.DataLatency            = 0;  /* 此配置用不到这个参数 */
	SRAM_TimingWrite.AccessMode             = FSMC_ACCESS_MODE_A; /* 配置为模式A */

	hsram.Init.NSBank             = FSMC_NORSRAM_BANK4;              /* 使用的BANK4，即使用的片选FSMC_NE4 */
	hsram.Init.DataAddressMux     = FSMC_DATA_ADDRESS_MUX_DISABLE;   /* 禁止地址数据复用 */
	hsram.Init.MemoryType         = FSMC_MEMORY_TYPE_SRAM;           /* 存储器类型SRAM */
	hsram.Init.MemoryDataWidth    = FSMC_NORSRAM_MEM_BUS_WIDTH_16;	/* 16位总线宽度 */
	hsram.Init.BurstAccessMode    = FSMC_BURST_ACCESS_MODE_DISABLE;  /* 关闭突发模式 */
	hsram.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;   /* 用于设置等待信号的极性，关闭突发模式，此参数无效 */
	hsram.Init.WaitSignalActive   = FSMC_WAIT_TIMING_BEFORE_WS;      /* 关闭突发模式，此参数无效 */
	hsram.Init.WriteOperation     = FSMC_WRITE_OPERATION_ENABLE;     /* 用于使能或者禁止写保护 */
	hsram.Init.WaitSignal         = FSMC_WAIT_SIGNAL_DISABLE;        /* 关闭突发模式，此参数无效 */
	hsram.Init.ExtendedMode       = FSMC_EXTENDED_MODE_DISABLE;      /* 禁止扩展模式 */
	hsram.Init.AsynchronousWait   = FSMC_ASYNCHRONOUS_WAIT_DISABLE;  /* 用于异步传输期间，使能或者禁止等待信号，这里选择关闭 */
	hsram.Init.WriteBurst         = FSMC_WRITE_BURST_DISABLE;        /* 禁止写突发 */
	hsram.Init.ContinuousClock    = FSMC_CONTINUOUS_CLOCK_SYNC_ONLY; /* 仅同步模式才做时钟输出 */
    hsram.Init.WriteFifo          = FSMC_WRITE_FIFO_ENABLE;          /* 使能写FIFO */

	/* 初始化SRAM控制器 */
	if (HAL_SRAM_Init(&hsram, &SRAM_TimingRead, &SRAM_TimingWrite) != HAL_OK)
	{
		/* 初始化错误 */
		Error_Handler(__FILE__, __LINE__);
	}	
#endif
}

#if 0
/*
*********************************************************************************************************
*	函 数 名: LCD_SetPwmBackLight
*	功能说明: 初始化控制LCD背景光的GPIO,配置为PWM模式。
*			当关闭背光时，将CPU IO设置为浮动输入模式（推荐设置为推挽输出，并驱动到低电平)；将TIM3关闭 省电
*	形    参:  _bright 亮度，0是灭，255是最亮
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_SetPwmBackLight(uint8_t _bright)
{
	#if 0	/* P02 */
		bsp_SetTIMOutPWM(GPIOC, GPIO_Pin_6, TIM3, 1, 100, (_bright * 10000) /255);
	#else
		bsp_SetTIMOutPWM(GPIOF, GPIO_PIN_6, TIM10, 1, 100, (_bright * 10000) /255);
	#endif
}

/*
*********************************************************************************************************
*	函 数 名: LCD_SetBackLight
*	功能说明: 初始化控制LCD背景光的GPIO,配置为PWM模式。
*			当关闭背光时，将CPU IO设置为浮动输入模式（推荐设置为推挽输出，并驱动到低电平)；将TIM3关闭 省电
*	形    参: _bright 亮度，0是灭，255是最亮
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_SetBackLight(uint8_t _bright)
{
	s_ucBright =  _bright;	/* 保存背光值 */

	if (g_ChipID == IC_8875)
	{
		RA8875_SetBackLight(_bright);
	}
	else if (g_ChipID == IC_8876)
	{
		//RA8876_SetBackLight(_bright);
	}
	else
	{
		LCD_SetPwmBackLight(_bright);
	}
}

/*
*********************************************************************************************************
*	函 数 名: LCD_GetBackLight
*	功能说明: 获得背光亮度参数
*	形    参: 无
*	返 回 值: 背光亮度参数
*********************************************************************************************************
*/
uint8_t LCD_GetBackLight(void)
{
	return s_ucBright;
}

/*
*********************************************************************************************************
*	函 数 名: LCD_SetDirection
*	功能说明: 设置显示屏显示方向（横屏 竖屏）
*	形    参: 显示方向代码 0 横屏正常, 1=横屏180度翻转, 2=竖屏, 3=竖屏180度翻转
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_SetDirection(uint8_t _dir)
{
	g_LcdDirection =  _dir;		/* 保存在全局变量 */

	if (g_ChipID == IC_8875)
	{
		RA8875_SetDirection(_dir);
	}
	else if (g_ChipID == IC_8876)
	{
		//RA8876_SetDirection(_dir);
	}
	else
	{
		//ILI9488_SetDirection(_dir);
	}
}

#endif
/*
*********************************************************************************************************
*	函 数 名: SOFT_DrawLine
*	功能说明: 采用 Bresenham 算法，在2点间画一条直线。
*	形    参:
*			_usX1, _usY1 ：起始点坐标
*			_usX2, _usY2 ：终止点Y坐标
*			_usColor     ：颜色
*	返 回 值: 无
*
*	注释：布雷森汉姆直线算法
		用来描绘由两点所决定的直线的算法，它会算出一条线段在n维位图上最接近的点。
		这个算法只会用到较为快速的整数加法、减法和位元移位，常用于绘制电脑画面中的直线。
		是计算机图形学中最先发展出来的算法。 经过少量的延伸之后，原本用来画直线的算法也可用来画圆
*********************************************************************************************************
*/
void SOFT_DrawLine(uint16_t _usX1 , uint16_t _usY1 , uint16_t _usX2 , uint16_t _usY2 , uint16_t _usColor)
{
	int32_t dx , dy ;
	int32_t tx , ty ;
	int32_t inc1 , inc2 ;
	int32_t d , iTag ;
	int32_t x , y ;

	/* 采用 Bresenham 算法，在2点间画一条直线 */

	g_tLCD.PutPixel(_usX1 , _usY1 , _usColor);

	/* 如果两点重合，结束后面的动作。*/
	if ( _usX1 == _usX2 && _usY1 == _usY2 )
	{
		return;
	}
	
	/* 绘制垂直线 */
	if (_usX1 == _usX2)
	{
		if (_usY2 > _usY1)
		{
			for (y = _usY1; y <= _usY2; y++)
			{
				g_tLCD.PutPixel(_usX1, y, _usColor);
			}
		}
		else
		{
			for (y = _usY2; y <= _usY1; y++)
			{
				g_tLCD.PutPixel(_usX1, y, _usColor);
			}			
		}
	}
	
	/* 绘制水平线 */
	if (_usY1 == _usY2)
	{
		if (_usX2 > _usX1)
		{
			for (x = _usX1; x <= _usX2; x++)
			{
				g_tLCD.PutPixel(x, _usY1, _usColor);
			}
		}
		else
		{
			for (x = _usX2; x <= _usX1; x++)
			{
				g_tLCD.PutPixel(x, _usY1, _usColor);
			}			
		}
	}	

	iTag = 0 ;
	/* dx = abs ( _usX2 - _usX1 ); */
	if (_usX2 >= _usX1)
	{
		dx = _usX2 - _usX1;
	}
	else
	{
		dx = _usX1 - _usX2;
	}

	/* dy = abs ( _usY2 - _usY1 ); */
	if (_usY2 >= _usY1)
	{
		dy = _usY2 - _usY1;
	}
	else
	{
		dy = _usY1 - _usY2;
	}

	if ( dx < dy )   /*如果dy为计长方向，则交换纵横坐标。*/
	{
		uint16_t temp;

		iTag = 1 ;
		temp = _usX1; _usX1 = _usY1; _usY1 = temp;
		temp = _usX2; _usX2 = _usY2; _usY2 = temp;
		temp = dx; dx = dy; dy = temp;
	}
	tx = _usX2 > _usX1 ? 1 : -1 ;    /* 确定是增1还是减1 */
	ty = _usY2 > _usY1 ? 1 : -1 ;
	x = _usX1 ;
	y = _usY1 ;
	inc1 = 2 * dy ;
	inc2 = 2 * ( dy - dx );
	d = inc1 - dx ;
	while ( x != _usX2 )     /* 循环画点 */
	{
		if ( d < 0 )
		{
			d += inc1 ;
		}
		else
		{
			y += ty ;
			d += inc2 ;
		}
		if ( iTag )
		{
			g_tLCD.PutPixel( y , x , _usColor) ;
		}
		else
		{
			g_tLCD.PutPixel( x , y , _usColor) ;
		}
		x += tx ;
	}
}

/*
*********************************************************************************************************
*	函 数 名: SOFT_DrawCircle
*	功能说明: 绘制一个圆，笔宽为1个像素, 使用软件算法绘制，没用驱动IC的硬件功能
*	形    参:
*			_usX,_usY  : 圆心的坐标
*			_usRadius  : 圆的半径
*	返 回 值: 无
*********************************************************************************************************
*/
void SOFT_DrawCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor)
{
	int32_t  D;			/* Decision Variable */
	uint32_t  CurX;		/* 当前 X 值 */
	uint32_t  CurY;		/* 当前 Y 值 */

	D = 3 - (_usRadius << 1);
	
	CurX = 0;
	CurY = _usRadius;

	while (CurX <= CurY)
	{
		g_tLCD.PutPixel(_usX + CurX, _usY + CurY, _usColor);
		g_tLCD.PutPixel(_usX + CurX, _usY - CurY, _usColor);
		g_tLCD.PutPixel(_usX - CurX, _usY + CurY, _usColor);
		g_tLCD.PutPixel(_usX - CurX, _usY - CurY, _usColor);
		g_tLCD.PutPixel(_usX + CurY, _usY + CurX, _usColor);
		g_tLCD.PutPixel(_usX + CurY, _usY - CurX, _usColor);
		g_tLCD.PutPixel(_usX - CurY, _usY + CurX, _usColor);
		g_tLCD.PutPixel(_usX - CurY, _usY - CurX, _usColor);

		if (D < 0)
		{
			D += (CurX << 2) + 6;
		}
		else
		{
			D += ((CurX - CurY) << 2) + 10;
			CurY--;
		}
		CurX++;
	}
}

/*
*********************************************************************************************************
*	函 数 名: SOFT_DrawQuterCircle
*	功能说明: 绘制一个1/4圆，笔宽为1个像素, 使用软件算法绘制
*	形    参:
*			_usX,_usY  : 圆心的坐标
*			_usRadius  : 圆的半径
*			_ucMode    : 0 表示左上角1/4圆 1表示右上角  2表示右下角 3表示左下角
*	返 回 值: 无
*********************************************************************************************************
*/
void SOFT_DrawQuterCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor, uint8_t _ucMode)
{
	int32_t  D;			/* Decision Variable */
	uint32_t  CurX;		/* 当前 X 值 */
	uint32_t  CurY;		/* 当前 Y 值 */

	D = 3 - (_usRadius << 1);
	
	CurX = 0;
	CurY = _usRadius;

	while (CurX <= CurY)
	{	
		if (_ucMode == 0)
		{
			g_tLCD.PutPixel(_usX - CurY, _usY - CurX, _usColor);   // 左 -> 上
			g_tLCD.PutPixel(_usX - CurX, _usY - CurY, _usColor);   // 上 -> 左
		}
		else if (_ucMode == 1)
		{
			g_tLCD.PutPixel(_usX + CurX, _usY - CurY, _usColor);	// 上 -> 右
			g_tLCD.PutPixel(_usX + CurY, _usY - CurX, _usColor);	// 右 -> 上	
		}
		else if (_ucMode == 2)
		{
			g_tLCD.PutPixel(_usX + CurX, _usY + CurY, _usColor);	// 下 -> 右
			g_tLCD.PutPixel(_usX + CurY, _usY + CurX, _usColor);	// 右 -> 下
		}
		else if (_ucMode == 3)
		{			
			g_tLCD.PutPixel(_usX - CurX, _usY + CurY, _usColor);	// 下 -> 左
			g_tLCD.PutPixel(_usX - CurY, _usY + CurX, _usColor);    // 左 -> 下
		}
		
		if (D < 0)
		{
			D += (CurX << 2) + 6;
		}
		else
		{
			D += ((CurX - CurY) << 2) + 10;
			CurY--;
		}
		CurX++;
	}
}

/*
*********************************************************************************************************
*	函 数 名: SOFT_FillCircle
*	功能说明: 填充一个圆，软件算法实现。
*	形    参:
*			_usX,_usY  : 圆心的坐标
*			_usRadius  : 圆的半径
*			_usColor   : 填充的颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void SOFT_FillCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor)
{
	int32_t  D;
	uint32_t  CurX;		/* 当前 X 值 */
	uint32_t  CurY;		/* 当前 Y 值 */

	D = 3 - (_usRadius << 1);
	CurX = 0;
	CurY = _usRadius;

	while (CurX <= CurY)
	{			
		g_tLCD.DrawLine(_usX + CurX, _usY + CurY, _usX - CurX, _usY + CurY, _usColor);
		g_tLCD.DrawLine(_usX + CurX, _usY - CurY, _usX - CurX, _usY - CurY, _usColor);
		g_tLCD.DrawLine(_usX + CurY, _usY + CurX, _usX - CurY, _usY + CurX, _usColor);
		g_tLCD.DrawLine(_usX + CurY, _usY - CurX, _usX - CurY, _usY - CurX, _usColor);  	

		if (D < 0)
		{
			D += (CurX << 2) + 6;
		}
		else
		{
			D += ((CurX - CurY) << 2) + 10;
			CurY--;
		}
		CurX++;
	}
}

/*
*********************************************************************************************************
*	函 数 名: SOFT_FillQuterCircle
*	功能说明: 填充一个1/4圆，软件算法实现。
*	形    参:
*			_usX,_usY  : 圆心的坐标
*			_usRadius  : 圆的半径
*			_usColor   : 填充的颜色
*			_ucMode    : 0 表示左上角1/4圆 1表示右上角  2表示左下角 3表示右下角
*	返 回 值: 无
*********************************************************************************************************
*/
void SOFT_FillQuterCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor, uint8_t _ucMode)
{
	int32_t  D;
	uint32_t  CurX;		/* 当前 X 值 */
	uint32_t  CurY;		/* 当前 Y 值 */

	D = 3 - (_usRadius << 1);
	CurX = 0;
	CurY = _usRadius;

	while (CurX <= CurY)
	{			
		if (_ucMode == 0)
		{
			g_tLCD.DrawLine(_usX - CurY, _usY - CurX, _usX, _usY - CurX, _usColor);   // 左 -> 上
			g_tLCD.DrawLine(_usX - CurX, _usY - CurY, _usX, _usY - CurY, _usColor);   // 上 -> 左
		}
		else if (_ucMode == 1)
		{
			g_tLCD.DrawLine(_usX + CurX, _usY - CurY, _usX, _usY - CurY, _usColor);	// 上 -> 右
			g_tLCD.DrawLine(_usX + CurY, _usY - CurX, _usX, _usY - CurX, _usColor);	// 右 -> 上	
		}
		else if (_ucMode == 2)
		{
			g_tLCD.DrawLine(_usX + CurX, _usY + CurY, _usX, _usY + CurY, _usColor);	// 下 -> 右
			g_tLCD.DrawLine(_usX + CurY, _usY + CurX, _usX, _usY + CurX, _usColor);	// 右 -> 下
		}
		else if (_ucMode == 3)
		{			
			g_tLCD.DrawLine(_usX - CurX, _usY + CurY, _usX, _usY + CurY, _usColor);	// 下 -> 左
			g_tLCD.DrawLine(_usX - CurY, _usY + CurX, _usX, _usY + CurX, _usColor);    // 左 -> 下
		}		
		
		if (D < 0)
		{
			D += (CurX << 2) + 6;
		}
		else
		{
			D += ((CurX - CurY) << 2) + 10;
			CurY--;
		}
		CurX++;
	}
}

/*
*********************************************************************************************************
*	函 数 名: SOFT_DrawRoundRect
*	功能说明: 绘制圆角矩形轮廓，笔宽度1像素
*	形    参:
*			_usX,_usY:矩形左上角的坐标
*			_usHeight :矩形的高度
*			_usWidth  :矩形的宽度
*			_usArc    :圆角的弧半径
*			_usColor  :颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void SOFT_DrawRoundRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, 
	uint16_t _usRadius, uint16_t _usColor)
{
	if (_usHeight < 2 *_usRadius)
	{
		_usHeight = 2 *_usRadius;
	}

	if (_usWidth < 2 *_usRadius)
	{
		_usWidth = 2 *_usRadius;
	}	
	
	SOFT_DrawQuterCircle(_usX + _usRadius, _usY + _usRadius, _usRadius, _usColor, 0);	/* 左上角的弧 */
	g_tLCD.DrawLine(_usX + _usRadius, _usY, _usX + _usWidth - _usRadius - 1, _usY, _usColor);
	
	SOFT_DrawQuterCircle(_usX + _usWidth - _usRadius - 1, _usY + _usRadius, _usRadius, _usColor, 1);	/* 右上角的弧 */
	g_tLCD.DrawLine(_usX + _usWidth - 1, _usY + _usRadius, _usX + _usWidth - 1, _usY + _usHeight  - _usRadius - 1, _usColor);
	
	SOFT_DrawQuterCircle(_usX + _usWidth - _usRadius - 1, _usY + _usHeight - _usRadius - 1, _usRadius, _usColor, 2);	/* 右下角的弧 */
	g_tLCD.DrawLine(_usX + _usRadius, _usY + _usHeight - 1, _usX + _usWidth - _usRadius - 1, _usY + _usHeight - 1, _usColor);
			
	SOFT_DrawQuterCircle(_usX + _usRadius,  _usY + _usHeight - _usRadius - 1, _usRadius, _usColor, 3);	/* 左下角的弧 */
	g_tLCD.DrawLine(_usX, _usY + _usRadius, _usX,  _usY + _usHeight - _usRadius - 1, _usColor);
}


/*
*********************************************************************************************************
*	函 数 名: SOFT_FillRoundRect
*	功能说明: 填充圆角矩形
*	形    参:
*			_usX,_usY:矩形左上角的坐标
*			_usHeight :矩形的高度
*			_usWidth  :矩形的宽度
*			_usArc    :圆角的弧半径
*			_usColor  :颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void SOFT_FillRoundRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, 
	uint16_t _usRadius, uint16_t _usColor)
{
	if (_usHeight < 2 *_usRadius)
	{
		_usHeight = 2 *_usRadius;
	}

	if (_usWidth < 2 *_usRadius)
	{
		_usWidth = 2 *_usRadius;
	}	
	
	SOFT_FillQuterCircle(_usX + _usRadius, _usY + _usRadius, _usRadius, _usColor, 0);	/* 左上角的弧 */

	g_tLCD.FillRect(_usX + _usRadius + 1,  _usY,  _usRadius + 1, _usWidth - 2 * _usRadius - 2, _usColor);
	
	SOFT_FillQuterCircle(_usX + _usWidth - _usRadius - 1, _usY + _usRadius, _usRadius, _usColor, 1);	/* 右上角的弧 */

	g_tLCD.FillRect(_usX, _usY + _usRadius, _usHeight - 2 * _usRadius, _usWidth, _usColor);

	SOFT_FillQuterCircle(_usX + _usWidth - _usRadius - 1, _usY + _usHeight - _usRadius - 1, _usRadius, _usColor, 2);	/* 右下角的弧 */

	g_tLCD.FillRect(_usX + _usRadius + 1,  _usY + _usHeight - _usRadius - 1,  _usRadius + 1, _usWidth - 2 * _usRadius - 2, _usColor);	

	SOFT_FillQuterCircle(_usX + _usRadius,  _usY + _usHeight - _usRadius - 1, _usRadius, _usColor, 3);	/* 左下角的弧 */
}


/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/

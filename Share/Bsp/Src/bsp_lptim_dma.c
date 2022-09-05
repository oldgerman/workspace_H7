/*
 * bsp_tim_dma.c
 *
 *  Created on: Sep 4, 2022
 *      Author: OldGerman
 */

#include "bsp_config.h"
#ifdef EN_BSP_LPTIM_DMA
#include "bsp.h"

/**
 * @brief   将一串数据视作bit，bit代表电平，每一个bit都转换为BSRR数据组的元素
 * 			虽然会将数据整个写入GPIO组的BSSR，但是BSSR寄存器很特殊，只能写1，清0无效，将来只作用于GPIO_Pin指定的引脚，对本GPIO组的其他GPIO_PIN无影响
 * @param	ptrBitArray			源数据指针
 * @param	bitLengthInByte		源数据长度，以byte为单位
 * @param	ptrBSRRArray		目标数据组
 * @param   GPIO_Pin: specifies the port bit
 *          This parameter can be GPIO_PIN_x where x can be (0..15).
 *          	备注：HAL库GPIO_Pin_x宏定义特点
 *          		#define GPIO_PIN_0                 ((uint16_t)0x0001)  // Pin 0 selected    0...001
 *          		#define GPIO_PIN_1                 ((uint16_t)0x0002)  // Pin 1 selected    0...010
 *          		#define GPIO_PIN_2                 ((uint16_t)0x0004)  // Pin 2 selected    0...100
 *          		......
 *          		#define GPIO_PIN_15                ((uint16_t)0x8000)  // Pin 15 selected 	1...000
 * @retval None
 */
void bsp_Lptim_DMA_convertLevelToBSRR(uint8_t *ptrBitArray, uint32_t bitArrayLengthInByte, uint32_t* ptrBSRRArray, uint16_t GPIO_Pin){
	/* BSRR 高 16 位用于控制GPIO的输出低电平，低 16 位用于输出高电平工作 */
	uint32_t numBit = bitArrayLengthInByte * 8;
	for(uint32_t i = 0; i < numBit; i += 8)
	{
		uint32_t j = i / 8;
		for(uint8_t bitOffset = 0; bitOffset < 8; bitOffset++){
			if( ((*(ptrBitArray + j)) >> (7 - bitOffset)) & 0x0001) {
				*(ptrBSRRArray + i + bitOffset) = (uint32_t)GPIO_Pin;			//输出高电平，设置BS0~BS15  BSRR[0:15]
			}
			else {
				*(ptrBSRRArray + i + bitOffset) = (uint32_t)GPIO_Pin << 16;		//输出低电平，设置BR0~BR15  BSRR[16:31]
			}
		}
	}
}


/**
 * @brief  单独设置DMA
 * @param  hdma				  	DMA句柄，支持BDMA、DMA1、DMA2，注意DMA能访问的内存域，建议用BDMA以节省其他高级DMA
 * @param  SrcAddress			源地址
 * @param  DstAddress			目标地址
 * @param  DataLength			数据长度
 * @retval HAL Status
 */
HAL_StatusTypeDef bsp_Lptim_DMA_DMA_Set(
		DMA_HandleTypeDef *hdma, uint32_t SrcAddress, uint32_t DstAddress, uint32_t DataLength,
		void (*XferCpltCallback)(DMA_HandleTypeDef * hdma),
		void (*XferHalfCpltCallback)(DMA_HandleTypeDef * hdma))
{
	HAL_StatusTypeDef status = HAL_OK;

	/* 注册回调函数 */
	status += HAL_DMA_RegisterCallback(hdma, HAL_DMA_XFER_CPLT_CB_ID, XferCpltCallback);			/*!< Full transfer     */
	status += HAL_DMA_RegisterCallback(hdma, HAL_DMA_XFER_HALFCPLT_CB_ID, XferHalfCpltCallback);	/*!< Half Transfer     */

	/* 使能DMA请求发生器，注意CubeMX不会在自动生成的代码中调用此函数，需要用户自行在别处调用 */
	status += HAL_DMAEx_EnableMuxRequestGenerator (hdma);

	/* 启动DMA传输，若不使能DMA请求发生器，则DMA无法开始传输 */
	status += HAL_DMA_Start_IT(hdma, SrcAddress, DstAddress, DataLength);

	return status;
}

/**
 * @brief  单独设置PWM，占空比建议固定为50%
 * @param  hlptim				hlptim句柄指针
 * @param  LptimClockFreq		lptim的时钟源频率，单位Hz （这个可以在CubeMX的时钟树配置并看到频率，因为从hlptim获取LPTIMx时钟源频率很麻烦，所以麻烦你一下~）
 * @param  pwmFrequency 		pwm频率，  范围 1 ~ LptimClockFreq	单位: Hz
 * @param  pwmDutyCycle  		pwm占空比，范围 0.0... ~ 100.0...，	单位: %
 * @retval pwmSet_InfoTypeDef 	经计算后的情况
 */
pwmSet_InfoTypeDef bsp_Lptim_DMA_PWM_Set(
		LPTIM_HandleTypeDef *hlptim, uint32_t LptimClockFreq, uint32_t pwmFrequency, float pwmDutyCycle)
{
	/* 配置LPTIM触发DMAMUX
	 * 占空比建议固定为50%，方便使用数组合成LPTIM PWM 1/2 周期整数倍数的GPIO脉冲
	 */
	return bsp_LPTIMx_PWM_Set(hlptim, LptimClockFreq, pwmFrequency, pwmDutyCycle);
}

/**
 * @brief  打开或关闭LPTIM以开关DMA PWM，仅通过开关LPTIM PWM就行
 * @param  hlptim	hlptim句柄指针
 * @retval HAL Status
 */
HAL_StatusTypeDef bsp_Lptim_DMA_PWM_En(LPTIM_HandleTypeDef *hlptim, bool enable){
	 return bsp_LPTIMx_PWM_En(hlptim, enable);
}


#if 0
//暂时没卵用的
static LPTIM_HandleTypeDef *hlptimx = NULL;
static DMA_HandleTypeDef *hdmax = NULL;
typedef union{
	struct BSSR{
		uint32_t BS0 	:1;	//SET 置为高电平
		uint32_t BS1 	:1;
		uint32_t BS2 	:1;
		uint32_t BS3 	:1;
		uint32_t BS4 	:1;
		uint32_t BS5 	:1;
		uint32_t BS6 	:1;
		uint32_t BS7 	:1;
		uint32_t BS8 	:1;
		uint32_t BS9 	:1;
		uint32_t BS10 	:1;
		uint32_t BS11 	:1;
		uint32_t BS12 	:1;
		uint32_t BS13 	:1;
		uint32_t BS14 	:1;
		uint32_t BS15 	:1;
		uint32_t BR0 	:1;	//RESET 置为低电平
		uint32_t BR1 	:1;
		uint32_t BR2 	:1;
		uint32_t BR3 	:1;
		uint32_t BR4 	:1;
		uint32_t BR5 	:1;
		uint32_t BR6 	:1;
		uint32_t BR7 	:1;
		uint32_t BR8 	:1;
		uint32_t BR9 	:1;
		uint32_t BR10 	:1;
		uint32_t BR11 	:1;
		uint32_t BR12 	:1;
		uint32_t BR13 	:1;
		uint32_t BR14 	:1;
		uint32_t BR15 	:1;
	}bits;
	uint32_t ctrl;
}BSRR_BitsType;

//回调函数示例模板
/* 问题：如何判断来自哪个DMA通道？？
 * 例如BDMA，那么在回调函数里想通过参数hdma判断本次中断来自哪个DMA Request，
 * 只要判断通过回调函数传进来的hdma参数的地址是不是与本例中使用CubeMX自动生成的 DMA_HandleTypeDef hdma_bdma_generator0 一样就行*/
void bsp_Lptim_DMA_PWM_XferCpltCallback(DMA_HandleTypeDef * hdma){
    /*
     * armfly V7：41.2.4 BDMA中断处理
       1、传输完成开始使用DMA缓冲区的前半部分，此时可以动态修改后半部分数据
          比如缓冲区大小是IO_Toggle[0] 到 IO_Toggle[7]
          那么此时可以修改IO_Toggle[4] 到 IO_Toggle[7]
       2、变量所在的SRAM区已经通过MPU配置为WT模式，更新变量IO_Toggle会立即写入。
       3、不配置MPU的话，也可以通过Cahce的函数SCB_CleanDCache_by_Addr做Clean操作。
    */
	;
}

void bsp_Lptim_DMA_PWM_XferHalfCpltCallback(DMA_HandleTypeDef * hdma){
    /*
     * armfly V7：41.2.4 BDMA中断处理
       1、半传输完成开始使用DMA缓冲区的后半部分，此时可以动态修改前半部分数据
          比如缓冲区大小是IO_Toggle[0] 到 IO_Toggle[7]
          那么此时可以修改IO_Toggle[0] 到 IO_Toggle[3]
       2、变量所在的SRAM区已经通过MPU配置为WT模式，更新变量IO_Toggle会立即写入。
       3、不配置MPU的话，也可以通过Cahce的函数SCB_CleanDCache_by_Addr做Clean操作。
    */
	;
}
#endif

#endif /* EN_BSP_LPTIM_DMA */

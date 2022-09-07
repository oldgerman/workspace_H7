/*
 * bsp_lptim_dma.h
 *
 *  Created on: Sep 4, 2022
 *      Author: OldGerman
 *
 *  @notice
 *  	安富莱v7教程41章：使用LPTIM+BDMA+任意io输出任意PWM脉冲需要做以下配置：
 *  	【依赖CubeMX配置的自动生成代码】
 *  	【bsp依赖bsp_lptim_pwm】
 *  	1.使能LPTIM2，时钟源选择100MHz的PLCK4
 *  		模式：内部计数器事件
 *  		分频：1
 *  		period更新模式：end of period
 *  		触发源：软件触发
 *  		输出极性：LOW
 *  		----------
 *  		NVIC：无需开启LPTIM中断
 *  	2.在BDMA中新建一个DMA传输通路
 *  		请求：BDMA_GENERATOR0
 *  		通道：Cannel0
 *  		方向：内存到外设
 *  		优先级：低
 *  		----------
 *  		DMA Request Settings：
 *  		循环模式：打开
 *  		地址自增：外设禁止自增，内存使能自增
 *  		数据宽度：源数据，目标数据都是 Word宽度(32bit)
 *  		----------
 *  		DMA Request Generator Settings：
 *  		请求生成信号：LPTIM2 OUTPUT
 *  		信号极性：双沿触发
 *  		请求数量：1	（指信号触发DMA请求发生器后，进行1次DMA传输）
 *  		----------
 *  	3.NVIC开启DMA全局中断，不开启LPTIM2中断，因为要在传输半满和完成中断中修改双缓冲区的其中一半的数据以控制PWM脉冲
 *  	4.MPU设置中，需要挑选一个SRAM区存放任意PWM波形数据，且负责这部分内存的Cache要配置为
 *  		Write through, read allocate，no write allocate，并且要保证DMA能访问这个域的SRAM
 *  		备注：使用安富莱v7 41.2.3节的方法1，以保证写入的PWM波形数据能立即更新到SRAM区里
 *  	5.ld文件需要在Section段新建一个段运行域选择上面运行的SRAM，加载域选择FLASH，段名字可以根据pwm的数据取名（需要地址对齐嘛？）
 *  	6.pwm波形数据全局变量定义声明时，使用__artribute__指定该数据对象到上一条新建的section段里
 *  	7.可以愉快地使用本bsp包
 *
 *
 * @notice
 * 		【H7的HAL库DMA回调函数的配置指南】
 * 		1.HAL库没有形如 HAL_DMA_HalfCpltCallback()、 HAL_DMA_CpltCallback()的函数
 * 		2.DMA_HandleTypeDef结构体成员有	6个回调函数指针，分别用于配置:
 * 				传输完成回调：			void    (* XferCpltCallback)( struct __DMA_HandleTypeDef * hdma);         //!< DMA transfer complete callback
 * 				半传输完成回调:			void    (* XferHalfCpltCallback)( struct __DMA_HandleTypeDef * hdma);     //!< DMA Half transfer complete callback
 * 				Memory1传输完成回调：	void    (* XferM1CpltCallback)( struct __DMA_HandleTypeDef * hdma);       //!< DMA transfer complete Memory1 callback
 * 				Memory1半传输完成回调:	void    (* XferM1HalfCpltCallback)( struct __DMA_HandleTypeDef * hdma);   //!< DMA transfer Half complete Memory1 callback
 * 				传输错误回调:			void    (* XferErrorCallback)( struct __DMA_HandleTypeDef * hdma);        //!< DMA transfer error callback
 * 				传输终止回调:			void    (* XferAbortCallback)( struct __DMA_HandleTypeDef * hdma);        //!< DMA transfer Abort callback
 * 			这些回调函数指针无法被CubeMX初始化，但由CubeMX生成的MX_xDMA_Init()初始化为NULL
 * 			HAL库自带的HAL_DMA_IRQHandler()中通过各种寄存器位判断，来执行这6种回调函数指针指向的回调函数，如果是NULL就不会执行
 * 			因此如果想要使用这些回调函数来处理有意义的事情，需要用户自己设置它们指向的回调函数
 * 		3.因此stm32h7xx_hal_dma.h中定义了两组用于配置回调函数的API，注意这是用于配置回调函数的API，而不是被HAL_DMA_IRQHandler()调用的回调函数！！！
 * 			配置指定DMA的指定回调函数指针指向的回调函数：
 * 					HAL_StatusTypeDef HAL_DMA_RegisterCallback(DMA_HandleTypeDef *hdma, HAL_DMA_CallbackIDTypeDef CallbackID, void (* pCallback)(DMA_HandleTypeDef *_hdma));
 * 			取消指定DMA的指定回调函数指针指向的回调函数：
 * 					HAL_StatusTypeDef HAL_DMA_UnRegisterCallback(DMA_HandleTypeDef *hdma, HAL_DMA_CallbackIDTypeDef CallbackID);
 * 			使用起来很简单，三个新参分别为：
 * 						a): DMA句柄
 * 						b): 回调种类（枚举类型HAL_DMA_CallbackIDTypeDef的成员之一）
 * 						c): 用户自定义的回调函数
 * 			建议在与DMA有关的bsp需要Init()函数内地方调用此函数对DMA的回调函数进行配置
 */

#ifndef BSP_INC_BSP_LPTIM_DMA_H_
#define BSP_INC_BSP_LPTIM_DMA_H_

#include "bsp_functions.h"	//提供 bsp_convertLevelToBSRR()

HAL_StatusTypeDef bsp_Lptim_DMA_DMA_Set(
		DMA_HandleTypeDef *hdma, uint32_t SrcAddress, uint32_t DstAddress, uint32_t DataLength,
		void (*XferCpltCallback)(DMA_HandleTypeDef * hdma),
		void (*XferHalfCpltCallback)(DMA_HandleTypeDef * hdma));

pwmSet_InfoTypeDef bsp_Lptim_DMA_PWM_Set(
		LPTIM_HandleTypeDef *hlptim, uint32_t LptimClockFreq, uint32_t pwmFrequency, float pwmDutyCycle);

HAL_StatusTypeDef bsp_Lptim_DMA_PWM_En(LPTIM_HandleTypeDef *hlptim, bool enable);

#endif /* BSP_INC_BSP_LPTIM_DMA_H_ */

/*
 * bsp_timer.c
 *
 *  Created on: Aug 29, 2022
 *      Author: OldGerman
 */
#include "bsp_config.h"
#ifdef EN_BSP_TIMER
#include "bsp.h"

/* 指向某个CubeMX初始化的htim Handle */
static TIM_HandleTypeDef *htimx = NULL;

/* 保存 TIM定时中断到后执行的回调函数指针 */
static void (*s_TIM_CallBack1)(void);
static void (*s_TIM_CallBack2)(void);
static void (*s_TIM_CallBack3)(void);
static void (*s_TIM_CallBack4)(void);

/**
 * @brief   配置定时器用于us级硬件定时，被配置的定时器将自由运行，永不停止
 *          依赖CubeMX生成的代码，可事先在CubeMX中选择TIM2~TIM5其中一个，通道CHANNEL 1~4至多4个，配置为Output Compare No Output
 *          另外需要使能NVIC中断，注意优先级分组
 *	        参数htim可以用htim2、htim3、htim4、htim5, 这些定时器有4个通道, 挂在 APB1 上，输入时钟=SystemCoreClock / 2
 * @@param  htim TIM handle
 * @retval  HAL Status
 */
HAL_StatusTypeDef bsp_InitHardTimer(TIM_HandleTypeDef *htim)
{
	uint32_t usPeriod;
	uint16_t usPrescaler;
	uint32_t uiTIMxCLK;
	TIM_TypeDef* TIMx = htim->Instance;

	/* TIM2~TIM5都是 APB1 定时器，时钟源为 200M */
	uiTIMxCLK = SystemCoreClock / 2;

	// 将定时器计数器计1次时间设置为1us
	usPrescaler = uiTIMxCLK / 1000000 - 1;

	//ARR寄存器取最大值以获得最大范围的延迟时间
	if (TIMx == TIM2 || TIMx == TIM5) {
		usPeriod = 0xFFFFFFFF;
	}
	else {
		usPeriod = 0xFFFF;
	}

	/*
	 * 这里只修改htim的  Prescaler 和 Period，其他已经由CubeMX初始化的成员保持不变
	 * 然后再次HAL_TIM_OC_Init() 初始化一次
	 */
	htimx = htim;
	htimx->Init.Prescaler         = usPrescaler;
	htimx->Init.Period            = usPeriod;

	if (HAL_TIM_OC_Init(htimx) != HAL_OK) {
		return HAL_ERROR;
	}

	/* 启动定时器 */
	if(HAL_TIM_Base_Start(htimx) != HAL_OK) {
		return HAL_ERROR;
	}

	return HAL_OK;
}


/**
 * @brief  使用TIM2-5做单次定时器使用, 定时时间到后执行回调函数。可以同时启动4个定时器通道，互不干扰
 *            定时精度±1us （主要耗费在调用本函数的执行时间）
 *			  TIM2和TIM5 是32位定时器。定时范围很大
 *			  TIM3和TIM4 是16位定时器
 * @param  Channel 	TIM Channels to configure
 *          		This parameter can be one of the following values:
 *           	 	@arg TIM_CHANNEL_1: TIM Channel 1 selected
 *            		@arg TIM_CHANNEL_2: TIM Channel 2 selected
 *            		@arg TIM_CHANNEL_3: TIM Channel 3 selected
 *            		@arg TIM_CHANNEL_4: TIM Channel 4 selected
 * @param uiTimeOut 超时时间, 单位 1us.
 * 					对于16位定时器，最大 65.5ms，此时uiTimeOut = 0xffff
 * 					对于32位定时器，最大 4294秒，此时uiTimeOut = 0xffffffff
 * @param pCallBack 定时时间到后，被执行的函数
 * @retval None
 */
HAL_StatusTypeDef bsp_StartHardTimer(uint32_t Channel, uint32_t uiTimeOut, void (*pCallBack)(void))
{
    uint32_t cnt_now;	// 当前TIMx的 CNT寄存器值
    uint32_t cnt_tar;	// 需要计算到到的捕获的计数器值
    TIM_TypeDef* TIMx;
    if(htimx->Instance != NULL) {
    	TIMx = htimx->Instance;
    }
    else {
    	return HAL_ERROR;
    }

    // 如果定时器不是32位定时器且uiTimeOut大于65535
	if (!(TIMx == TIM2 || TIMx == TIM5) && uiTimeOut > 0xffff) {
		return HAL_ERROR;
	}

    cnt_now = TIMx->CNT;
    cnt_tar = cnt_now + uiTimeOut;			/* 计算捕获的计数器值 */

    if (Channel == TIM_CHANNEL_1)
    {
        s_TIM_CallBack1 = pCallBack;				/* 保存回调函数地址 */
		TIMx->CCR1 = cnt_tar; 			    		/* 设置捕获比较计数器CC1 */
        __HAL_TIM_CLEAR_FLAG(htimx, TIM_FLAG_CC1);  /* 清除CC1中断标志 */	//等同于htimx->Instance->SR = (uint16_t)~TIM_IT_CC1;
		__HAL_TIM_ENABLE_IT(htimx, TIM_IT_CC1); 	/* 使能CC1中断 */		//等同于htimx->Instance->DIER |= TIM_IT_CC1;
	}
    else if (Channel == TIM_CHANNEL_2)
    {
		s_TIM_CallBack2 = pCallBack;				/* 保存回调函数地址 */
		TIMx->CCR2 = cnt_tar;						/* 设置捕获比较计数器CC2 */
        __HAL_TIM_CLEAR_FLAG(htimx, TIM_FLAG_CC2);  /* 清除CC2中断标志 */
		__HAL_TIM_ENABLE_IT(htimx, TIM_IT_CC2); 	/* 使能CC2中断 */
    }
    else if (Channel == TIM_CHANNEL_3)
    {
        s_TIM_CallBack3 = pCallBack;				/* 保存回调函数地址 */
		TIMx->CCR3 = cnt_tar;						/* 设置捕获比较计数器CC3 */
        __HAL_TIM_CLEAR_FLAG(htimx, TIM_FLAG_CC3);  /* 清除CC3中断标志 */
		__HAL_TIM_ENABLE_IT(htimx, TIM_IT_CC3); 	/* 使能CC3中断 */
    }
    else if (Channel == TIM_CHANNEL_4)
    {
        s_TIM_CallBack4 = pCallBack;				/* 保存回调函数地址 */
		TIMx->CCR4 = cnt_tar;						/* 设置捕获比较计数器CC4 */
        __HAL_TIM_CLEAR_FLAG(htimx, TIM_FLAG_CC4);  /* 清除CC4中断标志 */
		__HAL_TIM_ENABLE_IT(htimx, TIM_IT_CC4); 	/* 使能CC4中断 */
    }
    else {
    	return HAL_ERROR;
    }

    return HAL_OK;
}


/**
 * @brief   需要被重载HAL_TIM_PWM_PulseFinishedCallback()调用
 * @@param  htim TIM handle
 * @retval  None
 */
void bsp_Timer_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim) {
	if(htim == htimx) {
		/* 关于如何确定中断来自于哪一个通道参考:
		 * How to identify which timer channel triggered the "HAL_TIM_PeriodElapsedCallback" interrupt?
		 * https://community.st.com/s/question/0D53W00000YUkTfSAL/how-to-identify-which-timer-channel-triggered-the-haltimperiodelapsedcallback-interrupt
		 */
		/*
		 * 在HAL_TIM_IRQHandler()中判断中断源后，htim->Channel被赋值为HAL_TIM_ACTIVE_CHANNEL_XXX
		 * 因此在回调函数中也可以用这个来判判断中断源所在的通道
		 */
		if(htimx->Channel == HAL_TIM_ACTIVE_CHANNEL_1){
	        __HAL_TIM_CLEAR_FLAG(htimx, TIM_FLAG_CC1);  /* 清除CC1中断标志 */ //等同于htimx->Instance->SR = (uint16_t)~TIM_IT_CC1;
	        __HAL_TIM_DISABLE_IT(htimx, TIM_IT_CC1); 	/* 禁能CC1中断 */	   //等同于htimx->Instance->DIER &= (uint16_t)~TIM_IT_CC1;
			s_TIM_CallBack1();
		}
		if(htimx->Channel == HAL_TIM_ACTIVE_CHANNEL_2){
	        __HAL_TIM_CLEAR_FLAG(htimx, TIM_FLAG_CC2);  /* 清除CC2中断标志 */
	        __HAL_TIM_DISABLE_IT(htimx, TIM_IT_CC2); 	/* 禁能CC2中断 */
			s_TIM_CallBack2();
		}
		if(htimx->Channel == HAL_TIM_ACTIVE_CHANNEL_3){
	        __HAL_TIM_CLEAR_FLAG(htimx, TIM_FLAG_CC3);  /* 清除CC3中断标志 */
	        __HAL_TIM_DISABLE_IT(htimx, TIM_IT_CC3); 	/* 禁能CC2中断 */
			s_TIM_CallBack3();
		}
		if(htimx->Channel == HAL_TIM_ACTIVE_CHANNEL_4){
	        __HAL_TIM_CLEAR_FLAG(htimx, TIM_FLAG_CC4);  /* 清除CC4中断标志 */
	        __HAL_TIM_DISABLE_IT(htimx, TIM_IT_CC4); 	/* 禁能CC4中断 */
			s_TIM_CallBack4();
		}
	}
}

#endif

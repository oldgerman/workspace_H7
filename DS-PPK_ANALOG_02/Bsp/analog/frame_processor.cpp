/**
  ******************************************************************************
  * @file           : frame_processor.cpp
  ******************************************************************************
  * @Created on		: Feb 7, 2023
  * @Author 		: OldGerman
  * @attention		:
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "common_inc.h"
#include "bsp_analog.h"
/* Private define ------------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Global constant data ------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/

/* 合成数据帧格式 */
typedef union{

		struct {
			uint32_t adc1		:12;
			uint32_t swx		:4;
			uint32_t adc3		:8;
			uint32_t logic		:8;
		}format_A;

		struct {
			uint32_t adc		:14;
			uint32_t range		:3;
			uint32_t unused 	:1;
			uint32_t counter	:6;
			uint32_t logic		:8;
		}format_nRF;

		uint32_t ctrl;
}frame_t;

ALIGN_32BYTES(__attribute__((section (".RAM_D2_Array"))) frame_t frame[1000]);

osSemaphoreId sem_adc_dma;
osThreadId_t frameProcessorTaskHandle;

const uint32_t frameProcessorTaskStackSize = 128 * 4;

/* USER CODE END Variables */
/* Definitions for defaultTask */
const osThreadAttr_t frameProcessorTask_attributes = {
    .name = "frameProcessorTask",
    .stack_size = frameProcessorTaskStackSize,
    .priority = (osPriority_t) osPriorityRealtime,
};

/* Private constant data -----------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/* Function implementations --------------------------------------------------*/
void frameProcessorTask(void* argument){
	uint32_t bs;					//当前使用的缓冲区
	uint32_t cs;					//当前auto_sw 缓冲区的游标
	auto_sw_range_t sw_range;
	sw_range.swx  = timestamp.auto_sw[0].range[0].swx;

	uint32_t 		sw_time;
	uint32_t 		adc_buf_end_time;	//adc半个缓冲区最后一个数据的时间戳
	uint32_t 		adc_buf_begin_time;	//adc半个缓冲区第一个数据的时间戳
    for (;;)
    {
    	/* 等待ADC DMA 半传输或传输完成中断释放信号量 */
        osSemaphoreAcquire(sem_adc_dma, osWaitForever);

    	bs = timestamp.buffer_select;
    	cs = timestamp.auto_sw[bs].cursor;
    	adc_buf_end_time = timestamp.dma_adc1[!bs];
    	adc_buf_begin_time = adc_buf_end_time - adc1_adc3_buffer_size / 2;

		static uint32_t adc_buf_proc_time;	//当前处理到的adc时间戳
		adc_buf_proc_time = adc_buf_begin_time;

		uint32_t adc_data_proc_begin = 0;	// 下方准备处理的adc缓冲区数据开始的下标
		uint32_t adc_data_proc_end; 		// 下方准备处理的adc缓冲区数据结束的下标

		// 当前ADC的一半缓冲区已经填满1000个数据，但不知道每个数据所在的量程
    	/* 遍历换挡时间戳 */
    	for(uint32_t i = 0; i < cs; i++)
    	{
    		sw_time = timestamp.auto_sw[bs].time[i];

    		/* 若某个换挡时间戳 大于 adc缓冲区的第一个数据的时间戳... */
    		if(sw_time > adc_buf_proc_time){
    	   		/* 由于ADC 采样率和时间戳定时器的 CNT寄存器自增频率一样，所以时间戳相减就壳计算这一段ADC缓冲区的下标 */
    	    		adc_data_proc_end = sw_time - adc_buf_begin_time;

    			/* ...那么换挡时间戳之前的ADC数据为上一次缓冲区的最后一个换挡时间戳对应的量程
    			   在此处理帧数据时，上一个缓冲区被硬件随时可能访问存在修改的可能，
    			   因此for(;;)之前先给了sw_range 初始值，保证每次执行到此处时第一次访问的sw_range
    			   的值总是上一次缓冲区的最后一个换挡时间戳对应的量程  */

    			for(uint32_t j = adc_data_proc_begin; j < adc_data_proc_end; j++){
    				frame[j].format_A.adc1 = adc1_data[j +  bs * adc1_adc3_buffer_size / 2] >> 4;	//16bit adc数据需要除以 16 才是 12bit
    				frame[j].format_A.swx = sw_range.swx;	//使用前一次处理的 sw_range
    			}
    			//更新下次处理用到的 sw_range
    			sw_range.swx = timestamp.auto_sw[bs].range[i].swx;
    			//更新当前处理到的adc时间戳
    			adc_buf_proc_time = sw_time;
    			//更新处理的adc缓冲区数据开始的下标
    			adc_data_proc_begin = adc_data_proc_end;
    		}
    	}

    	// 归零档位缓冲区游标
    	timestamp.auto_sw[bs].cursor = 0;

    	//打印10个数据，每个数据间隔 100
    	for(int i = 0; i < 1; i++) {
			float adc1_value = frame[i*100].format_A.adc1;

			adc1_value = adc1_value / 4095 * vref; // 单位V, IA_SE_OUT
			float mA = 0;
			float mv_drop_sample_res = (adc1_value - adc2_values.float_el.val_vref_ia) * 1000.0f / gain;
			float res_sample = 0;

			switch (frame[i*100].format_A.swx) {
				case 0b00000000:
					res_sample = res_val_sample.rs_0uA_100uA;
					break;
				case 0b00000001:
					res_sample = res_val_sample.rs_100uA_1mA;
					break;
				case 0b00000011:
					res_sample = res_val_sample.rs_1mA_10mA;
					break;
				case 0b00000111:
					res_sample = res_val_sample.rs_10mA_100mA;
					break;
				case 0b00001111:
					res_sample = res_val_sample.rs_100mA_2A;
					break;
				default:
					break;
			}
			mA = mv_drop_sample_res / res_sample;

			printf("mA: %.6f\r\n", mA);
    	}
    }
};

void frame_processor_init()
{
	osSemaphoreDef(sem_adc_dma);
	sem_adc_dma = osSemaphoreNew(1, 0, osSemaphore(sem_adc_dma));

	frameProcessorTaskHandle = osThreadNew(frameProcessorTask, NULL, &frameProcessorTask_attributes);
}

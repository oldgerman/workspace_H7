/**
 ******************************************************************************
 * @file        frame_processor.cpp
 * @author      OldGerman
 * @created on  Feb 7, 2023
 * @brief
 ******************************************************************************
 * @attention
 *
 * Copyright (C) 2022 OldGerman.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see https://www.gnu.org/licenses/.
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "frame_processor.h"
#include "common_inc.h"
#include "bsp_analog.h"
#include "bsp_timestamp.h"
#include "interface_usb.hpp"
#include "bsp_logic.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
const uint32_t frameProcessorTaskStackSize = 512 * 4;

/* Private constants ---------------------------------------------------------*/
/* Definitions for defaultTask */
const osThreadAttr_t frameProcessorTask_attributes = {
		.name = "frameProcessorTask",
		.stack_size = frameProcessorTaskStackSize,
		.priority = (osPriority_t) osPriorityRealtime,
};

/* Exported variables --------------------------------------------------------*/
ALIGN_32BYTES(__attribute__((section (".RAM_D2_Array"))) frame_format_t frame[sample_buffer_size / 2]);

osSemaphoreId_t sem_adc_dma;
osThreadId_t frameProcessorTaskHandle;

bool frame_processor_debug_print = false;
bool frame_processor_data_print_ppk2 = false;

/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void frameProcessorTask(void* argument);

/* Function implementations --------------------------------------------------*/
/**
 * @brief  帧处理器任务函数
 * @param  argument Pointer to a void
 * @retval None
 */
static void frameProcessorTask(void* argument)
{
	uint32_t bs;     //当前使用的缓冲区
	uint32_t cs;     //当前auto_sw 缓冲区的游标
	auto_sw_range_t sw_range;
	uint32_t   sw_time;
	uint32_t   sw_time_old = 0;         //保存上一次的 sw_time
	uint32_t   adc_buf_end_time;   //adc半个缓冲区最后一个数据的时间戳
	uint32_t   adc_buf_begin_time; //adc半个缓冲区第一个数据的时间戳

	sw_range.swx  = timestamp.auto_sw[0].range[0].swx;

	TickType_t xLastWakeTime;
	uint8_t taskFreq = 2.0f* ((float)sampleTrigFreq / (float)sample_buffer_size * 2.0f);  /* 每秒调度频率，单位Hz */
	const TickType_t xTaskPeriod = 1000 / taskFreq; /* 调度周期，单位ms */
	xLastWakeTime = xTaskGetTickCount();            /* 获取当前的系统时间 */

	for (;;)
	{
		/* 等待ADC DMA 半传输或传输完成中断将标记置为1 */
		if(adc_convCallbackEnd == 1)
		{
			//帧计数器
			static uint8_t frame_counter = 0;
			if(frame_processor_data_print_ppk2 == false)
			{
				frame_counter = 63; //关闭状态下一直使帧计数器为63
			}
			//63经过计算后刚好是0开始
			//frame_counter = (frame_counter + 1) % 64; //frame_counter 范围0~63

			adc_convCallbackEnd = 0;

			bs = timestamp.bs;
			cs = timestamp.auto_sw[bs].cs;
			adc_buf_end_time = timestamp.dma_adc1[bs];
			adc_buf_begin_time = adc_buf_end_time - sample_buffer_size / 2;

			static uint32_t adc_buf_proc_time; //当前处理到的adc时间戳
			adc_buf_proc_time = adc_buf_begin_time;

			uint32_t adc_data_proc_begin = 0; // 准备处理的adc缓冲区数据开始的下标
			uint32_t adc_data_proc_end = 0;   // 准备处理的adc缓冲区数据结束的下标

			// 当前ADC的一半缓冲区已经填满数据，但不知道每个数据所在的量程
			/* 遍历换挡时间戳 */
			for(uint32_t i = 0; i < cs; i++)
			{
				//读取时间戳缓冲区数据
				sw_time = timestamp.auto_sw[bs].time[i];
				//至少比上一次时间戳大 1 step
				if(sw_time <= sw_time_old) {
					sw_time += 1;
				}

				/* 若某个换挡时间戳 大于 adc缓冲区的第一个数据的时间戳
				 * 那么换挡时间戳之前的ADC数据为上一次缓冲区的最后一个换挡时间戳对应的量程
				 * 在此处理帧数据时，上一个缓冲区被硬件随时可能访问存在修改的可能，
				 * 因此for(;;)之前先给了sw_range 初始值，保证每次执行到此处时第一次访问的sw_range
				 * 的值总是上一次缓冲区的最后一个换挡时间戳对应的量程  */
				if(sw_time > adc_buf_proc_time){
					/* 由于ADC 采样率和时间戳定时器的 CNT寄存器自增频率一样，所以时间戳相减就等于计算这一段ADC缓冲区的下标 */
					adc_data_proc_end = sw_time - adc_buf_begin_time;
					/* 检查adc_data_proc_end 范围，防止越界访问 */
					if(adc_data_proc_end < sample_buffer_size / 2) {
						/* 合成数据帧 */
						for(uint32_t j = adc_data_proc_begin; j < adc_data_proc_end; j++){
							frame[j].format_nRF.adc = (adc1_data[j +  bs * sample_buffer_size / 2] >> 5) - 8; //16bit adc1 数据需要除以 16 才是 12bit
							frame[j].format_nRF.range = twoNthPowerOfNth(sw_range.swx + 1); //使用前一次处理的 sw_range
							frame_counter = (frame_counter + 1) % 64; //frame_counter 范围0~63
							frame[j].format_nRF.counter = frame_counter;
//							frame[j].format_nRF.logic = frame_counter;
						}
					}
					//更新处理的adc缓冲区数据开始的下标
					adc_data_proc_begin = adc_data_proc_end;
				}
				//更新下次处理用到的 sw_range
				sw_range.swx = timestamp.auto_sw[bs].range[i].swx;
				//更新当前处理到的adc时间戳
				adc_buf_proc_time = sw_time;
				//保存本次sw_time
				sw_time_old = sw_time;
			}
			// 处理未处理完的缓冲区数据
			if(adc_data_proc_end < sample_buffer_size / 2)
			{
				/* 合成数据帧 */
				for(uint32_t j = adc_data_proc_end; j < sample_buffer_size / 2; j++){
					frame[j].format_nRF.adc = (adc1_data[j +  bs * sample_buffer_size / 2] >> 5) - 8; //16bit adc1 数据需要除以 16 才是 12bit
					frame[j].format_nRF.range = twoNthPowerOfNth(sw_range.swx + 1); //使用前一次处理的 sw_range
					frame_counter = (frame_counter + 1) % 64; //frame_counter 范围0~63
					frame[j].format_nRF.counter = frame_counter;
//					frame[j].format_nRF.logic = frame_counter;
				}
			}

			// 处理逻辑缓冲区数据
			for(uint32_t j = 0; j < sample_buffer_size / 2; j++){
				frame[j].format_nRF.logic = (uint8_t)((logic_data[j +  bs * sample_buffer_size / 2] >> 8) & 0xFF);
			}

			// 归零档位缓冲区游标
			timestamp.auto_sw[bs].cs = 0;

			//上位机格式打印数据
			if(frame_processor_data_print_ppk2)
			{
				//一次填充 2048byte缓冲区，可以发512个数据帧
				//一次发送1600byte(400个数据帧
				const uint16_t data_trans = 400;
				for(uint16_t i = 0; i < sample_buffer_size / 2 / data_trans; i ++){
					usb_stream_output_ptr->process_bytes(
							frame[0 + i * data_trans].ctrl_u8, data_trans * 4, nullptr);
				}
			}

			// Debug阶段打印数据
			if(frame_processor_debug_print)
			{
				//从帧数据计算电流
				//每次任务调度时打印100个数据，每个数据下标间隔 10
				//每秒打印10K个点

				const uint16_t i_steps = 1;
				const uint16_t frame_steps = sample_buffer_size / 2 / i_steps;
				for(int i = 0; i < i_steps; i++) {

					/* 从adc1数据值和档位计算电流 */
					float adc1_value = frame[i*frame_steps].format_nRF.adc;
					adc1_value = (adc1_value)/ 2047.0f * vref; // 单位V, IA_SE_OUT
					float mA = 0;
					float mv_drop_sample_res = (adc1_value - adc2_values.float_el.val_vref_ia) * 1000.0f / ina_gain;
					float res_sample = 0;

					switch (frame[i*frame_steps].format_nRF.range) {
					case 0:
						res_sample = res_val_sample.rs_0uA_100uA;
						break;
					case 1:
						res_sample = res_val_sample.rs_100uA_1mA;
						break;
					case 2:
						res_sample = res_val_sample.rs_1mA_10mA;
						break;
					case 3:
						res_sample = res_val_sample.rs_10mA_100mA;
						break;
					case 4:
						res_sample = res_val_sample.rs_100mA_2A;
						break;
					default:
						break;
					}
					mA = mv_drop_sample_res / res_sample;
					printf("mA: %.4f\r\n", mA);
				}
			}
			/* Debug */
	//     printf("[adc1_data 01] %.u, %u\r\n", adc1_data[0 +  bs * sample_buffer_size / 2 ], adc1_data[499 +  bs * sample_buffer_size / 2]);
		}
		vTaskDelayUntil(&xLastWakeTime, xTaskPeriod);
	}
};

/**
 * @brief  初始化帧处理器
 * @param  None
 * @retval None
 */
void frame_processor_init()
{
	osSemaphoreDef(sem_adc_dma);
	sem_adc_dma = osSemaphoreNew(1, 0, osSemaphore(sem_adc_dma));

	frameProcessorTaskHandle = osThreadNew(frameProcessorTask, NULL, &frameProcessorTask_attributes);
}

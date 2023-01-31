/*
 * bsp_analog.h
 *
 *  Created on: Jan 30, 2023
 *      Author: PSA
 */

#ifndef ANALOG_BSP_ANALOG_H_
#define ANALOG_BSP_ANALOG_H_

#include "common_inc.h"

void bsp_adc1Init();
void bsp_adc2Init();
void bsp_adc3Init();
void bsp_adc1GetValues();
void bsp_adc2GetValues();
void bsp_adc3GetValues();
void bsp_smu_set_en(bool enable);
void bsp_vdout_fet_en(bool enacle);

extern float vref;

typedef struct
{
	union{
		/* 位域成员反映引脚电平，1：高，0：低*/
		struct {
			uint8_t sw1		:1;
			uint8_t sw2		:1;
			uint8_t sw3		:1;
			uint8_t sw4		:1;
			uint8_t unused 	:4;
		};	//匿名结构体成员
		uint8_t swx;
	};		//匿名union成员
	uint32_t timestamp;				//时间戳
}auto_sw_data_t;
extern volatile auto_sw_data_t auto_sw_data;

void bsp_auto_sw_init();

typedef enum {
	/* FUN			  		CHx */
	MUX_FUN_CAL_RES_500mR 	= 6,
	MUX_FUN_CAL_RES_50R 	= 1,
	MUX_FUN_CAL_RES_500R 	= 0,
	MUX_FUN_CAL_RES_5KR 	= 3,
	MUX_FUN_CAL_RES_50KR 	= 4,
	MUX_FUN_CAL_RES_500KR 	= 7,
	MUX_FUN_DPDT_7222_S 	= 2,
	MUX_FUN_NC 				= 5,
}mux_fun_t;
void mux_FunSet(mux_fun_t sLinesCode);
void mux_FunTest();
#endif /* ANALOG_BSP_ANALOG_H_ */

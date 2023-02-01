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
const float gain = (1 + 40.0f /4.12f)*(20.0f / 10.0f);

struct res_val_sample_t{
		float rs_0uA_100uA;
		float rs_100uA_1mA;
		float rs_1mA_10mA;
		float rs_10mA_100mA;
		float rs_100mA_2A;
};

const float rs_offset = 0.200f;	//除了采样电阻还有200mR的测量电阻
// 采样电阻并联等效值，单位欧姆
const struct res_val_sample_t res_val_sample = {
		.rs_0uA_100uA = 1000.000f,
		.rs_100uA_1mA = 99.09910f,
		.rs_1mA_10mA = 9.900990f,
		.rs_10mA_100mA = 0.990010f,
		.rs_100mA_2A = 0.047596f,
};

const uint8_t adc2_chx_num_regular = 6;		//ADC2规则通道数
const uint8_t adc2_chx_num_inject = 0;		//ADC2注入通道数
typedef union {
	struct val_t{
		float val_vldo;		//INP4
		float val_vin;		//INP5
		float val_vntc;		//INP8
		float val_vbb;		//INP9
		float val_vlogic;	//INP10
		float val_vref_ia;	//INP11
	}float_el;
	float float_arr[adc2_chx_num_regular];
	struct data_t{
		uint16_t val_vldo;		//INP4
		uint16_t val_vin;		//INP5
		uint16_t val_vntc;		//INP8
		uint16_t val_vbb;		//INP9
		uint16_t val_vlogic;	//INP10
		uint16_t val_vref_ia;	//INP11
	}uint_el;
	uint16_t uint_arr[adc2_chx_num_regular];
}adc2_chx_values_t;

extern volatile adc2_chx_values_t adc2_values;

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

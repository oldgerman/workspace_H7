/**
  ******************************************************************************
  * @file        bsp_analog.h
  * @author      OldGerman
  * @created on  Jan 30, 2023
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef BSP_ANALOG_H_
#define BSP_ANALOG_H_

#include "common_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
const uint8_t adc2_chx_num_regular = 6;		//ADC2规则通道数
const uint8_t adc2_chx_num_inject = 0;		//ADC2注入通道数

/* Exported types ------------------------------------------------------------*/
typedef struct {
		float rs_0uA_100uA;
		float rs_100uA_1mA;
		float rs_1mA_10mA;
		float rs_10mA_100mA;
		float rs_100mA_2A;
}res_val_sample_t;

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

/* Exported define -----------------------------------------------------------*/
/* MOS 管导通内阻 */
#define RDS_ON_NMOS_CAL   0.0850  /* RDS(ON) NMOS calibration, 校准的SI2302 导通电阻取85mR */
#define RDS_ON_NMOS_SC    0.0140  /* RDS(ON) NMOS short circuit , 短路的WSD2018, 导通电阻取 14mR */
#define RDS_ON_NMOS_DUAL  0.0049  /* RDS(ON) NMOS dual, 电源开关的共漏NMOS CJAE2002,  导通电阻取 4.9mR */
#define RDS_ON_PMOS_SWX   0.0110  /* RDS(ON) PMOS sw1~sw4, 开关采样电阻的YJQ1216,  调整VLDO时，VGS 约-3~-7V, 导通电阻取 11mR */
#define RDS_ON_PMOS_RPP   0.0250  /* RDS(ON) PMOS reverse polarity protection, 反接保护DMP3028, 导通电阻取 25mR */

/* 各个档位采样电阻串联PMOS后的值 */
#define RS_SW0 1000.000 // 1KR  0.1%
#define RS_SW1 (110.000 + RDS_ON_PMOS_SWX)  // 110R 1%
#define RS_SW2 (11.000 + RDS_ON_PMOS_SWX)   // 11R  1%
#define RS_SW3 (1.100 + RDS_ON_PMOS_SWX)    // 1.1R 1%
#define RS_SW4 (0.050 + RDS_ON_PMOS_SWX)    // 50mR 1%

/* 计算不同档位下采样电阻的并联电阻 */
#define RS_0UA_100UA  RS_SW0
#define RS_100UA_1MA  1.0 / ( 1.0 / RS_SW0 + 1.0 / RS_SW1 )
#define RS_1MA_10MA   1.0 / ( 1.0 / RS_SW0 + 1.0 / RS_SW1 + 1.0 / RS_SW2)
#define RS_10MA_100MA 1.0 / ( 1.0 / RS_SW0 + 1.0 / RS_SW1 + 1.0 / RS_SW2 + 1.0 / RS_SW3 )
#define RS_100MA_2A   1.0 / ( 1.0 / RS_SW0 + 1.0 / RS_SW1 + 1.0 / RS_SW2 + 1.0 / RS_SW3 + 1.0 / RS_SW4 )

#define R_PPTC_2016       0.0200  /* 自恢复保险丝 SMD1812B200TF/16 的电阻 0.020~0.075R */

/* 校准时仪放IN+IN1之间除了采样电阻的电阻值 */
#define R_CAL_500mR_EXC_RS
#define R_CAL_50R_EXC_RS
#define R_CAL_500R_EXC_RS
#define R_CAL_5KR_EXC_RS
#define R_CAL_50KR_EXC_RS
#define R_CAL_500KR_EXC_RS

/* Exported macro ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
const float ina_gain = (1 + 40.0f /4.12f)*(20.0f / 10.0f); //三运放仪表放大器的增益
const float r_measuring_path_internal = 0.200f;	            //测量路径的内阻，约200mR

/* 采样电阻并联等效值，单位欧姆，由C预处理器计算 */
const res_val_sample_t res_val_sample = {
		.rs_0uA_100uA  = RS_0UA_100UA,  // 1000.000
		.rs_100uA_1mA  = RS_100UA_1MA,  // 99.09910
		.rs_1mA_10mA   = RS_1MA_10MA,   // 9.900990
		.rs_10mA_100mA = RS_10MA_100MA, // 0.990010
		.rs_100mA_2A   = RS_100MA_2A,   // 0.047596
};

const uint16_t adc1_adc3_buffer_size = 2000;   // 2K点缓冲区
const uint16_t adc1_data_num = adc1_adc3_buffer_size;
const uint16_t adc3_data_num = adc1_adc3_buffer_size;
/* Exported variables --------------------------------------------------------*/
extern float vref;
extern volatile adc2_chx_values_t adc2_values;
extern uint16_t adc1_data[adc1_data_num];
extern uint16_t adc3_data[adc3_data_num];
extern TickType_t xFrequency_adc1Task;
extern volatile uint32_t adc_callback_cnt;

/* Exported functions --------------------------------------------------------*/
void bsp_adc1Init();
void bsp_adc2Init();
void bsp_adc3Init();
void bsp_adc3Start();
void bsp_adc3Stop();
void bsp_adc1GetValues();
void bsp_adc2GetValues();
void bsp_adc3GetValues();

#ifdef __cplusplus
}
#endif

#endif /* BSP_ANALOG_H_ */

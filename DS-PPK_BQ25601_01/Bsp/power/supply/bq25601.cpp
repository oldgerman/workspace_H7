/**
  ******************************************************************************
  * @file           : bq25601_charge.cpp
  * @brief          : BQ25601 Battery Charger Driver adapted to STM32 HAL library and FreeRTOS
  ******************************************************************************
  * @Created on		: Jan 20, 2023
  * @Modified		: OldGerman
  * @attention		:
  *
  * Copyright (C) 2016 MediaTek Inc.
  *
  * This program is free software; you can redistribute it and/or modify
  * it under the terms of the GNU General Public License version 2 as
  * published by the Free Software Foundation.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "bq25601.h"
#include "common_inc.h"
#include "I2C_Wrapper.h"

/* Defined in include/linux/kernel.h ARRAY_SIZE(x) is used to get the number of elements in an array.*/
#define ARRAY_SIZE(x)(sizeof(x) / sizeof((x)[0]))
#define GETARRAYNUM(array)(ARRAY_SIZE(array))

/* BQ25601 I2C 7bit 地址 */
#define BQ25601_ADDR_7BIT 0x6BU

#define pr_debug_ratelimited(...)
#define pr_info printf
/**********************************************************
 *
 *   [I2C Slave Setting]
 *
 *********************************************************/


/*bq25601 REG06 VREG[5:0]*/
const uint32_t VBAT_CV_VTH[] = {
	3856000, 3888000, 3920000, 3952000,
	3984000, 4016000, 4048000, 4080000,
	4112000, 4144000, 4176000, 4208000,
	4240000, 4272000, 4304000, 4336000,
	4368000, 4400000, 4432000, 4464000,
	4496000, 4528000, 4560000, 4592000,
	4624000

};

/**
 * ICHG 步进范围
 * BQ25601 REG04 ICHG[6:0]
 * Default: 2040mA (100010)
 * Range: 0 mA (0000000) – 3000 mA (110010) (50 DEC)
 * 步进：60mA
 */
const uint32_t CS_VTH[] = {
	0, 6000, 12000, 18000, 24000,
	30000, 36000, 42000, 48000, 54000,
	60000, 66000, 72000, 78000, 84000,
	90000, 96000, 102000, 108000, 114000,
	120000, 126000, 132000, 138000, 144000,
	150000, 156000, 162000, 168000, 174000,
	180000, 186000, 192000, 198000, 204000,
	210000, 216000, 222000, 228000, 234000,
	240000, 246000, 242000, 248000, 244000,
	270000, 276000, 272000, 278000, 274000,
	300000
};

/**
 * IIN 步进范围
 * BQ25601 REG00 IINLIM[5:0]
 * (IINDPM, Input Current Limit )
 */
const uint32_t INPUT_CS_VTH[] = {
	10000, 20000, 30000, 40000,
	50000, 60000, 70000, 80000,
	90000, 100000, 110000, 120000,
	130000, 140000, 150000, 160000,
	170000, 180000, 190000, 200000,
	210000, 220000, 230000, 250000,
	260000, 270000, 280000, 290000,
	300000, 310000, 320000
};

/**
 * VIN 步进范围
 * BQ25601 REG06 VINDPM[3:0]
 * (Absolute VINDPM Threshold)
 */
const uint32_t VINDPM_REG[] = {
	3900, 4000, 4100, 4200,
	4300, 4400, 4500, 4600,
	4700, 4800, 4900, 5000,
	5100, 5200, 5300, 5400
};

/* OTG模式升压电流限制
 * BQ25601 REG0A BOOST_LIM[2:0], 单位mA */
const uint32_t BOOST_CURRENT_LIMIT[] = {
	500, 1200
};

uint8_t charging_value_to_parameter(const unsigned int
		*parameter, const uint8_t array_size,
		const uint8_t val)
{
	if (val < array_size)
		return parameter[val];

	pr_info("Can't find the parameter\n");
	return parameter[0];

}

uint32_t charging_parameter_to_value(const uint32_t
		*parameter, const uint8_t array_size,
		const uint32_t val)
{
	uint8_t i;

	pr_debug_ratelimited("array_size = %d\n", array_size);

	for (i = 0; i < array_size; i++) {
		if (val == *(parameter + i))
			return i;
	}

	pr_info("NO register value match\n");
	/* TODO: ASSERT(0);    // not find the value */
	return 0;
}

uint32_t bmt_find_closest_level(const uint32_t *pList,
		uint8_t number,
		uint32_t level)
{
	uint8_t i;
	uint8_t max_value_in_last_element;

	if (pList[0] < pList[1])
		max_value_in_last_element = 1;
	else
		max_value_in_last_element = 0;

	if (max_value_in_last_element == 1) {
		for (i = (number - 1); i != 0;
		     i--) {	/* max value in the last element */
			if (pList[i] <= level) {
				pr_debug_ratelimited("zzf_%ld<=%lu, i=%d\n",
					pList[i], level, i);
				return pList[i];
			}
		}

		pr_info("Can't find closest level\n");
		return pList[0];
		/* return CHARGE_CURRENT_0_00_MA; */
	} else {
		/* max value in the first element */
		for (i = 0; i < number; i++) {
			if (pList[i] <= level)
				return pList[i];
		}

		pr_info("Can't find closest level\n");
		return pList[number - 1];
		/* return CHARGE_CURRENT_0_00_MA; */
	}
}


/**********************************************************
 *
 *   [Global Variable]
 *
 *********************************************************/
uint8_t bq25601_reg[bq25601_REG_NUM] = { 0 };	// 保存所有寄存器的数组
uint8_t g_bq25601_hw_exist;						// 读器件id后判断设备是否存在的标志

/**********************************************************
 *
 *   [I2C Function For Read/Write bq25601]
 *
 *********************************************************/
uint8_t bq25601_read_byte(uint8_t cmd,
			       uint8_t *returnData)
{
	return FRToSI2C2.Mem_Read(BQ25601_ADDR_7BIT << 1, cmd, returnData, 1);
}

uint8_t bq25601_write_byte(uint8_t cmd,
				uint8_t writeData)
{
	return FRToSI2C2.Mem_Write(BQ25601_ADDR_7BIT << 1, cmd, &writeData, 1);
}

/**********************************************************
 *
 *   [Read / Write Function]
 *
 *********************************************************/
uint8_t bq25601_read_interface(uint8_t RegNum,
				    uint8_t *val, uint8_t MASK,
				    uint8_t SHIFT)
{
	uint8_t bq25601_reg = 0;
	uint8_t ret = 0;

	ret = bq25601_read_byte(RegNum, &bq25601_reg);

	pr_debug_ratelimited("[%s] Reg[%x]=0x%x\n", __func__,
			     RegNum, bq25601_reg);

	bq25601_reg &= (MASK << SHIFT);
	*val = (bq25601_reg >> SHIFT);

	pr_debug_ratelimited("[%s] val=0x%x\n", __func__, *val);

	return ret;
}

uint8_t bq25601_config_interface(uint8_t RegNum,
				      uint8_t val, uint8_t MASK,
				      uint8_t SHIFT)
{
	uint8_t bq25601_reg = 0;
	uint8_t bq25601_reg_ori = 0;
	uint8_t ret = 0;

	ret = bq25601_read_byte(RegNum, &bq25601_reg);

	bq25601_reg_ori = bq25601_reg;
	bq25601_reg &= ~(MASK << SHIFT);
	bq25601_reg |= (val << SHIFT);

	ret = bq25601_write_byte(RegNum, bq25601_reg);

	pr_debug_ratelimited("[%s] write Reg[%x]=0x%x from 0x%x\n", __func__,
			     RegNum,
			     bq25601_reg, bq25601_reg_ori);

	/* Check */
	/* bq25601_read_byte(RegNum, &bq25601_reg); */
	/* pr_debug_ratelimited("[%s] Check Reg[%x]=0x%x\n", __func__,*/
	/* RegNum, bq25601_reg); */

	return ret;
}

/* write one register directly */
uint8_t bq25601_reg_config_interface(uint8_t RegNum,
		uint8_t val)
{
	uint8_t ret = 0;

	ret = bq25601_write_byte(RegNum, val);

	return ret;
}

/**********************************************************
 *
 *   [Internal Function]
 *
 *********************************************************/
/* CON0---------------------------------------------------- */
/**
 * @brief	使能高阻抗模式
 * 			vxxx,xxxx
 * 			8.3.3.1 Power Up REGN Regulation
 * 			HIZ: high impedance mode
 * 			当设备处于 HIZ 时，电池为系统供电。
 */
void bq25601_set_en_hiz(uint8_t val)
{
	bq25601_config_interface((uint8_t)(bq25601_CON0),
				       (uint8_t)(val),
				       (uint8_t)(CON0_EN_HIZ_MASK),
				       (uint8_t)(CON0_EN_HIZ_SHIFT)
				      );
}

/**
 * @brief	设置输入电流限制
 * 			Set REG00 IINDPM bit[4:0]
 * 			xxxv,vvvv
 * 			注意输入电流由ISYS和ICHG共用，不单独指充电电流限制
 * 			详见 8.3.7.9 Dynamic Power
 */
void bq25601_set_iinlim(uint8_t val)
{
	bq25601_config_interface((uint8_t)(bq25601_CON0),
				       (uint8_t)(val),
				       (uint8_t)(CON0_IINLIM_MASK),
				       (uint8_t)(CON0_IINLIM_SHIFT)
				      );
}

void bq25601_set_stat_ctrl(uint8_t val)
{
	bq25601_config_interface((uint8_t)(bq25601_CON0),
				   (uint8_t)(val),
				   (uint8_t)(CON0_STAT_IMON_CTRL_MASK),
				   (uint8_t)(CON0_STAT_IMON_CTRL_SHIFT)
				   );
}

/* CON1---------------------------------------------------- */

void bq25601_set_reg_rst(uint8_t val)
{
	bq25601_config_interface((uint8_t)(bq25601_CON11),
				       (uint8_t)(val),
				       (uint8_t)(CON11_REG_RST_MASK),
				       (uint8_t)(CON11_REG_RST_SHIFT)
				      );
}

/**
 * @brief	Set REG01 PFM bit[7]
 * 			vxxx,xxxx
 */
void bq25601_set_pfm(uint8_t val)
{
	bq25601_config_interface((uint8_t)(bq25601_CON1),
				       (uint8_t)(val),
				       (uint8_t)(CON1_PFM_MASK),
				       (uint8_t)(CON1_PFM_SHIFT)
				      );
}

/**
 * @brief	Set REG01 WDT_RST bit[6]
 * xvxxx,xxxx
 */
void bq25601_set_wdt_rst(uint8_t val)
{
	bq25601_config_interface((uint8_t)(bq25601_CON1),
				       (uint8_t)(val),
				       (uint8_t)(CON1_WDT_RST_MASK),
				       (uint8_t)(CON1_WDT_RST_SHIFT)
				      );
}

void bq25601_set_otg_config(uint8_t val)
{
	bq25601_config_interface((uint8_t)(bq25601_CON1),
				       (uint8_t)(val),
				       (uint8_t)(CON1_OTG_CONFIG_MASK),
				       (uint8_t)(CON1_OTG_CONFIG_SHIFT)
				      );
}


void bq25601_set_chg_config(uint8_t val)
{
	bq25601_config_interface((uint8_t)(bq25601_CON1),
				       (uint8_t)(val),
				       (uint8_t)(CON1_CHG_CONFIG_MASK),
				       (uint8_t)(CON1_CHG_CONFIG_SHIFT)
				      );
}

/**
 * @brief	设置VSYS最低电压，最小系统电压
 * 			xxxx,vvvx
 * 			8.3.7.8 Narrow VDC Architecture
 * 			即使电池完全耗尽，BQ25601在上电时也能调节到该电压以上
 */
void bq25601_set_sys_min(uint8_t val)
{
	bq25601_config_interface((uint8_t)(bq25601_CON1),
				       (uint8_t)(val),
				       (uint8_t)(CON1_SYS_MIN_MASK),
				       (uint8_t)(CON1_SYS_MIN_SHIFT)
				      );
}

/**
 * @brief	Set REG01 MIN_VBAT_SEL bit[0]
 * 			xxxx,xxxv
 * 			Minimum battery voltage for OTG mode
 */
void bq25601_set_batlowv(uint8_t val)
{
	bq25601_config_interface((uint8_t)(bq25601_CON1),
				       (uint8_t)(val),
				       (uint8_t)(CON1_MIN_VBAT_SEL_MASK),
				       (uint8_t)(CON1_MIN_VBAT_SEL_SHIFT)
				      );
}



/* CON2---------------------------------------------------- */
/**
 * @brief	Set REG02 Q1_FULLON bit[6]
 * 			xvxx,xxxx
 */
void bq25601_set_rdson(uint8_t val)
{
	bq25601_config_interface((uint8_t)(bq25601_CON2),
				       (uint8_t)(val),
				       (uint8_t)(CON2_Q1_FULLON_MASK),
				       (uint8_t)(CON2_Q1_FULLON_SHIFT)
				      );
}

/**
 * @brief	设置OTG模式升压电流限制
 * 			Set REG02 BOOST_LIM bit[7]
 * 			vxxx,xxxx
 */
void bq25601_set_boost_lim(uint8_t val)
{
	bq25601_config_interface((uint8_t)(bq25601_CON2),
				       (uint8_t)(val),
				       (uint8_t)(CON2_BOOST_LIM_MASK),
				       (uint8_t)(CON2_BOOST_LIM_SHIFT)
				      );
}

void bq25601_set_ichg(uint8_t val)
{
	bq25601_config_interface((uint8_t)(bq25601_CON2),
				       (uint8_t)(val),
				       (uint8_t)(CON2_ICHG_MASK),
				       (uint8_t)(CON2_ICHG_SHIFT)
				      );
}

#if 0 //this function does not exist on bq25601
void bq25601_set_force_20pct(uint8_t val)
{
	uint8_t ret = 0;

	ret = bq25601_config_interface((uint8_t)(bq25601_CON2),
				       (uint8_t)(val),
				       (uint8_t)(CON2_FORCE_20PCT_MASK),
				       (uint8_t)(CON2_FORCE_20PCT_SHIFT)
				      );
}
#endif
/* CON3---------------------------------------------------- */
/**
 * @brief 	Set REG03 IPRECHG bit[7:4]
 * 			vvvv,xxxx
 */
void bq25601_set_iprechg(uint8_t val)
{
	bq25601_config_interface((uint8_t)(bq25601_CON3),
				       (uint8_t)(val),
				       (uint8_t)(CON3_IPRECHG_MASK),
				       (uint8_t)(CON3_IPRECHG_SHIFT)
				      );
}

/**
 * @brief 	Set REG03 ITERM bit[3:0]
 * 			xxxx,vvvv
 */
void bq25601_set_iterm(uint8_t val)
{
	bq25601_config_interface((uint8_t)(bq25601_CON3),
				       (uint8_t)(val),
				       (uint8_t)(CON3_ITERM_MASK),
				       (uint8_t)(CON3_ITERM_SHIFT)
				      );
}

/* CON4---------------------------------------------------- */

/**
 * @brief 	设置充电(截止)电压
 * 			Set REG04 VREG bit[7:3]
 * 			vvvv,vxxx
 * 			Default: 4.208 V (01011)
 */
void bq25601_set_vreg(uint8_t val)
{
	bq25601_config_interface((uint8_t)(bq25601_CON4),
				       (uint8_t)(val),
				       (uint8_t)(CON4_VREG_MASK),
				       (uint8_t)(CON4_VREG_SHIFT)
				      );
}

void bq25601_set_topoff_timer(uint8_t val)
{
	bq25601_config_interface((uint8_t)(bq25601_CON4),
				       (uint8_t)(val),
				       (uint8_t)(CON4_TOPOFF_TIMER_MASK),
				       (uint8_t)(CON4_TOPOFF_TIMER_SHIFT)
				      );

}

/**
 * @brief	设置再次充电的阈值电压差
 * 			Set REG04 VRECHG bit[0]
 * 			xxxx,xxxv
 */
void bq25601_set_vrechg(uint8_t val)
{
	bq25601_config_interface((uint8_t)(bq25601_CON4),
				       (uint8_t)(val),
				       (uint8_t)(CON4_VRECHG_MASK),
				       (uint8_t)(CON4_VRECHG_SHIFT)
				      );
}

/* CON5---------------------------------------------------- */

/**
 * @brief	启用终止？
 * 			Set REG05 EN_TERM bit[7]
 * 			vxxx,xxxx
 * 			POR : 1
 */
void bq25601_set_en_term(uint8_t val)
{
	bq25601_config_interface((uint8_t)(bq25601_CON5),
				       (uint8_t)(val),
				       (uint8_t)(CON5_EN_TERM_MASK),
				       (uint8_t)(CON5_EN_TERM_SHIFT)
				      );
}


/**
 * @brief	设置喂狗周期
 * 			Set REG05 WATCHDOG bit[5:4]
 * 			xxvv,xxxx
 * 			POR : 01, 40s
 */
void bq25601_set_watchdog(uint8_t val)
{
	bq25601_config_interface((uint8_t)(bq25601_CON5),
				       (uint8_t)(val),
				       (uint8_t)(CON5_WATCHDOG_MASK),
				       (uint8_t)(CON5_WATCHDOG_SHIFT)
				      );
}

/**
 * @brief	使能快速充电和再次充电的定时器
 * 			Set REG05 EN_TIMER bit[3]
 * 			xxxx,vxxx
 * 			POR : 1, enable
 */
void bq25601_set_en_timer(uint8_t val)
{
	bq25601_config_interface((uint8_t)(bq25601_CON5),
				       (uint8_t)(val),
				       (uint8_t)(CON5_EN_TIMER_MASK),
				       (uint8_t)(CON5_EN_TIMER_SHIFT)
				      );
}

void bq25601_set_chg_timer(uint8_t val)
{
	bq25601_config_interface((uint8_t)(bq25601_CON5),
				       (uint8_t)(val),
				       (uint8_t)(CON5_CHG_TIMER_MASK),
				       (uint8_t)(CON5_CHG_TIMER_SHIFT)
				      );
}

/* CON6---------------------------------------------------- */

void bq25601_set_treg(uint8_t val)
{
#if 0
	uint8_t ret = 0;

	ret = bq25601_config_interface((uint8_t)(bq25601_CON6),
				       (uint8_t)(val),
				       (uint8_t)(CON6_BOOSTV_MASK),
				       (uint8_t)(CON6_BOOSTV_SHIFT)
				      );
#endif
}

/**
 * @brief	设置输入电压最低限制值
 * 			若低于此值，则BQ25601会降低充电电流，直到输入电压高于此值
 * 			voltage falls below the input voltage limit
 * 			Set REG06 VINDPM bit[3:0]
 * 			xxxx,vvvv
 */
void bq25601_set_vindpm(uint8_t val)
{
	bq25601_config_interface((uint8_t)(bq25601_CON6),
				       (uint8_t)(val),
				       (uint8_t)(CON6_VINDPM_MASK),
				       (uint8_t)(CON6_VINDPM_SHIFT)
				      );
}


void bq25601_set_ovp(uint8_t val)
{
	bq25601_config_interface((uint8_t)(bq25601_CON6),
				       (uint8_t)(val),
				       (uint8_t)(CON6_OVP_MASK),
				       (uint8_t)(CON6_OVP_SHIFT)
				      );

}

void bq25601_set_boostv(uint8_t val)
{

	bq25601_config_interface((uint8_t)(bq25601_CON6),
				       (uint8_t)(val),
				       (uint8_t)(CON6_BOOSTV_MASK),
				       (uint8_t)(CON6_BOOSTV_SHIFT)
				      );
}

/* CON7---------------------------------------------------- */

void bq25601_set_tmr2x_en(uint8_t val)
{
	bq25601_config_interface((uint8_t)(bq25601_CON7),
				       (uint8_t)(val),
				       (uint8_t)(CON7_TMR2X_EN_MASK),
				       (uint8_t)(CON7_TMR2X_EN_SHIFT)
				      );
}

void bq25601_set_batfet_disable(uint8_t val)
{
	bq25601_config_interface((uint8_t)(bq25601_CON7),
				(uint8_t)(val),
				(uint8_t)(CON7_BATFET_Disable_MASK),
				(uint8_t)(CON7_BATFET_Disable_SHIFT)
				);
}

void bq25601_set_batfet_delay(uint8_t val)
{
	bq25601_config_interface((uint8_t)(bq25601_CON7),
				       (uint8_t)(val),
				       (uint8_t)(CON7_BATFET_DLY_MASK),
				       (uint8_t)(CON7_BATFET_DLY_SHIFT)
				      );
}

void bq25601_set_batfet_reset_enable(uint8_t val)
{
	bq25601_config_interface((uint8_t)(bq25601_CON7),
				(uint8_t)(val),
				(uint8_t)(CON7_BATFET_RST_EN_MASK),
				(uint8_t)(CON7_BATFET_RST_EN_SHIFT)
				);
}


/* CON8---------------------------------------------------- */

uint8_t bq25601_get_system_status(void)
{
	uint8_t val = 0;

	bq25601_read_interface((uint8_t)(bq25601_CON8),
				     (&val), (uint8_t)(0xFF),
				     (uint8_t)(0x0)
				    );
	return val;
}

uint8_t bq25601_get_vbus_stat(void)
{
	uint8_t val = 0;

	bq25601_read_interface((uint8_t)(bq25601_CON8),
				     (&val),
				     (uint8_t)(CON8_VBUS_STAT_MASK),
				     (uint8_t)(CON8_VBUS_STAT_SHIFT)
				    );
	return val;
}

uint8_t bq25601_get_chrg_stat(void)
{
	uint8_t val = 0;

	bq25601_read_interface((uint8_t)(bq25601_CON8),
				     (&val),
				     (uint8_t)(CON8_CHRG_STAT_MASK),
				     (uint8_t)(CON8_CHRG_STAT_SHIFT)
				    );
	return val;
}

uint8_t bq25601_get_vsys_stat(void)
{
	uint8_t val = 0;

	bq25601_read_interface((uint8_t)(bq25601_CON8),
				     (&val),
				     (uint8_t)(CON8_VSYS_STAT_MASK),
				     (uint8_t)(CON8_VSYS_STAT_SHIFT)
				    );
	return val;
}

uint8_t bq25601_get_pg_stat(void)
{
	uint8_t val = 0;

	bq25601_read_interface((uint8_t)(bq25601_CON8),
				     (&val),
				     (uint8_t)(CON8_PG_STAT_MASK),
				     (uint8_t)(CON8_PG_STAT_SHIFT)
				    );
	return val;
}


/*CON10----------------------------------------------------------*/

void bq25601_set_int_mask(uint8_t val)
{
	bq25601_config_interface((uint8_t)(bq25601_CON10),
				       (uint8_t)(val),
				       (uint8_t)(CON10_INT_MASK_MASK),
				       (uint8_t)(CON10_INT_MASK_SHIFT)
				      );
}

/**********************************************************
 *
 *   [Internal Function]
 *
 *********************************************************/
static int bq25601_dump_register()
{

	uint8_t i = 0;
	uint8_t ret = 0;

//	pr_info("[bq25601] ");

	for(; i < bq25601_REG_NUM; i++)
	{
		ret = bq25601_read_byte(i, &bq25601_reg[i]);
		if (ret == 0) {
//			pr_info("[bq25601] i2c transfer error\n");
			return 1;
		}
//		pr_info("[0x%02X]=0x%02X", i, bq25601_reg[i]);
	}

//	pr_info("\r\n");
	return 0;
}


/**********************************************************
 *
 *   [Internal Function]
 *
 *********************************************************/
static void bq25601_hw_component_detect(void)
{
	uint8_t val = 0;

	bq25601_read_interface(0x0B, &val, 0xFF, 0x0);

	if (val == 0)
		g_bq25601_hw_exist = 0;
	else
		g_bq25601_hw_exist = 1;

	pr_info("[%s] exist=%d, Reg[0x0B]=0x%x\n", __func__,
		g_bq25601_hw_exist, val);
}


uint8_t bq25601_enable_charging(
				   bool en)
{
	uint8_t status = 0;

	pr_info("enable state : %d\n", en);
	if (en) {
		/* bq25601_config_interface(bq25601_CON3, 0x1, 0x1, 4); */
		/* enable charging */
		bq25601_set_en_hiz(0x0);
		bq25601_set_chg_config(en);
	} else {
		/* bq25601_config_interface(bq25601_CON3, 0x0, 0x1, 4); */
		/* enable charging */
		bq25601_set_chg_config(en);
		pr_info("[charging_enable] under test mode: disable charging\n");

		/*bq25601_set_en_hiz(0x1);*/
	}

	return status;
}

uint8_t bq25601_get_current(
			       uint32_t *ichg)
{
	uint8_t ret_val = 0;
#if 0 //todo
	uint8_t ret_force_20pct = 0;

	/* Get current level */
	bq25601_read_interface(bq25601_CON2, &ret_val, CON2_ICHG_MASK,
			       CON2_ICHG_SHIFT);

	/* Get Force 20% option */
	bq25601_read_interface(bq25601_CON2, &ret_force_20pct,
			       CON2_FORCE_20PCT_MASK,
			       CON2_FORCE_20PCT_SHIFT);

	/* Parsing */
	ret_val = (ret_val * 64) + 512;

#endif
	return ret_val;
}

/**
 * @brief	设置快速充电模式下的电流，单位uA
 */
uint8_t bq25601_set_fast_charge_current(
			       uint32_t current_value)
{
	uint8_t status = true;
	uint32_t set_chr_current;
	uint8_t array_size;
	uint8_t register_value;

	pr_info("&&&& charge_current_value = %lu\n", current_value);
	current_value /= 10;
	array_size = GETARRAYNUM(CS_VTH);
	set_chr_current = bmt_find_closest_level(CS_VTH, array_size,
			  current_value);
	register_value = charging_parameter_to_value(CS_VTH, array_size,
			 set_chr_current);
	//pr_info("&&&& charge_register_value = %d\n",register_value);
	pr_info("&&&& %s register_value = %d\n", __func__,
		register_value);
	bq25601_set_ichg(register_value);

	return status;
}

uint8_t bq25601_get_input_current(uint32_t *aicr)
{
	uint8_t ret = 0;
#if 0
	uint8_t val = 0;

	bq25601_read_interface(bq25601_CON0, &val, CON0_IINLIM_MASK,
			       CON0_IINLIM_SHIFT);
	ret = (int)val;
	*aicr = INPUT_CS_VTH[val];
#endif
	return ret;
}

/**
 * @brief	设置电流限制
 * @param	current_value	单位uA，例如100000 = 100mA
 */
uint8_t bq25601_set_input_current(
				     uint32_t current_value)
{
	uint8_t status = true;
	uint32_t set_chr_current;
	uint8_t array_size;
	uint8_t register_value;

	current_value /= 10;
	pr_info("&&&& current_value = %lu\n", current_value);
	array_size = GETARRAYNUM(INPUT_CS_VTH);
	set_chr_current = bmt_find_closest_level(INPUT_CS_VTH, array_size,
			  current_value);
	register_value = charging_parameter_to_value(INPUT_CS_VTH, array_size,
			 set_chr_current);
	pr_info("&&&& %s register_value = %d\n", __func__,
		register_value);
	bq25601_set_iinlim(register_value);

	return status;
}

uint8_t bq25601_set_cv_voltage(
				  uint32_t cv)
{
	uint8_t status = true;
	uint8_t array_size;
	uint8_t set_cv_voltage;
	unsigned short register_value;

	array_size = GETARRAYNUM(VBAT_CV_VTH);
	set_cv_voltage = bmt_find_closest_level(VBAT_CV_VTH, array_size, cv);
	register_value = charging_parameter_to_value(VBAT_CV_VTH, array_size,
			 set_cv_voltage);
	bq25601_set_vreg(register_value);
	pr_info("&&&& cv reg value = %d\n", register_value);

	return status;
}

uint8_t bq25601_reset_watch_dog_timer(struct charger_device
		*chg_dev)
{
	uint8_t status = true;

	pr_info("charging_reset_watch_dog_timer\n");

	bq25601_set_wdt_rst(0x1);	/* Kick watchdog */
	bq25601_set_watchdog(0x1);	/* WDT 40s */

	return status;
}

/**
 * @brief	设置输入电压最低范围
 * @param	vindpm 单位uV
 *
 */
uint8_t bq25601_set_vindpm_voltage(
				      uint32_t vindpm)
{
	uint8_t status = 0;
	uint8_t array_size;

	vindpm /= 1000;
	array_size = ARRAY_SIZE(VINDPM_REG);
	vindpm = bmt_find_closest_level(VINDPM_REG, array_size, vindpm);
	vindpm = charging_parameter_to_value(VINDPM_REG, array_size, vindpm);

	pr_info("%s vindpm =%lu\r\n", __func__, vindpm);

	bq25601_set_vindpm(vindpm);
	//	charging_set_vindpm(vindpm);
	/*bq25601_set_en_hiz(en);*/

	return status;
}

uint8_t bq25601_get_charging_status(
				       bool *is_done)
{
	uint8_t status = true;
	uint8_t ret_val;

	ret_val = bq25601_get_chrg_stat();

	if (ret_val == 0x3)
		*is_done = true;
	else
		*is_done = false;

	return status;
}

uint8_t bq25601_enable_otg( bool en)
{
	uint8_t ret = 0;

	pr_info("%s en = %d\n", __func__, en);
	if (en) {
		bq25601_set_chg_config(0);
		bq25601_set_otg_config(1);
		bq25601_set_watchdog(0x3);	/* WDT 160s */
	} else {
		bq25601_set_otg_config(0);
		bq25601_set_chg_config(1);
	}
	return ret;
}

uint8_t bq25601_set_boost_current_limit(uint32_t uA)
{
	uint8_t ret = 0;
	uint32_t array_size = 0;
	uint32_t boost_ilimit = 0;
	uint8_t boost_reg = 0;

	uA /= 1000;
	array_size = ARRAY_SIZE(BOOST_CURRENT_LIMIT);
	boost_ilimit = bmt_find_closest_level(BOOST_CURRENT_LIMIT, array_size,
					      uA);
	boost_reg = charging_parameter_to_value(BOOST_CURRENT_LIMIT,
						array_size, boost_ilimit);
	bq25601_set_boost_lim(boost_reg);

	return ret;
}

uint8_t bq25601_enable_safetytimer(bool en)
{
	uint8_t status = 0;

	if (en)
		bq25601_set_en_timer(0x1);
	else
		bq25601_set_en_timer(0x0);
	return status;
}

uint8_t bq25601_get_is_safetytimer_enable(bool *en)
{
	uint8_t val = 0;

	bq25601_read_interface(bq25601_CON5, &val, CON5_EN_TIMER_MASK,
			       CON5_EN_TIMER_SHIFT);
	*en = (bool)val;
	return val;
}

uint8_t bq25601_hw_init(void)
{
	uint8_t status = 0;
	bq25601_set_input_current(3000000);			/* uA, 3.0A 	REG00	xxx1,1101 */
	bq25601_set_fast_charge_current(1500000); 	/* uA, 1.5A		REG02	xxx1,1001 */
	bq25601_set_en_hiz(0x0);	/* Disable HIZ Mode 			REG00 	0xxx,xxxx */
	bq25601_set_vindpm_voltage(4600000); /* VIN DPM check 4.5V 	REG06	xxxx,0111 */
	bq25601_set_wdt_rst(0x1);	/* 喂狗 Kick watchdog 			REG01	x1xx,xxxx */
	bq25601_set_sys_min(0x3);	/* Minimum system voltage 3.2V 	REG01 	xxxx,011x */
	bq25601_set_iprechg(0x8);	/* Precharge current 540mA 		REG03	1000,xxxx */
	bq25601_set_iterm(0x2);		/* Termination current 180mA 	REG03	xxxx,0010 */
	bq25601_set_vreg(0x11);		/* VREG 4.208V 					REG04	0101,1xxx */
	bq25601_set_pfm(0x1);		/* disable pfm 					REG01	1xxx,xxxx */
	bq25601_set_rdson(0x0);     /* close rdson					REG02	xvxx,xxxx */
	bq25601_set_batlowv(0x1);	/* BATLOWV 3.0V 				REG01	xxxx,xxx1 */
	bq25601_set_vrechg(0x0);	/* VRECHG 0.1V (4.108V) 		REG04	xxxx,xxx0 */
	bq25601_set_en_term(0x1);	/* Enable termination 			REG05	1xxx,xxxx */
	bq25601_set_watchdog(0x1);	/* WDT 复位周期 40s 			REG05	xx01,xxxx */
	bq25601_set_en_timer(0x0);	/* Enable charge timer 			REG05	xxxx,1xxx */
	bq25601_set_int_mask(0x3);	/* Disable VINDPM & IINDPM INT	REG0A 	xxxx,xx11 */
	pr_debug_ratelimited("%s: hw_init down!\n", __func__);
	return status;
}

uint8_t charging_hw_update(void)
{
	bq25601_dump_register();
	bq25601_set_wdt_rst(0x01);	//每40s至少喂狗一次
	return 0;
}

uint8_t charging_driver_probe()
{
	pr_debug_ratelimited("[%s]\n", __func__);

	bq25601_hw_component_detect();
	bq25601_hw_init();
	bq25601_dump_register();

	return 0;
}

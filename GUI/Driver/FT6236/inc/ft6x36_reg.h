/*
 * ft6x36_reg.h
 *
 *  Created on: Nov 27, 2022
 *      Author: OldGerman
 *
 *  Change Logs:
 *  Date           Notes
 *  2022-10-27     the first version
 */

#ifndef _FT6X36_REG_H_
#define _FT6X36_REG_H_

//#include "touch.h"
//int rt_hw_ft6236_init(const char *name, struct rt_touch_config *cfg,  rt_base_t pin);
//0x08U 0x09U 0x0AU 0x0BU 0x0CU 0x0DU 0x0FU
#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stddef.h>

/** @addtogroup  Interfaces_Functions
 * @brief       This section provide a set of functions used to read and
 *              write a generic register of the device.
 *              MANDATORY: return 0 -> no Error.
 * @{
 *
 */

typedef int32_t (*ftdev_write_ptr)(void *, uint8_t, const uint8_t *, uint16_t);
typedef int32_t (*ftdev_read_ptr)(void *, uint8_t, uint8_t *, uint16_t);

typedef struct
{
	/** Component mandatory fields **/
	ftdev_write_ptr  write_reg;
	ftdev_read_ptr   read_reg;
	/** Customizable optional pointer **/
	void *handle;
} ftdev_ctx_t;


/**
 * @}
 *
 */

//#endif /* TPIC_SHARED_TYPES */
//
//#ifndef TPIC_UCF_SHARED_TYPES
//#define TPIC_UCF_SHARED_TYPES

/** @defgroup    Generic address-data structure definition
 * @brief       This structure is useful to load a predefined configuration
 *              of a sensor.
 *              You can create a sensor configuration by your own or using
 *              Unico / Unicleo tools available on STMicroelectronics
 *              web site.
 *
 * @{
 *
 */

typedef struct
{
	uint8_t address;
	uint8_t data;
} ucf_line_t;

/**
 * @}
 *
 */

//#endif /* TPIC_UCF_SHARED_TYPES (TOUCH PANEL IC)*/

/**
 * @}
 *
 */

/** @defgroup FT6X36_Infos
 * @{
 *
 */

/** I2C Device Address 7 bit format **/
#define FT6X36_I2C_ADD     0x38U
/** I2C Device Address 8 bit format **/
#define FT6X36_I2C_ADD_L   0x70U

/** Device Identification (Who am I) **/
//#define FT6X36_FIRMID
#define FT6X36_FOCALTECH_ID		0x11U
#define FT6X36_RELEASE_CODE_ID  0x01U


/**
 * @}
 *
 */

/** @def register group with consecutive addresses: 0x00U ~ 0x0EU
 * @{
 *
 */
#define FT6X36_REG_DEV_MODE				0x00U	/* R/W */
typedef struct
{
	uint8_t not_used_01       		: 4;
	uint8_t device_mode       		: 3;
	uint8_t not_used_02       		: 1;
} ft6x36_reg_dev_mode_t;

#define FT6X36_REG_GEST_ID				0x01U	/* R */
typedef struct
{
	uint8_t gest_id              		: 8;
} ft6x36_reg_gest_id_t;

#define FT6X36_REG_TD_STATUS			0x02U	/* R */
typedef struct
{
	uint8_t number_of_touch_points  : 4;		/* The detected point number, 1-2 is valid */
	uint8_t not_used_01           	: 4;
} ft6x36_reg_td_status_t;


#define FT6X36_REG_P1_XH				0x03U	/* R */
#define FT6X36_REG_P1_XL				0x04U	/* R */
#define FT6X36_REG_P1_YH				0x05U	/* R */
#define FT6X36_REG_P1_YL				0x06U	/* R */
#define FT6X36_REG_P1_WEIGHT			0x07U	/* R */	/* 1st Touch Weight */
#define FT6X36_REG_P1_MISC				0x08U	/* R */	/* 1st Touch Area */

#define FT6X36_REG_P2_XH				0x09U	/* R */
#define FT6X36_REG_P2_XL				0x0AU	/* R */
#define FT6X36_REG_P2_YH				0x0BU	/* R */
#define FT6X36_REG_P2_YL				0x0CU	/* R */
#define FT6X36_REG_P2_WEIGHT			0x0DU	/* R */	/* 2st Touch Weight */
#define FT6X36_REG_P2_MISC				0x0EU	/* R */	/* 2st Touch Area */

typedef struct
{
	uint8_t touch_x_position_h   		: 4;
	uint8_t not_used_01       			: 2;
	uint8_t event_flag       			: 2;
} ft6x36_reg_px_xh_t;
typedef struct
{
	uint8_t touch_x_position_l   		: 8;
} ft6x36_reg_px_xl_t;
typedef struct
{
	uint8_t touch_y_position_h   		: 4;
	uint8_t touch_id       				: 4;
} ft6x36_reg_px_yh_t;
typedef struct
{
	uint8_t touch_y_position_l   		: 8;
} ft6x36_reg_px_yl_t;
typedef struct
{
	uint8_t weight              		: 8;
} ft6x36_reg_px_weight_t;
typedef struct
{
	uint8_t misc              			: 8;
} ft6x36_reg_px_misc_t;


typedef union
{
	struct {
		ft6x36_reg_gest_id_t	gest_id;	/* 0x01U */		/* R */
		ft6x36_reg_td_status_t	td_status;	/* 0x02U */		/* R */

		ft6x36_reg_px_xh_t		p1_xh;		/* 0x03U */		/* R */
		ft6x36_reg_px_xl_t		p1_xl;		/* 0x04U */		/* R */
		ft6x36_reg_px_yh_t		p1_yh;		/* 0x05U */		/* R */
		ft6x36_reg_px_yl_t		p1_yl;		/* 0x06U */		/* R */
		ft6x36_reg_px_weight_t	p1_weight;	/* 0x07U */		/* R */
		ft6x36_reg_px_misc_t	p1_misc;	/* 0x08U */		/* R */

		ft6x36_reg_px_xh_t		p2_xh;		/* 0x09U */		/* R */
		ft6x36_reg_px_xl_t		p2_xl;		/* 0x0AU */		/* R */
		ft6x36_reg_px_yh_t		p2_yh;		/* 0x0BU */		/* R */
		ft6x36_reg_px_yl_t		p2_yl;		/* 0x0CU */		/* R */
		ft6x36_reg_px_weight_t	p2_weight;	/* 0x0DU */		/* R */
		ft6x36_reg_px_misc_t	p2_misc;	/* 0x0EU */		/* R */
	}data;
	uint8_t					byte[14];
} ft6x36_reg_td_t;
int32_t ft6x36_touch_data_get(ftdev_ctx_t *ctx, ft6x36_reg_td_t *buff);

typedef enum
{
	FT6X36_TD_STATUS_0_POINT = 0x00U,	/* 检测到0个有效触摸点 */
	FT6X36_TD_STATUS_1_POINT = 0x01U,	/* 检测到1个有效触摸点 */
	FT6X36_TD_STATUS_2_POINT = 0x02U,	/* 检测到2个有效触摸点 */
} ft6x36_td_status_t;	//自己加的，手册没有此枚举

typedef enum
{
	FT6X36_DEVICE_MODE_WORKING_MODE = 0x00U,	/* 000b */
	FT6X36_DEVICE_MODE_FACTORY_MODE = 0x04U,	/* 100b */
} ft6x36_device_mode_t;
/* 访问0x00寄存器，可读可写*/
int32_t ft6x36_device_mode_set(ftdev_ctx_t *ctx, ft6x36_device_mode_t val);
int32_t ft6x36_device_mode_get(ftdev_ctx_t *ctx, ft6x36_device_mode_t *val);

typedef enum
{
	FT6X36_GEST_ID_MOVE_UP 		= 0X10U,
	FT6X36_GEST_ID_MOVE_RIGHT 	= 0X14U,
	FT6X36_GEST_ID_MOVE_DOWN 	= 0X18U,
	FT6X36_GEST_ID_MOVE_LEFT 	= 0X1CU,
	FT6X36_GEST_ID_ZOOM_IN 		= 0X48U,
	FT6X36_GEST_ID_ZOOM_OUT 	= 0X49U,
	FT6X36_GEST_ID_NO_GESTURE 	= 0X00U,
} ft6x36_gest_id_t;
/* 访问0x01寄存器，仅可读*/
int32_t ft6x36_gest_id_get(ftdev_ctx_t *ctx, ft6x36_gest_id_t *val);

/* 3.1.4 Pn_XH (n:1-2) 0x03 bit 7:6 Event Flag */
typedef enum
{
	FT6X36_EVENT_FLAG_PRESS_DOWN 		= 0X00U,	/* 00b */	/* 按下 */
	FT6X36_EVENT_FLAG_LIFT_UP 			= 0X01U,	/* 01b */	/* 抬起 */
	FT6X36_EVENT_FLAG_CONTACT 			= 0X02U,	/* 10b */	/* 保持接触 */
	FT6X36_EVENT_FLAG_NO_EVENT 			= 0X03U,	/* 11b */	/* 无 */
} ft6x36_event_flag_t;
/* 访问p1的0x03或p2的0x09寄存器，仅可读*/
int32_t ft6x36_event_flag_get(ftdev_ctx_t *ctx, ft6x36_event_flag_t *val, uint8_t reg_px_xh);

/**
 * @}
 *
 */

#define FT6X36_REG_TH_GROUP				0x80U		/* R/W */	/* Threshold for touch detection, (ft5) The actual value will be 4 times of the register’s value. */ /* Default:280/4? */
/**
 * @ft5xxx
 * @{
 */
#define FT6X36_REG_TH_PEAK				0x81U		/* R/W */ 	/* valid touching peak detect threshold.*/					/* Default:60 */
#define FT6X36_REG_TH_CAL				0x82U		/* R/W */ 	/* the threshold when calculating the focus of touching */	/* Default:16 */
#define FT6X36_REG_TH_WATER				0x82U		/* R/W */ 	/* the threshold when there is surface water. */			/* Default:60 */
#define FT6X36_REG_TH_TEMP				0x84U		/* R/W */ 	/* the threshold of temperature compensation. */			/* Default:10 */
/**
 * @}
 *
 */
#define FT6X36_REG_TH_DIFF				0x85U		/* R/W */	/* Filter function coefficient */
#define FT6X36_REG_CTRL					0x86U		/* R/W */	/* 0: Will keep the Active mode when there is no touching. 1: Switching from Active mode to Monitor mode automatically when there is no touching. */
#define FT6X36_REG_TIME_ENTER_MONITOR	0x87U 		/* R/W */	/* The time period of switching from Active mode to Monitor mode when there is no touching. */
#define FT6X36_REG_PERIOD_ACTIVE		0x88U		/* R/W */	/* Report rate in Active mode. (ft5)Range form 3 to 14, default 12 */
#define FT6X36_REG_PERIOD_MONITOR		0x89u		/* R/W */	/* report rate in monitor mode. value in ms？ */	/* default: 0x28 */
#define FT6X36_REG_NO_USED_0X90			0x90U		/* R/W */	/* Reversed */
#define FT6X36_REG_RADIAN_VALUE			0x91U		/* R/W */	/* The value of the minimum allowed angle while Rotating gesture mode */
#define FT6X36_REG_OFFSET_LEFT_RIGHT	0x92U		/* R/W */	/* Maximum offset while Moving Left and Moving Right gesture */
#define FT6X36_REG_OFFSET_UP_DOWN		0x93U		/* R/W */	/* Maximum offset while Moving Up and Moving Down gesture */
#define FT6X36_REG_DISTANCE_LEFT_RIGHT	0x94U		/* R/W */	/* Minimum distance while Moving Left and Moving Right gesture */
#define FT6X36_REG_DISTANCE_UP_DOWN		0x95U		/* R/W */	/* Minimum distance while Moving	 Up and Moving Down gesture */
#define FT6X36_REG_DISTANCE_ZOOM		0x96U		/* R/W */	/* Maximum distance while Zoom In and Zoom Out gesture */
/**
 * @}
 *
 */
typedef union
{
	struct {
		uint8_t 	th_group;			/* 0x80U */				/* R/W */
		uint8_t 	th_peak;			/* 0x81U */				/* R/W */
		uint8_t 	th_cal;				/* 0x82U */				/* R/W */
		uint8_t 	th_water;			/* 0x83U */				/* R/W */
		uint8_t 	th_temp;			/* 0x84U */				/* R/W */
		uint8_t 	th_diff;			/* 0x85U */				/* R/W */
		uint8_t 	ctrl;				/* 0x86U */				/* R/W */
		uint8_t 	time_enter_monitor;	/* 0x87U */				/* R/W */
		uint8_t 	period_active;		/* 0x88U */				/* R/W */
		uint8_t 	period_monitor;		/* 0x89U */				/* R/W */
		uint8_t		no_used_0x90;		/* 0x90U */				/* R/W */
		uint8_t 	radian_value;		/* 0x91U */				/* R/W */
		uint8_t 	offset_left_right;	/* 0x92U */				/* R/W */
		uint8_t		offset_up_down;		/* 0x93U */				/* R/W */
		uint8_t		distance_left_right;/* 0x94U */				/* R/W */
		uint8_t		distance_up_down;	/* 0x95U */				/* R/W */
		uint8_t		distance_zoom;		/* 0x96U */				/* R/W */
	}data;
	uint8_t			byte[16];
} ft6x36_reg_settings_t;
int32_t ft6x36_settings_set(ftdev_ctx_t *ctx, ft6x36_reg_settings_t *val);
int32_t ft6x36_settings_get(ftdev_ctx_t *ctx, ft6x36_reg_settings_t *val);


/** @def register group with consecutive addresses: 0xA1U ~ 0xA8U
 * @{
 *
 */
#define FT6X36_REG_LIB_VER_H			0xA1U		/* R */		/* High 8-bit of LIB Version info */
#define FT6X36_REG_LIB_VER_L			0xA2U		/* R */		/* Low 8-bit of LIB Version info */
#define FT6X36_REG_CIPHER				0xA3U		/* R */		/* Chip Selecting, This register describes vendor’s chip id. 该寄存器描述了供应商的芯片 ID, 不同尺寸的触摸面板的chip id不同*/
#define FT6X36_REG_G_MODE				0xA4U		/* R/W */	/* 0x00: Interrupt Polling mode  0x01: Interrupt Trigger mode */
int32_t ft6x36_g_mode_set(ftdev_ctx_t *ctx, uint8_t val);
int32_t ft6x36_g_mode_get(ftdev_ctx_t *ctx, uint8_t *val);
#define FT6X36_REG_PWR_MODE				0xA5U		/* R/W */	/* Current power mode which system is in */
typedef enum
{
	FT6X36_PWR_MODE_ACTIVE 		= 0X00U,	/* 活动模式 */
	FT6X36_PWR_MODE_MONITOR 	= 0X01U,	/* 监控模式 */
	FT6X36_PWR_MODE_HIBERNATE 	= 0X03U,	/* 休眠模式（深度休眠） */
} ft6x36_pwr_mode_t;
int32_t ft6x36_pwr_mode_set(ftdev_ctx_t *ctx, ft6x36_pwr_mode_t val);
int32_t ft6x36_pwr_mode_get(ftdev_ctx_t *ctx, ft6x36_pwr_mode_t *val);
#define FT6X36_REG_FIRMID				0xA6U		/* R */		/* Firmware Version */
#define FT6X36_REG_NO_USED_0XA7			0xA7U		/* R */		/* Reversed */
#define FT6X36_REG_FOCALTECH_ID			0xA8U		/* R */		/* FocalTech’s Panel ID */
#define FT6X36_REG_ERR					0xA9U		/* R */		/* (ft5) Error Code */
typedef enum
{
	FT6X36_ERR_CODE_OK			= 0X00U,	/* ok */
	FT6X36_ERR_CODE_REG_WR		= 0X03U,	/* chip register writing inconsistent with reading */
	FT6X36_ERR_CODE_START_FAIL	= 0X05U,	/* chip start fail  */
	FT6X36_ERR_CODE_CALIBRATION	= 0X1AU,	/* no match among the basic input(such as TX_ORDER) while calibration */
}ft6x36_err_code_t;
int32_t ft6x36_err_code_get(ftdev_ctx_t *ctx, ft6x36_err_code_t *val);

typedef union
{
	struct {
		uint8_t			lib_ver_h;			/* 0xA1U */				/* R */
		uint8_t			lib_ver_l;			/* 0xA2U */				/* R */
		uint8_t			cipher;				/* 0xA3U */				/* R */
		uint8_t			g_mode;				/* 0xA4U */				/* R/W */
		uint8_t			pwr_mode;			/* 0xA5U */				/* R/W */
		uint8_t			firmid;				/* 0xA6U */				/* R */
		uint8_t			no_used_0xa7;		/* 0xA7U  */			/* ? */
		uint8_t			focaltech_id;		/* 0xA8U */				/* R */
		uint8_t			err;				/* 0xA9U */				/* R */
	}data;
	uint8_t			byte[9];
} ft6x36_reg_info_t;
int32_t ft6x36_info_get(ftdev_ctx_t *ctx, ft6x36_reg_info_t *val);

/**
 * @}
 *
 */


#define FT6X36_REG_RELEASE_CODE_ID   	0xAFU   	/* R */		/* Release code version */
int32_t ft6x36_release_code_id_get(ftdev_ctx_t *ctx, uint8_t *val);

#define FT6X36_REG_STATE				0XBCU		/* R/W */	/* Current Operating mode */
int32_t ft6x36_state_set(ftdev_ctx_t *ctx, uint8_t val);
int32_t ft6x36_state_get(ftdev_ctx_t *ctx, uint8_t *val);

/**
 * @defgroup FT62X6_Register_Union
 * @brief    This union group all the registers having a bit-field
 *           description except p1_x, p1_y, p2_x, p2_y
 *           This union is useful but it's not needed by the driver.
 *
 *           REMOVING this union you are compliant with:
 *           MISRA-C 2012 [Rule 19.2] -> " Union are not allowed "
 *
 * @{
 *
 */
typedef union {
	ft6x36_reg_dev_mode_t 	dev_mode;	/* 0x00U */				/* R/W */

	ft6x36_reg_gest_id_t	gest_id;	/* 0x01U */		/* R */
	ft6x36_reg_td_status_t	td_status;	/* 0x02U */		/* R */

	ft6x36_reg_px_xh_t		p1_xh;		/* 0x03U */		/* R */
	ft6x36_reg_px_xl_t		p1_xl;		/* 0x04U */		/* R */
	ft6x36_reg_px_yh_t		p1_yh;		/* 0x05U */		/* R */
	ft6x36_reg_px_yl_t		p1_yl;		/* 0x06U */		/* R */
	ft6x36_reg_px_weight_t	p1_weight;	/* 0x07U */		/* R */
	ft6x36_reg_px_misc_t	p1_misc;	/* 0x08U */		/* R */

	ft6x36_reg_px_xh_t		p2_xh;		/* 0x09U */		/* R */
	ft6x36_reg_px_xl_t		p2_xl;		/* 0x0AU */		/* R */
	ft6x36_reg_px_yh_t		p2_yh;		/* 0x0BU */		/* R */
	ft6x36_reg_px_yl_t		p2_yl;		/* 0x0CU */		/* R */
	ft6x36_reg_px_weight_t	p2_weight;	/* 0x0DU */		/* R */
	ft6x36_reg_px_misc_t	p2_misc;	/* 0x0EU */		/* R */

	uint8_t 		th_group;			/* 0x80U */				/* R/W */
	uint8_t 		th_peak;			/* 0x81U */				/* R/W */
	uint8_t 		th_cal;				/* 0x82U */				/* R/W */
	uint8_t 		th_water;			/* 0x83U */				/* R/W */
	uint8_t 		th_temp;			/* 0x84U */				/* R/W */
	uint8_t 		th_diff;			/* 0x85U */				/* R/W */
	uint8_t 		ctrl;				/* 0x86U */				/* R/W */
	uint8_t 		time_enter_monitor;	/* 0x87U */				/* R/W */
	uint8_t 		period_active;		/* 0x88U */				/* R/W */
	uint8_t 		period_monitor;		/* 0x89U */				/* R/W */
	uint8_t			no_used_0x90;		/* 0x90U */				/* R/W */
	uint8_t 		radian_value;		/* 0x91U */				/* R/W */
	uint8_t 		offset_left_right;	/* 0x92U */				/* R/W */
	uint8_t			offset_up_down;		/* 0x93U */				/* R/W */
	uint8_t			distance_left_right;/* 0x94U */				/* R/W */
	uint8_t			distance_up_down;	/* 0x95U */				/* R/W */
	uint8_t			distance_zoom;		/* 0x96U */				/* R/W */

	uint8_t			lib_ver_h;			/* 0xA1U */				/* R */
	uint8_t			lib_ver_l;			/* 0xA2U */				/* R */
	uint8_t			cipher;				/* 0xA3U */				/* R */
	uint8_t			g_mode;				/* 0xA4U */				/* R/W */
	uint8_t			pwr_mode;			/* 0xA5U */				/* R/W */
	uint8_t			firmid;				/* 0xA6U */				/* R */
	uint8_t			no_used_0xa7;		/* 0xA7U  */			/* ? */
	uint8_t			focaltech_id;		/* 0xA8U */				/* R */
	uint8_t			err;				/* 0xA9U */				/* R */

	uint8_t			release_code_id;	/* 0xAFU */				/* R */

	uint8_t			state;				/* 0xBCU */				/* R/W */

	uint16_t 		half_word;
}ft6x36_reg_t;

/**
 * @}
 *
 */

#ifdef __cplusplus
}
#endif
#endif /* _FT6X36_REG_H_ */

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

#endif /* TPIC_SHARED_TYPES */

#ifndef TPIC_UCF_SHARED_TYPES
#define TPIC_UCF_SHARED_TYPES

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

#endif /* TPIC_UCF_SHARED_TYPES (TOUCH PANEL IC)*/

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
  uint8_t not_used_01       		: 1;
  uint8_t device_mode       		: 3;
  uint8_t not_used_02       		: 4;
} ft6x36_reg_dev_mode_t;
#define FT6X36_REG_GEST_ID				0x01U	/* R */
#define FT6X36_REG_TD_STATUS			0x02U	/* R */
typedef struct
{
  uint8_t not_used_01           	: 4;
  uint8_t number_of_touch_points  	: 4;		/* The detected point number, 1-2 is valid */
} ft6x36_reg_td_status_t;
#define FT6X36_REG_P1_XH				0x03U	/* R */
typedef struct
{
  uint8_t _1st_event_flag       	: 2;
  uint8_t not_used_01       		: 2;
  uint8_t _1st_touch_x_position_h   : 4;
} ft6x36_reg_p1_xh_t;
#define FT6X36_REG_P1_XL				0x04U	/* R */
typedef struct
{
  uint16_t _1st_event_flag       	: 2;
  uint16_t not_used_01       		: 2;
  uint16_t _1st_touch_x_position    : 12;
} ft6x36_reg_p1_x_t;

#define FT6X36_REG_P1_YH				0x05U	/* R */
typedef struct
{
  uint8_t _1st_touch_id       		: 4;
  uint8_t _1st_touch_y_position_h   : 4;
} ft6x36_reg_p1_yh_t;
#define FT6X36_REG_P1_YL				0x06U	/* R */
typedef struct
{
  uint16_t _1st_touch_id       		: 4;
  uint16_t _1st_touch_y_position    : 12;
} ft6x36_reg_p1_y_t;

#define FT6X36_REG_P1_WEIGHT			0x07U	/* R */	/* 1st Touch Weight */
#define FT6X36_REG_P1_MISC				0x08U	/* R */	/* 1st Touch Area */
#define FT6X36_REG_P2_XH				0x09U	/* R */
typedef struct
{
  uint8_t _2st_event_flag       	: 2;
  uint8_t not_used_01       		: 2;
  uint8_t _2st_touch_x_position_h   : 4;
} ft6x36_reg_p2_xh_t;
#define FT6X36_REG_P2_XL				0x0AU	/* R */
typedef struct
{
  uint16_t _2st_event_flag       	: 2;
  uint16_t not_used_01       		: 2;
  uint16_t _2st_touch_x_position    : 12;
} ft6x36_reg_p2_x_t;
#define FT6X36_REG_P2_YH				0x0BU	/* R */
typedef struct
{
  uint8_t _2st_touch_id       		: 4;
  uint8_t _2st_touch_y_position_h   : 4;
} ft6x36_reg_p2_yh_t;
#define FT6X36_REG_P2_YL				0x0CU	/* R */
typedef struct
{
  uint16_t _2st_touch_id       		: 4;
  uint16_t _2st_touch_y_position   : 12;
} ft6x36_reg_p2_y_t;
#define FT6X36_REG_P2_WEIGHT			0x0DU	/* R */	/* 2st Touch Weight */
#define FT6X36_REG_P2_MISC				0x0EU	/* R */	/* 2st Touch Area */

typedef struct
{
	ft6x36_reg_dev_mode_t 	dev_mode_t;		/* 0x00U */				/* R/W */
	uint8_t					gest_id;		/* 0x01U */				/* R */
	ft6x36_reg_td_status_t	td_status_t;	/* 0x02U */				/* R */
	ft6x36_reg_p1_x_t		p1_x_t;			/* 0x03U ~ 0x04U */		/* R */
	ft6x36_reg_p1_y_t		p1_y_t;			/* 0x05U ~ 0x06U */		/* R */
	uint8_t					p1_weight;		/* 0x07U */				/* R */
	uint8_t					p1_misc;		/* 0x08U */				/* R */
	ft6x36_reg_p2_x_t		p2_x_t;			/* 0x09U ~ 0x0AU */		/* R */
	ft6x36_reg_p2_y_t		p2_y_t;			/* 0x0BU ~ 0x0CU */		/* R */
	uint8_t					p2_weight;		/* 0x07U */				/* R */
	uint8_t					p2_misc;		/* 0x08U */				/* R */
} ft6x36RegGroup1_t;


typedef enum
{
	FT6X36_WORKING_MODE = 0x00U,	/* 000b */
	FT6X36_FACTORY_MODE = 0x04U,	/* 100b */
} ft6x36_device_mode_t;
/* 访问0x00寄存器，可读可写*/
int32_t ft6x36_device_mode_set(ftdev_ctx_t *ctx,
		ft6x36_device_mode_t val);
int32_t ft6x36_device_mode_get(ftdev_ctx_t *ctx,
		ft6x36_device_mode_t *val);

typedef enum
{
	FT6X36_MOVE_UP 		= 0X10U,
	FT6X36_MOVE_RIGHT 	= 0X14U,
	FT6X36_MOVE_DOWN 	= 0X18U,
	FT6X36_MOVE_LEFT 	= 0X1CU,
	FT6X36_ZOOM_IN 		= 0X48U,
	FT6X36_ZOOM_OUT 	= 0X49U,
	FT6X36_NO_GESTURE 	= 0X00U,
} ft6x36_gest_id_t;
/* 访问0x01寄存器，仅可读*/
int32_t ft6x36_gest_id_get(ftdev_ctx_t *ctx,
		ft6x36_gest_id_t *val);

/* 3.1.4 Pn_XH (n:1-2) 0x03 bit 7:6 Event Flag */
typedef enum
{
	FT6X36_PRESS_DOWN 		= 0X00U,	/* 00b */	/* 按下 */
	FT6X36_LIFT_UP 			= 0X01U,	/* 01b */	/* 抬起 */
	FT6X36_CONTACT 			= 0X02U,	/* 10b */	/* 保持接触 */
	FT6X36_NO_EVENT 		= 0X03U,	/* 11b */	/* 无 */
} ft6x36_event_flag_t;
/* 访问p1的0x03或p2的0x09寄存器，仅可读*/
int32_t ft6x36_event_flag_get(ftdev_ctx_t *ctx,
		ft6x36_event_flag_t *val, uint8_t px /*1 or 2 (1st or 2nd point)*/ );

/**
  * @}
  *
  */

#define FT6X36_REG_TH_GROUP				0x80U		/* R/W */	/* Threshold for touch detection */

/** @def register group with consecutive addresses: 0x85U ~ 0x89U
  * @{
  *
  */
#define FT6X36_REG_TH_DIFF				0x85U		/* R/W */	/* Filter function coefficient */
#define FT6X36_REG_CTRL					0x86U		/* R/W */	/* 0: Will keep the Active mode when there is no touching. 1: Switching from Active mode to Monitor mode automatically when there is no touching. */
#define FT6X36_REG_TIMEENTERMONITOR		0x87U 		/* R/W */	/* The time period of switching from Active mode to Monitor mode when there is no touching. */
#define FT6X36_REG_PERIODACTIVE			0x88U		/* R/W */	/* Report rate in Active mode. */
#define FT6X36_REG_PERIODMONITOR		0x89U		/* R/W */	/* Report rate in Monitor mode. */
/**
  * @}
  *
  */

/** @def register group with consecutive addresses: 0x91U ~ 0x96U
  * @{
  *
  */
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

/** @def register group with consecutive addresses: 0xA1U ~ 0xA8U
  * @{
  *
  */
#define FT6X36_REG_LIB_VER_H			0xA1U		/* R */		/* High 8-bit of LIB Version info */
#define FT6X36_REG_LIB_VER_L			0xA2U		/* R */		/* Low 8-bit of LIB Version info */
#define FT6X36_REG_CIPHER				0xA3U		/* R */		/* Chip Selecting */
#define FT6X36_REG_G_MODE				0xA4U		/* R/W */	/* 0x00: Interrupt Polling mode  0x01: Interrupt Trigger mode */
#define FT6X36_REG_PWR_MODE				0xA5U		/* R/W */	/* Current power mode which system is in */
#define FT6X36_REG_FIRMID				0xA6U		/* R */		/* Firmware Version */
#define FT6X36_REG_FOCALTECH_ID			0xA8U		/* R */		/* FocalTech’s Panel ID */
/**
  * @}
  *
  */

#define FT6X36_REG_RELEASE_CODE_ID   	0xAFU   	/* R */		/* Release code version */

#define FT6X36_REG_STATE				0XBCU		/* R/W */	/* Current Operating mode */


/* 我需要一次读完所有可读的连续寄存器组吗？*/
/* 将连续的寄存器组封装成一个typedef union，这样好传这组寄存器的首地址一次读取，然后使用union成员解封*/


typedef union
{
	ft6x36RegP1_t ft6x36RegP1_t;
	uint8_t byte[2];		//Colum对象成员prBits和mask修改bits时使用
} ft6x36RegP1Type;

#ifdef __cplusplus
}

#endif /* _FT6X36_REG_H_ */

/**
  ******************************************************************************
  * @file    ft6x36_reg.c
  * @author  OldGerman
  * @brief   FT6X36 driver file
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */

#include "bsp_config.h"
#ifdef EN_BSP_FT6x36
#include "bsp.h"

/**
  * @defgroup  FT6X36
  * @brief     This file provides a set of functions needed to drive the
  *            ft6x36 enanced inertial module.
  * @{
  *
  */

/**
  * @defgroup  FT6X36_Interfaces_Functions
  * @brief     This section provide a set of functions used to read and
  *            write a generic register of the device.
  *            MANDATORY: return 0 -> no Error.
  * @{
  *
  */

/**
  * @brief  Read generic device register
  *
  * @param  ctx   read / write interface definitions(ptr)
  * @param  reg   register to read
  * @param  data  pointer to buffer that store the data read(ptr)
  * @param  len   number of consecutive register to read
  * @retval          interface status (MANDATORY: return 0 -> no Error)
  *
  */
int32_t ft6x36_read_reg(ftdev_ctx_t *ctx, uint8_t reg, uint8_t *data,
                        uint16_t len)
{
  int32_t ret;

  ret = ctx->read_reg(ctx->handle, reg, data, len);

  return ret;
}

/**
  * @brief  Write generic device register
  *
  * @param  ctx   read / write interface definitions(ptr)
  * @param  reg   register to write
  * @param  data  pointer to data to write in register reg(ptr)
  * @param  len   number of consecutive register to write
  * @retval          interface status (MANDATORY: return 0 -> no Error)
  *
  */
int32_t ft6x36_write_reg(ftdev_ctx_t *ctx, uint8_t reg,
                         uint8_t *data,
                         uint16_t len)
{
  int32_t ret;

  ret = ctx->write_reg(ctx->handle, reg, data, len);

  return ret;
}

/**
 * @}
 *
 */

/**
 * @defgroup  FT6X36_Data_generation
 * @brief     This section group all the functions concerning data generation.
 * @{
 *
 */

/**
  * @brief  Operating device mode selection.[set]
  *
  * @param  ctx      read / write interface definitions
  * @param  val      change the values of device_mode in reg FT6X36_REG_DEV_MODE
  * @retval          interface status (MANDATORY: return 0 -> no Error)
  *
  */
int32_t ft6x36_device_mode_set(ftdev_ctx_t *ctx,
		ft6x36_device_mode_t val)
{
	  ft6x36_reg_dev_mode_t reg_dev_mode;
	  int32_t ret;

	  ret = ft6x36_read_reg(ctx, FT6X36_REG_DEV_MODE,
	                        (uint8_t *)&reg_dev_mode, 1);

	  if (ret == 0)
	  {
	    reg_dev_mode.device_mode = (uint8_t)val;
	    ret = ft6x36_write_reg(ctx, FT6X36_REG_DEV_MODE, (uint8_t *)&reg_dev_mode, 1);
	  }

	  return ret;
}

/**
  * @brief  Output device mode selection.[get]
  *
  * @param  ctx      read / write interface definitions
  * @param  val      get the values of device_mode in reg FT6X36_REG_DEV_MODE
  * @retval          interface status (MANDATORY: return 0 -> no Error)
  *
  */
int32_t ft6x36_device_mode_get(ftdev_ctx_t *ctx,
		ft6x36_device_mode_t *val)
{
	  ft6x36_reg_dev_mode_t reg_dev_mode;
	  int32_t ret;

	  ret = ft6x36_read_reg(ctx, FT6X36_REG_DEV_MODE, (uint8_t *)&reg_dev_mode, 1);

	  switch (reg_dev_mode.device_mode)
	  {
	    case FT6X36_DEVICE_MODE_WORKING_MODE:
	      *val = FT6X36_DEVICE_MODE_WORKING_MODE;
	      break;

	    case FT6X36_DEVICE_MODE_FACTORY_MODE:
	      *val = FT6X36_DEVICE_MODE_FACTORY_MODE;
	      break;

	    default:
	      *val = FT6X36_DEVICE_MODE_WORKING_MODE;
	      break;
	  }

	  return ret;
}

/**
  * @brief  Output gesture id selection.[get]
  *
  * @param  ctx      read / write interface definitions
  * @param  val      get the values of device_mode in reg FT6X36_REG_GEST_ID
  * @retval          interface status (MANDATORY: return 0 -> no Error)
  *
  */
int32_t ft6x36_gest_id_get(ftdev_ctx_t *ctx,
		ft6x36_gest_id_t *val){
	ft6x36_reg_gest_id_t reg_gest_id;
	int32_t ret;

	ret = ft6x36_read_reg(ctx, FT6X36_REG_GEST_ID, (uint8_t *)&reg_gest_id, 1);
	switch (reg_gest_id.gest_id)
	{
	case FT6X36_GEST_ID_MOVE_UP:
		*val = FT6X36_GEST_ID_MOVE_UP;
		break;

	case FT6X36_GEST_ID_MOVE_RIGHT:
		*val = FT6X36_GEST_ID_MOVE_RIGHT;
		break;

	case FT6X36_GEST_ID_MOVE_DOWN:
		*val = FT6X36_GEST_ID_MOVE_DOWN;
		break;

	case FT6X36_GEST_ID_MOVE_LEFT:
		*val = FT6X36_GEST_ID_MOVE_LEFT;
		break;

	case FT6X36_GEST_ID_ZOOM_IN:
		*val = FT6X36_GEST_ID_ZOOM_IN;
		break;

	case FT6X36_GEST_ID_ZOOM_OUT:
		*val = FT6X36_GEST_ID_ZOOM_OUT;
		break;
	default:
		*val = FT6X36_GEST_ID_NO_GESTURE;
		break;
	}

	return ret;
}

/**
  * @brief  Output event flag selection.[get]
  *
  * @param  ctx      read / write interface definitions
  * @param  val      get the values of device_mode in reg FT6X36_REG_GEST_ID
  * @param  reg		 FT6X36_REG_P1_XH or FT6X36_REG_P2_XH
  * @retval          interface status (MANDATORY: return 0 -> no Error)
  *
  */
int32_t ft6x36_event_flag_get(ftdev_ctx_t *ctx,
		ft6x36_event_flag_t *val, uint8_t reg )
{
	ft6x36_reg_px_xh_t reg_px_xh;
	int32_t ret;

	ret = ft6x36_read_reg(ctx, reg, (uint8_t *)&reg_px_xh, 1);
	switch (reg_px_xh.event_flag)
	{
	case FT6X36_EVENT_FLAG_PRESS_DOWN:
		*val = FT6X36_EVENT_FLAG_PRESS_DOWN;
		break;

	case FT6X36_EVENT_FLAG_LIFT_UP:
		*val = FT6X36_EVENT_FLAG_LIFT_UP;
		break;

	case FT6X36_EVENT_FLAG_CONTACT:
		*val = FT6X36_EVENT_FLAG_CONTACT;
		break;

	default:
		*val = FT6X36_EVENT_FLAG_NO_EVENT;
		break;
	}

	return ret;
}


/**
 * @brief  touch data output value.[get]
 *
 * @param  ctx      read / write interface definitions
 * @param  buff   	ft6x36_reg_td_t that stores data read
 * @retval          interface status (MANDATORY: return 0 -> no Error)
 *
 */
int32_t ft6x36_touch_data_get(ftdev_ctx_t *ctx, ft6x36_reg_td_t *buff)
{
	int32_t ret;
	//取结构体指针成员地址的时候，我犹豫了，到底是&(t->m)还是&t->m；最后查了很久，才找到。这两个是一样的，因为->运算符的优先级高于&
	ret = ft6x36_read_reg(ctx, FT6X36_REG_GEST_ID, buff->byte, sizeof(ft6x36_reg_td_t));

	return ret;
}

/**
 * @Operating settings selection.[set]
 *
 * @param  ctx      read / write interface definitions
 * @param  buff   	ft6x36_reg_settings_t that stores data read
 * @retval          interface status (MANDATORY: return 0 -> no Error)
 *
 */
int32_t ft6x36_settings_set(ftdev_ctx_t *ctx, ft6x36_reg_settings_t *val)
{
	int32_t ret;
	ret = ft6x36_write_reg(ctx, FT6X36_REG_TH_GROUP, val->byte, sizeof(ft6x36_reg_settings_t));

	return ret;
}
/**
 * @brief  settings output value.[get]
 *
 * @param  ctx      read / write interface definitions
 * @param  buff   	ft6x36_reg_settings_t that stores data read
 * @retval          interface status (MANDATORY: return 0 -> no Error)
 *
 */
int32_t ft6x36_settings_get(ftdev_ctx_t *ctx, ft6x36_reg_settings_t *val){
	int32_t ret;
	ret = ft6x36_read_reg(ctx, FT6X36_REG_TH_GROUP, val->byte, sizeof(ft6x36_reg_settings_t));

	return ret;
}


int32_t ft6x36_pwr_mode_set(ftdev_ctx_t *ctx, ft6x36_pwr_mode_t val){
	int32_t ret;
	ret = ft6x36_write_reg(ctx, FT6X36_REG_PWR_MODE, &val, sizeof(ft6x36_reg_settings_t));

	return ret;
}
int32_t ft6x36_pwr_mode_get(ftdev_ctx_t *ctx, ft6x36_pwr_mode_t *val){
	int32_t ret;
	ret = ft6x36_read_reg(ctx, FT6X36_REG_PWR_MODE, val, sizeof(ft6x36_reg_settings_t));

	return ret;
}


/**
 * @brief  info output value.[get]
 *
 * @param  ctx      read / write interface definitions
 * @param  buff   	ft6x36_reg_settings_t that stores data read
 * @retval          interface status (MANDATORY: return 0 -> no Error)
 *
 */
int32_t ft6x36_info_get(ftdev_ctx_t *ctx, ft6x36_reg_info_t *val){
	int32_t ret;
	ret = ft6x36_read_reg(ctx, FT6X36_REG_LIB_VER_H, val->byte, sizeof(ft6x36_reg_info_t));

	return ret;
}
/**
 * @}
 *
 */


/**
  * @brief  Initialize ft6x36.
  * @param  ctx      read / write interface definitions
  * @param  GPIOx: Where x can be (A..K) to select the GPIO peripheral.
  * @param  GPIO_Pin: Specifies the pins to be toggled.
  * @retval None
  */
void ft6x36_init(ftdev_ctx_t *ctx, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
	HAL_Delay(50);
	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
	HAL_Delay(100);

//    ft6x36_device_mode_set(ctx, FT6X36_DEVICE_MODE_WORKING_MODE);	//默认就是0x00
//    ft6236_write_reg(bus, FT_ID_G_THGROUP, 12);		//设置灵敏度
//    ft6236_write_reg(bus, FT_ID_G_PERIODACTIVE, 12);	//设置更新频率
}
/**
 * @}
 *
 */
#endif

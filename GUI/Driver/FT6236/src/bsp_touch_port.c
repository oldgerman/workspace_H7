/*
 * bsp_touch_port.c
 *
 *  Created on: 2022年11月17日
 *      Author: OldGerman
 */
#include "bsp_touch_port.h"

/* Private macro -------------------------------------------------------------*/
/**I2C1 GPIO Configuration
PB7     ------> I2C1_SDA
PB8     ------> I2C1_SCL
*/
#define FT6X36_BUS hi2c1

#ifndef DEBUG_PRINT_TOUCH
#if 1  //< Change 0 to 1 to open debug macro and check program debug information
#define DEBUG_PRINT_TOUCH printf
#else
	#define DEBUG_PRINT_IMU(...)
	#endif
#endif

/* Private variables ---------------------------------------------------------*/
//static uint8_t whoamI;
/* Extern variables ----------------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
/*
 *   WARNING:
 *   Functions declare in this section are defined at the end of this file
 *   and are strictly related to the hardware platform used.
 *
 */
static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp,
                              uint16_t len);
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len);
static void platform_init(void);

static ftdev_ctx_t 		dev_ctx; //The device interface function
ft6x36_reg_td_t			ft6x36_reg_td;
ft6x36_reg_settings_t 	ft6x36_reg_settings;
ft6x36_reg_info_t 		ft6x36_reg_info;

void touch_init(void)
{
	/* Initialize the device interface function */
	dev_ctx.write_reg = platform_write;
	dev_ctx.read_reg = platform_read;
	dev_ctx.handle = &FT6X36_BUS;
	/* Initialize platform specific hardware */
	platform_init();
	/*  Check device ID */
//	ft6x36_device_id_get(&dev_ctx, &whoamI);
//
//	if (whoamI != FT6X36_ID) {
//		while (1) {
//			/* manage here device not found */
//		}
//	}

	// read default initialize value
	ft6x36_settings_get(&dev_ctx, &ft6x36_reg_settings);
	ft6x36_reg_settings.data.th_group = 70;//12;
	ft6x36_reg_settings.data.th_peak = 60;
	ft6x36_reg_settings.data.th_cal = 16;
	ft6x36_reg_settings.data.th_water = 60;
	ft6x36_reg_settings.data.th_temp = 10;
	ft6x36_reg_settings.data.th_diff = 20;
	/* 持续按下时，INT电平是脉冲信号。每隔1000ms/45Hz=22ms 左右出现1个宽度50us的低电平。 */
	ft6x36_reg_settings.data.period_active = 14; //12;	//更新频率，实际上4倍即48Hz，示波器测得45Hz左右，下降沿有效，从12修改为14没卵用，还是45Hz
	ft6x36_reg_settings.data.ctrl = 0;				//禁止由Active模式切换为Monitor模式

#if 0	/* for debug */
	/* FT6236、FT6206手册上说好的支持的手势识别，实际芯片的出厂固件没有实现，ctmd，不过这几个寄存器倒是可读可写*/
	ft6x36_reg_settings.data.offset_left_right	   	= 0x19U;
	ft6x36_reg_settings.data.offset_up_down 		= 0x19U;
	ft6x36_reg_settings.data.distance_left_right   	= 0x19U;
	ft6x36_reg_settings.data.distance_up_down 		= 0x19U;
	ft6x36_reg_settings.data.distance_zoom		  	= 0x32U;
#endif
	ft6x36_settings_set(&dev_ctx, &ft6x36_reg_settings);

	ft6x36_device_mode_set(&dev_ctx, FT6X36_DEVICE_MODE_WORKING_MODE);
	ft6x36_pwr_mode_set(&dev_ctx, FT6X36_PWR_MODE_ACTIVE);

}

void touch_update(){
#if 0	/* for debug */
	ft6x36_settings_get(&dev_ctx, &ft6x36_reg_settings);
	ft6x36_info_get(&dev_ctx, &ft6x36_reg_info);
#endif
	ft6x36_touch_data_get(&dev_ctx, &ft6x36_reg_td);
}


/*
 * @brief  Write generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to write
 * @param  bufp      pointer to data to write in register reg
 * @param  len       number of consecutive register to write
 *
 */
static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp,
                              uint16_t len)
{
  /* Write multiple command */
	int32_t ret;
  //RET = 0, HAL_STAUS = HAL_OK
  ret = HAL_I2C_Mem_Write((I2C_HandleTypeDef*)handle, FT6X36_I2C_ADD_L, reg,
                    I2C_MEMADD_SIZE_8BIT, (uint8_t*) bufp, len, 1000);

  return ret;
}

/*
 * @brief  Read generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to read
 * @param  bufp      pointer to buffer that store the data read
 * @param  len       number of consecutive register to read
 *
 */
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len)
{
  /* Read multiple command */
int32_t ret;
  //RET = 0, HAL_STAUS = HAL_OK
  ret = HAL_I2C_Master_Transmit((I2C_HandleTypeDef*)handle, FT6X36_I2C_ADD_L, &reg, 1, 1000);
  if(ret != 0) return ret;
  ret = HAL_I2C_Master_Receive((I2C_HandleTypeDef*)handle, FT6X36_I2C_ADD_L, bufp, len, 1000);

//  HAL_I2C_Mem_Read((I2C_HandleTypeDef*)handle, FT6X36_I2C_ADD_L, reg,
//                   I2C_MEMADD_SIZE_8BIT, bufp, len, 1000);

  return ret;
}


/*
 * @brief  platform specific initialization (platform dependent)
 */
static void platform_init(void)
{
	;	//Nothing to do
}

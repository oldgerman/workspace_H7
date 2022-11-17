/*
 * bsp_touch_port.h
 *
 *  Created on: 2022年11月17日
 *      Author: PSA
 */

#ifndef DRIVER_FT6236_INC_BSP_TOUCH_PORT_H_
#define DRIVER_FT6236_INC_BSP_TOUCH_PORT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp.h"
//#include "usart.h"
#include "gpio.h"
#include "i2c.h"
#include "ft6x36_reg.h"

void touch_init();
void touch_update();
//void touch_GetState(ft6x36_td_status_t *val);
extern ft6x36_reg_td_t			ft6x36_reg_td;
#ifdef __cplusplus
}
#endif
#endif /* DRIVER_FT6236_INC_BSP_TOUCH_PORT_H_ */

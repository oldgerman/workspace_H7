/*
 * BSP.cpp
 *
 *  Created on: May 16, 2022
 *      Author: OldGerman
 */

#include "bsp.h"

void bsp_Init(){

}

bool firstPwrOffToRUN = true;
/* @brief 在main()执行完MX_XXX_Init()函数们，到执行setup()之前插入的函数
 * @param None
 * @return None
 */
void preSetupInit(void){

}


/* @brief 非阻塞下等待固定的时间
 * @param timeOld 必须传入局部静态变量或全局变量
 * @param 等待时间
 * @return bool
 */
bool waitTime(uint32_t *timeOld, uint32_t wait) {
	uint32_t time = HAL_GetTick();
	if ((time - *timeOld) > wait) {
		*timeOld = time;
		return true;
	}
	return false;
}


/* @brief 非阻塞下等待固定的时间
 * @param timeOld 必须传入局部静态变量或全局变量
 * @param 等待时间
 * @return bool
 */
bool waitTimeOS(TickType_t *timeOld, TickType_t wait) {
    TickType_t time = xTaskGetTickCount();
    if ((time - *timeOld) > wait) {
        *timeOld = time;
        return true;
    }
    return false;
}

/**
  * @brief 扫描I2C从设备
  * @param hi2c     : hi2c句柄
  * @param i2cBusNum: i2c总线编号
  * @return None
  */
void i2c_scaner(I2C_HandleTypeDef *hi2c, uint8_t i2cBusNum) {
    uint8_t i = 0;
    HAL_StatusTypeDef status;
    printf("MCU: i2c%d scan...\r\n",i2cBusNum);

    for (i = 0; i < 128; i++) {
        status = HAL_I2C_IsDeviceReady(hi2c, (uint16_t)(i<<1), 1, 10);
        if (status == HAL_OK) {
            printf("addr: 0x%02X is ok\r\n",i);
        } else if (status == HAL_TIMEOUT) {
            printf("addr: 0x%02X is timeout\r\n",i);
        } else if (status == HAL_BUSY) {
            printf("addr: 0x%02X is busy\r\n",i);
        }
    }
}

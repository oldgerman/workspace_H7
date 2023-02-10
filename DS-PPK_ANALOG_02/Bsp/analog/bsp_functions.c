/**
  ******************************************************************************
  * @file        bsp_functions.c
  * @author      OldGerman
  * @created on  Sep 6, 2022
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

/* Includes ------------------------------------------------------------------*/
#include "bsp.h"
#include "bsp_functions.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Function implementations --------------------------------------------------*/
/**
  * @brief  计算2的N次幂，N最多计算到31
  * @param  Nth uint8_t 幂次数
  * @retval uint32_t 计算结果
  */
uint32_t twoNthPower(uint8_t Nth){
	if(Nth < 32){
		return 1 << Nth;
	}else
		return 0xffffffff;
}

/**
  * @brief  返回某个数（2 N次幂）的N
  * @param  num 被计算数
  * @retval uint8_t 计算的幂次
  */
uint8_t twoNthPowerOfNth(uint32_t num){
	uint8_t Nth = 0;
	for(; Nth < 32; Nth++){
		if((num >> Nth) == 0){
			break;
		}
	}
	return Nth;
}

/**
  * @brief  将一个数字(浮点型)从一个范围重新映射到另一个区域
  * @param  x: 要映射的数字
  * @param  in_min: 值的当前范围的下界
  * @param  in_max: 值的当前范围的上界
  * @param  out_min: 值的目标范围的下界
  * @param  out_max: 值目标范围的上界
  * @retval 映射的值(double)
  */
double fmap(double x, double in_min, double in_max, double out_min, double out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


/**
 * @brief   将一串数据视作bit，bit代表电平，每一个bit都转换为BSRR数据组的元素
 * 			虽然会将数据整个写入GPIO组的BSSR，但是BSSR寄存器很特殊，只能写1，清0无效，将来只作用于GPIO_Pin指定的引脚，对本GPIO组的其他GPIO_PIN无影响
 * @param	ptrBitArray			源数据指针
 * @param	bitLengthInByte		源数据长度，以byte为单位
 * @param	ptrBSRRArray		目标数据组
 * @param   GPIO_Pin: specifies the port bit
 *          This parameter can be GPIO_PIN_x where x can be (0..15).
 *          	备注：HAL库GPIO_Pin_x宏定义特点
 *          		#define GPIO_PIN_0                 ((uint16_t)0x0001)  // Pin 0 selected    0...001
 *          		#define GPIO_PIN_1                 ((uint16_t)0x0002)  // Pin 1 selected    0...010
 *          		#define GPIO_PIN_2                 ((uint16_t)0x0004)  // Pin 2 selected    0...100
 *          		......
 *          		#define GPIO_PIN_15                ((uint16_t)0x8000)  // Pin 15 selected 	1...000
 * @retval None
 */
void bsp_convertLevelToBSRR(uint8_t *ptrBitArray, uint32_t bitArrayLengthInByte, uint32_t* ptrBSRRArray, uint16_t GPIO_Pin){
	/* BSRR 高 16 位用于控制GPIO的输出低电平，低 16 位用于输出高电平工作 */
	uint32_t numBit = bitArrayLengthInByte * 8;
	for(uint32_t i = 0; i < numBit; i += 8)
	{
		uint32_t j = i / 8;
		for(uint8_t bitOffset = 0; bitOffset < 8; bitOffset++){
			if( ((*(ptrBitArray + j)) >> (7 - bitOffset)) & 0x0001) {
				*(ptrBSRRArray + i + bitOffset) = (uint32_t)GPIO_Pin;			//输出高电平，设置BS0~BS15  BSRR[0:15]
			}
			else {
				*(ptrBSRRArray + i + bitOffset) = (uint32_t)GPIO_Pin << 16;		//输出低电平，设置BR0~BR15  BSRR[16:31]
			}
		}
	}
}

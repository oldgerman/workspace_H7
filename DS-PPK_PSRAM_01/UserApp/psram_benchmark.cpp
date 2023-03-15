/**
  ******************************************************************************
  * @file        psram_benchmark.cpp
  * @author      OldGerman
  * @created on  Mar 12, 2023
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
#include "common_inc.h"
#include "psram_benchmark.h"

using namespace ns_frtos_spi;

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
const uint32_t SPI2_Cmd_PSC  = SPI_BAUDRATEPRESCALER_2;		//84MHz
//const uint32_t SPI2_Cmd_PSC  = SPI_BAUDRATEPRESCALER_4;	//42MHz
//const uint32_t SPI2_Cmd_PSC  = SPI_BAUDRATEPRESCALER_16;	//10.5MHz
//const uint32_t SPI2_Cmd_PSC  = SPI_BAUDRATEPRESCALER_64;	//2.625MHz

/* Private variables ---------------------------------------------------------*/
/* FRTOS_SPIBase类对象：SPI2_Base */
RAM_REGION_NO_CACHE uint8_t SPI2_RxBuf[FRTOS_SPIBase::sizeCmdOnly];
RAM_REGION_NO_CACHE uint8_t SPI2_TxBuf[FRTOS_SPIBase::sizeCmdOnly];

/* Exported variables --------------------------------------------------------*/
FRTOS_SPIBase SPI2_Base(hspi2, SPI2_TxBuf, SPI2_RxBuf, FRTOS_SPIBase::sizeCmdOnly);

FRTOS_SPICmd SPI2_Cmd(
		&SPI2_Base,
		SPI2_CS_GPIO_Port,
		SPI2_CS_Pin,
		SPI2_Cmd_PSC,
		SPI_PHASE_1EDGE,
		SPI_POLARITY_LOW);

/* Private function prototypes -----------------------------------------------*/
/* Function implementations --------------------------------------------------*/


void psram_init(){

}
